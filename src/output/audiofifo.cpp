/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @ingroup Output
 * @{
 */

#include "audiofifo.h"

#include <algorithm>
#include <thread>

#include <boost/assert.hpp>
#include <boost/format.hpp>

/**
 * @brief Makes volume values logarithmic
 * @param[in,out] leftVol Left channel volume
 * @param[in,out] rightVol Right channel volume
 * @see AudioFifo::calcVolume()
 */
static void logify(uint16_t& leftVol, uint16_t& rightVol) {
	uint32_t tmp = leftVol << 1;
	if(tmp > 0x8000)
		tmp = (0x8000 + tmp) >> 1;
	if(tmp > 0xb000)
		tmp = (0xb000 + tmp) >> 1;
	if(tmp > 0xe000)
		tmp = (0xe000 + tmp) >> 1;
	leftVol = tmp > 0xffff ? 0xffff : tmp;
	tmp = rightVol << 1;
	if(tmp > 0x8000)
		tmp = (0x8000 + tmp) >> 1;
	if(tmp > 0xb000)
		tmp = (0xb000 + tmp) >> 1;
	if(tmp > 0xe000)
		tmp = (0xe000 + tmp) >> 1;
	rightVol = tmp > 0xffff ? 0xffff : tmp;
}

void AudioFifo::requestThread(AudioFifo* fifo)
{
	BOOST_ASSERT(fifo != nullptr);
	while(!fifo->m_source.expired()) {
		// continue if no data is available
		if(fifo->m_queuedFrames >= fifo->m_minFrameCount) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		AudioFrameBuffer buffer;
		// keep it low to avoid blockings
		int size = std::max<int>(256, fifo->m_minFrameCount - fifo->m_queuedFrames);
		if(size<=0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		{
			// request the data
			IAudioSource::Ptr src = fifo->m_source.lock();
			IAudioSource::LockGuard guard(src.get());
			if(0==src->getAudioData(buffer, size)) {
				logger()->trace(L4CXX_LOCATION, "Audio source dry");
				break;
			}
		}
		// add the data to the queue...
		logger()->trace(L4CXX_LOCATION, boost::format("Adding %d frames to a %d-frame queue, minimum is %d frames")%buffer->size()%fifo->m_queuedFrames%fifo->m_minFrameCount);
		// copy, because AudioFrameBuffer is a shared_ptr that may be modified
		AudioFrameBuffer cp(new AudioFrameBuffer::element_type);
		*cp = *buffer;
		fifo->waitLock();
		fifo->m_queuedFrames += cp->size();
		fifo->m_queue.push_back(cp);
		fifo->unlock();
	}
}

AudioFifo::AudioFifo(const IAudioSource::WeakPtr& source, size_t frameCount) :
	m_queue(), m_queuedFrames(0), m_minFrameCount(frameCount), m_volumeLeft(0),
	m_volumeRight(0), m_queueMutex(), m_requestThread(), m_source(source)
{
	BOOST_ASSERT(!source.expired());
	logger()->debug(L4CXX_LOCATION, boost::format("Created with %d frames minimum")%frameCount);
	m_requestThread = std::thread(requestThread, this);
	m_requestThread.detach();
}

AudioFifo::~AudioFifo() {
	logger()->trace(L4CXX_LOCATION, "Destroyed");
}

void AudioFifo::calcVolume(uint16_t& leftVol, uint16_t& rightVol) {
	if(m_queuedFrames == 0) {
		return;
	}
	std::lock_guard<std::recursive_mutex> mutexLock(m_queueMutex);
	leftVol = rightVol = 0;
	uint64_t leftSum = 0, rightSum = 0;
	size_t totalProcessed = 0;
	for(const AudioFrameBuffer& buf : m_queue) {
		for(const BasicSampleFrame& frame : *buf) {
			leftSum  += abs(frame.left);
			rightSum += abs(frame.right);
 		}
		totalProcessed += buf->size();
	}
	leftVol = (leftSum << 2) / totalProcessed;
	rightVol = (rightSum << 2) / totalProcessed;
	logify(leftVol, rightVol);
}

size_t AudioFifo::getAudioData(AudioFrameBuffer& data, size_t size) {
	calcVolume(m_volumeLeft, m_volumeRight);
	std::lock_guard<std::recursive_mutex> mutexLock(m_queueMutex);
	if(size > m_queuedFrames) {
		logger()->warn(L4CXX_LOCATION, boost::format("Buffer underrun: Requested %d frames while only %d frames in queue")%size%m_queuedFrames);
		size = m_queuedFrames;
	}
	if(!data) {
		data.reset(new AudioFrameBuffer::element_type(size, {0, 0}));
	}
	if(data->size() < size) {
		data->resize(size, {0, 0});
	}
	size_t copied = 0;
	size_t toCopy = size;
	while(!m_queue.empty() && size != 0) {
		AudioFrameBuffer& current = m_queue.front();
		toCopy = std::min(current->size(), size);
		std::copy(current->begin(), current->begin() + toCopy, data->begin() + copied);
		copied += toCopy;
		size -= toCopy;
		if(toCopy == current->size()) {
			m_queuedFrames -= toCopy;
			m_queue.pop_front();
			toCopy = 0;
		}
		else {
			break;
		}
	}
	if(toCopy != 0 && !m_queue.empty()) {
		m_queuedFrames -= toCopy;
		AudioFrameBuffer& current = m_queue.front();
		current->erase(current->begin(), current->begin() + toCopy);
	}
	if(data->size() != copied) {
		logger()->error(L4CXX_LOCATION, boost::format("Copied %d frames into a buffer with %d frames")%copied%data->size());
	}
	return copied;
}

size_t AudioFifo::queuedLength() const {
	return m_queuedFrames;
}

bool AudioFifo::isEmpty() const {
	return m_queuedFrames == 0;
}

uint16_t AudioFifo::volumeRight() const {
	return m_volumeRight;
}

uint16_t AudioFifo::volumeLeft() const {
	return m_volumeLeft;
}

void AudioFifo::setMinFrameCount(size_t len) {
	m_minFrameCount = len;
}

size_t AudioFifo::queuedChunkCount() const {
	return m_queue.size();
}

size_t AudioFifo::minFrameCount() const {
	return m_minFrameCount;
}

bool AudioFifo::initialize(uint32_t frequency)
{
	logger()->trace(L4CXX_LOCATION, "Initializing");
	BOOST_ASSERT(!m_source.expired());
	IAudioSource::Ptr lock(m_source.lock());
	return lock->initialize(frequency);
}

light4cxx::Logger::Ptr AudioFifo::logger()
{
	return light4cxx::Logger::get("audio.fifo");
}

/**
 * @}
 */
