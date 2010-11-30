/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "audiofifo.h"
//#include "genbase.h"

#include <algorithm>

using namespace ppp;

void AudioFifo::calcVolume( uint16_t& leftVol, uint16_t& rightVol ) throw( PppException ) {
	PPP_TEST( m_queuedFrames == 0 );
	leftVol = rightVol = 0;
	uint64_t leftSum = 0, rightSum = 0;
	std::size_t totalProcessed = 0;
	std::for_each(
		m_queue.begin(), m_queue.end(),
		[&](AudioFrameBuffer buf) {
			if(totalProcessed>m_minFrameCount)
				return;
			BasicSample *bPtr = &buf->front().left;
			std::size_t len = buf->size();
			for ( std::size_t i = 0; i < len; i++ ) {
				leftSum  += abs( *( bPtr++ ) );
				rightSum += abs( *( bPtr++ ) );
			}
			totalProcessed+=len;
		}
	);
	leftVol = ( leftSum << 2 ) / totalProcessed;
	rightVol = ( rightSum << 2 ) / totalProcessed;
	logify(leftVol,rightVol);
}

void AudioFifo::logify(uint16_t& leftVol, uint16_t& rightVol) throw () {
	unsigned long tmp = leftVol<<1;
	if(tmp>0x8000) tmp = (0x8000+tmp)>>1;
	if(tmp>0xb000) tmp = (0xb000+tmp)>>1;
	if(tmp>0xe000) tmp = (0xe000+tmp)>>1;
	leftVol = tmp>0xffff?0xffff:tmp;
	tmp = rightVol<<1;
	if(tmp>0x8000) tmp = (0x8000+tmp)>>1;
	if(tmp>0xb000) tmp = (0xb000+tmp)>>1;
	if(tmp>0xe000) tmp = (0xe000+tmp)>>1;
	rightVol = tmp>0xffff?0xffff:tmp;
}

AudioFifo::AudioFifo( const size_t frameCount ) : m_queue(), m_queuedFrames(0), m_minFrameCount(frameCount), m_volumeLeft(0), m_volumeRight(0) {
}

void AudioFifo::feedChunk(const AudioFrameBuffer& data) throw(PppException) {
	// copy, because AudioFrameBuffer is a shared_ptr that may be modified
	AudioFrameBuffer cp(new AudioFrameBuffer::element_type);
	*cp = *data;
	m_queuedFrames += cp->size();
	m_queue.push_back(cp);
	//LOG_DEBUG("Added %zd frames to the queue, now queued %zd frames", cp->size(), m_queuedFrames);
}

std::size_t AudioFifo::getAll(AudioFrameBuffer& data) {
	return get(data, nsize);
}

std::size_t ppp::AudioFifo::get(AudioFrameBuffer& data, std::size_t size) {
	//LOG_DEBUG("Requested %zd frames", size);
	if(needsData())
		return 0;
	calcVolume(m_volumeLeft, m_volumeRight);
	if(size==nsize)
		size = m_queuedFrames;
	if(!data)
		data.reset( new AudioFrameBuffer::element_type(size, {0,0}) );
	std::size_t copied = 0;
	std::size_t toCopy = m_queue.front()->size();
	while(!m_queue.empty() && size!=0) {
		AudioFrameBuffer &current = m_queue.front();
		toCopy = std::min(current->size(), size);
		std::copy(current->begin(), current->begin()+toCopy, data->begin()+copied);
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
	if(toCopy!=0 && !m_queue.empty()) {
		m_queuedFrames -= toCopy;
		AudioFrameBuffer &current = m_queue.front();
		current->erase(current->begin(), current->begin()+toCopy);
	}
	//LOG_DEBUG(" -- copied %zd frames", copied);
	return copied;
}

std::size_t ppp::AudioFifo::copy(AudioFrameBuffer& data, std::size_t size) {
	//LOG_DEBUG("Requested %zd frames", size);
	if(needsData())
		return 0;
	if(size==nsize)
		size = m_queuedFrames;
	if(!data)
		data.reset( new AudioFrameBuffer::element_type(size, {0,0}) );
	std::size_t copied = 0;
	AudioFrameBufferQueue::iterator queueIt = m_queue.begin();
	std::size_t toCopy = (*queueIt)->size();
	while(queueIt!=m_queue.end() && size!=0) {
		AudioFrameBuffer &current = *queueIt;
		toCopy = std::min(current->size(), size);
		std::copy(current->begin(), current->begin()+toCopy, data->begin()+copied);
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
