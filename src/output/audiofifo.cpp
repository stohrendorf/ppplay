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
#include <boost/thread.hpp>

namespace
{
/**
 * @brief Makes volume values logarithmic
 * @param[in] value Volume to make logarithmic
 * @return A more "natural" feeling value
 * @see AudioFifo::calcVolume()
 */
uint16_t logify( uint16_t value )
{
	uint32_t tmp = value << 1;
	if( tmp > 0x8000 )
		tmp = ( 0x8000 + tmp ) >> 1;
	if( tmp > 0xb000 )
		tmp = ( 0xb000 + tmp ) >> 1;
	if( tmp > 0xe000 )
		tmp = ( 0xe000 + tmp ) >> 1;
	return tmp > 0xffff ? 0xffff : tmp;
}

/**
 * @brief Sums up the absolute sample values of an AudioFrameBuffer
 * @param[in] buf The buffer with the values to sum up
 * @param[out] left Sum of absolute left sample values
 * @param[out] right Sum of absolute right sample values
 */
void sumAbsValues( const AudioFrameBuffer& buf, uint64_t& left, uint64_t& right )
{
	left = right = 0;
	for( const BasicSampleFrame & frame : *buf ) {
		left += std::abs( frame.left );
		right += std::abs( frame.right );
	}
}
}

void AudioFifo::requestThread()
{
	AudioFrameBuffer buffer;
	while( IAudioSource::Ptr src = m_source.lock() ) {
		if( m_stopping ) {
			break;
		}
		// continue if no data is available
		if( src->paused() || m_queuedFrames >= m_minFrameCount ) {
			logger()->trace(L4CXX_LOCATION, "FIFO filled, waiting..." );
			boost::this_thread::sleep( boost::posix_time::millisec( 10 ) );
			continue;
		}
		int size = src->preferredBufferSize();
		if( size == 0 ) {
			size = std::max<int>( m_minFrameCount / 2, m_minFrameCount - m_queuedFrames );
		}
		if( size <= 0 ) {
			boost::this_thread::sleep( boost::posix_time::millisec( 1 ) );
			continue;
		}
		src->getAudioData( buffer, size );
		if( !src->paused() && (!buffer || buffer->empty()) ) {
			logger()->debug( L4CXX_LOCATION, "Audio source dry" );
			continue;
		}
		// add the data to the queue...
		logger()->trace( L4CXX_LOCATION, "Adding %4d frames to a %4d-frame queue, minimum is %4d frames", buffer->size(), m_queuedFrames, m_minFrameCount );
		pushBuffer( buffer );
	}
}

AudioFifo::AudioFifo( const IAudioSource::WeakPtr& source, size_t minFrameCount, bool doVolumeCalc ) :
	m_queue(), m_queuedFrames( 0 ), m_minFrameCount( minFrameCount ),
	m_requestThread(), m_source( source ), m_volLeftSum( 0 ), m_volRightSum( 0 ),
	m_doVolumeCalc(doVolumeCalc), m_stopping(false), m_mutex()
{
	BOOST_ASSERT_MSG( !source.expired(), "Invalid source passed to AudioFifo constructor" );
	BOOST_ASSERT_MSG( minFrameCount >= 256, "Minimum frame count may not be less than 256" );
	logger()->debug( L4CXX_LOCATION, "Created with %d frames minimum", minFrameCount );
	m_requestThread = boost::thread( boost::bind(&AudioFifo::requestThread, this) );
	//m_requestThread.detach();
}

AudioFifo::~AudioFifo()
{
	logger()->trace( L4CXX_LOCATION, "Waiting for pulling thread to join" );
	m_stopping = true;
	m_requestThread.join();
	logger()->trace( L4CXX_LOCATION, "Destroyed" );
}

void AudioFifo::pushBuffer( const AudioFrameBuffer& buf )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	// copy, because AudioFrameBuffer is a shared_ptr that may be modified
	AudioFrameBuffer cp( new AudioFrameBuffer::element_type );
	*cp = *buf;
	if(m_doVolumeCalc) {
		uint64_t left, right;
		sumAbsValues( cp, left, right );
		m_volLeftSum += left;
		m_volRightSum += right;
	}
	m_queuedFrames += cp->size();
	m_queue.push( cp );
}

size_t AudioFifo::pullData( AudioFrameBuffer& data, size_t size )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( size > m_queuedFrames ) {
		logger()->trace( L4CXX_LOCATION, "Buffer underrun: Requested %d frames while only %d frames in queue", size, m_queuedFrames );
		size = m_queuedFrames;
	}
	if( !data ) {
		data.reset( new AudioFrameBuffer::element_type( size ) );
	}
	if( data->size() < size ) {
		data->resize( size );
	}
	size_t copied = 0;
	size_t toCopy = size;
	while( !m_queue.empty() && size != 0 ) {
		AudioFrameBuffer& current = m_queue.front();
		toCopy = std::min( current->size(), size );
		std::copy( current->begin(), current->begin() + toCopy, data->begin() + copied );
		copied += toCopy;
		size -= toCopy;
		if( toCopy == current->size() ) {
			m_queuedFrames -= toCopy;
			m_queue.pop();
			toCopy = 0;
		}
		else {
			break;
		}
	}
	if( toCopy != 0 && !m_queue.empty() ) {
		m_queuedFrames -= toCopy;
		AudioFrameBuffer& current = m_queue.front();
		current->erase( current->begin(), current->begin() + toCopy );
	}
	if( data->size() != copied ) {
		logger()->error( L4CXX_LOCATION, "Copied %d frames into a buffer with %d frames", copied, data->size() );
	}
	if(m_doVolumeCalc) {
		uint64_t left, right;
		sumAbsValues( data, left, right );
		m_volLeftSum -= left;
		m_volRightSum -= right;
	}
	logger()->trace( L4CXX_LOCATION, "Pulled %d frames, %d frames left", copied, m_queuedFrames );
	return copied;
}

size_t AudioFifo::queuedLength() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_queuedFrames;
}

bool AudioFifo::isEmpty() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_queuedFrames == 0;
}

uint16_t AudioFifo::volumeRight() const
{
	if(!m_doVolumeCalc) {
		return 0;
	}
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( m_queuedFrames == 0 ) {
		return 0;
	}
	return logify( m_volRightSum / ( m_queuedFrames >> 2 ) );
}

uint16_t AudioFifo::volumeLeft() const
{
	if(!m_doVolumeCalc) {
		return 0;
	}
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( m_queuedFrames == 0 ) {
		return 0;
	}
	return logify( m_volLeftSum / ( m_queuedFrames >> 2 ) );
}

void AudioFifo::setMinFrameCount( size_t len )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT_MSG( len >= 256, "Frame count too low (minimum 256)" );
	m_minFrameCount = len;
}

size_t AudioFifo::queuedChunkCount() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_queue.size();
}

size_t AudioFifo::minFrameCount() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_minFrameCount;
}

light4cxx::Logger* AudioFifo::logger()
{
	return light4cxx::Logger::get( "audio.fifo" );
}

/**
 * @}
 */
