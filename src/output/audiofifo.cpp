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
#include <boost/assert.hpp>
#include <boost/format.hpp>

/**
 * @brief Makes volume values logarithmic
 * @param[in,out] leftVol Left channel volume
 * @param[in,out] rightVol Right channel volume
 * @see calcVolume
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

void AudioFifo::calcVolume(uint16_t& leftVol, uint16_t& rightVol) {
	BOOST_ASSERT(m_queuedFrames != 0);
	//MutexLocker mutexLock(m_queueMutex);
	std::lock_guard<std::mutex> mutexLock(m_queueMutex);
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

AudioFifo::AudioFifo(size_t frameCount) : m_queue(), m_queuedFrames(0), m_minFrameCount(frameCount), m_volumeLeft(0), m_volumeRight(0), m_queueMutex() {
	logger()->debug(L4CXX_LOCATION, boost::format("Initialized FIFO with %d frames minimum")%frameCount);
}


void AudioFifo::push(const AudioFrameBuffer& data) {
	// copy, because AudioFrameBuffer is a shared_ptr that may be modified
	AudioFrameBuffer cp(new AudioFrameBuffer::element_type);
	*cp = *data;
	std::lock_guard<std::mutex> mutexLock(m_queueMutex);
	m_queuedFrames += cp->size();
	m_queue.push_back(cp);
	//LOG_DEBUG("Added %zd frames to the queue, now queued %zd frames", cp->size(), m_queuedFrames);
}

size_t AudioFifo::pullAll(AudioFrameBuffer& data) {
	return pull(data, nsize);
}

size_t AudioFifo::pull(BasicSampleFrame* data, size_t size) {
	AudioFrameBuffer buffer;
	if(0 == pull(buffer, size))
		return 0;
	if(!buffer || buffer->size() == 0)
		return 0;
	std::copy(buffer->begin(), buffer->end(), data);
	return buffer->size();
}

size_t AudioFifo::pull(AudioFrameBuffer& data, size_t size) {
	//LOG_DEBUG("Requested %zd frames", size);
	if(needsData())
		return 0;
	calcVolume(m_volumeLeft, m_volumeRight);
	std::lock_guard<std::mutex> mutexLock(m_queueMutex);
	if(size == nsize || size > m_queuedFrames) {
		if(size != nsize) {
			logger()->warn(L4CXX_LOCATION, boost::format("Requested %d frames while only %d frames in queue")%size%m_queuedFrames);
		}
		size = m_queuedFrames;
	}
	if(!data)
		data.reset(new AudioFrameBuffer::element_type(size, {0, 0}));
	if(data->size() < size)
		data->resize(size, {0, 0});
	size_t copied = 0;
	size_t toCopy = m_queue.front()->size();
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
		else
			break;
	}
	if(toCopy != 0 && !m_queue.empty()) {
		m_queuedFrames -= toCopy;
		AudioFrameBuffer& current = m_queue.front();
		current->erase(current->begin(), current->begin() + toCopy);
	}
	//LOG_DEBUG(" -- copied %zd frames", copied);
	return copied;
}

size_t AudioFifo::copy(AudioFrameBuffer& data, size_t size) {
	//LOG_DEBUG("Requested %zd frames", size);
	if(needsData())
		return 0;
	std::lock_guard<std::mutex> mutexLock(m_queueMutex);
	if(size == nsize || size > m_queuedFrames) {
		if(size != nsize) {
			logger()->warn(L4CXX_LOCATION, boost::format("Requested %d frames while only %d frames in queue")%size%m_queuedFrames);
		}
		size = m_queuedFrames;
	}
	if(!data)
		data.reset(new AudioFrameBuffer::element_type(size, {0, 0}));
	if(data->size() < size)
		data->resize(size, {0, 0});
	size_t copied = 0;
	AudioFrameBufferQueue::iterator queueIt = m_queue.begin();
	size_t toCopy = (*queueIt)->size();
	while(queueIt != m_queue.end() && size != 0) {
		AudioFrameBuffer& current = *queueIt;
		toCopy = std::min(current->size(), size);
		std::copy(current->begin(), current->begin() + toCopy, data->begin() + copied);
		copied += toCopy;
		size -= toCopy;
		if(toCopy == current->size())
			queueIt++;
		else
			break;
	}
	//LOG_DEBUG(" -- copied %zd frames", copied);
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

bool AudioFifo::needsData() const {
	return m_queuedFrames < m_minFrameCount;
}

size_t AudioFifo::queuedChunkCount() const {
	return m_queue.size();
}

size_t AudioFifo::minFrameCount() const {
	return m_minFrameCount;
}

light4cxx::Logger::Ptr AudioFifo::logger()
{
	return light4cxx::Logger::get("audio.fifo");
}

/**
 * @}
 */
