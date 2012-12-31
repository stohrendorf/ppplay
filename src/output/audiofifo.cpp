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
	while( AbstractAudioSource::Ptr src = m_source.lock() ) {
		if( m_stopping ) {
			// the destructor is waiting for the thread to join
			break;
		}

		boost::mutex::scoped_lock bufferLock(m_bufferMutex);
		// continue if no data is available
		if( src->paused() || m_buffer.size()>=m_threshold ) {
			logger()->trace( L4CXX_LOCATION, "FIFO filled, waiting..." );
			m_bufferChanged.wait(bufferLock);
			continue;
		}
		
		size_t size = src->preferredBufferSize();
		if( size == 0 ) {
			size = m_buffer.capacity() - m_buffer.size();
		}
		if( size <= 0 ) {
			continue;
		}
		src->getAudioData( buffer, size );
		if( !src->paused() && ( !buffer || buffer->empty() ) ) {
			// logger()->debug( L4CXX_LOCATION, "Audio source dry" );
			continue;
		}
		// add the data to the queue...
		pushData( buffer );
		bufferLock.unlock();
		m_bufferChanged.notify_one();
	}
}

AudioFifo::AudioFifo( const AbstractAudioSource::WeakPtr& source, size_t threshold, bool doVolumeCalc ) :
	m_buffer(), m_threshold(threshold), m_requestThread(), m_source( source ), m_volLeftSum( 0 ), m_volLeftLog(0), m_volRightSum( 0 ), m_volRightLog(0),
	m_doVolumeCalc( doVolumeCalc ), m_stopping( false ), m_bufferMutex(), m_bufferChanged()
{
	BOOST_ASSERT_MSG( !source.expired(), "Invalid source passed to AudioFifo constructor" );
	BOOST_ASSERT_MSG( threshold >= 256, "Minimum capacity may not be less than 256" );
	logger()->debug( L4CXX_LOCATION, "Created with %d frames threshold", threshold );
	m_requestThread = boost::thread( boost::bind( &AudioFifo::requestThread, this ) );
	// next bigger 2^x
	threshold = 1 << std::lround( std::log2(threshold) + 0.5 );
	m_buffer.set_capacity( threshold );
	logger()->debug( L4CXX_LOCATION, "Set capacity to %d", threshold );
}

AudioFifo::~AudioFifo()
{
	logger()->trace( L4CXX_LOCATION, "Waiting for pulling thread to join" );
	// request the thread to terminate
	m_stopping = true;
	m_bufferChanged.notify_one();
	m_requestThread.join();
	logger()->trace( L4CXX_LOCATION, "Destroyed" );
}

void AudioFifo::pushData( const AudioFrameBuffer& buf )
{
	// DO NOT LOCK THE MUTEX HERE! This is already done
	// in the requestThread() function.
	if(!buf) {
		return;
	}
	if( buf->size() > m_buffer.capacity() - m_buffer.size() ) {
		// resize to next bigger exponent of 2
		size_t nsize = 1 << std::lround(std::log2(buf->size() + m_buffer.capacity()) + 0.5);
		logger()->warn(L4CXX_LOCATION, "Capacity too low for %d frames, increasing from %d to %d", buf->size(), m_buffer.capacity(), nsize);
		m_buffer.set_capacity(nsize);
	}
	logger()->trace(L4CXX_LOCATION, "Pushing %d frames into buffer", buf->size());
	std::copy( buf->begin(), buf->end(), std::back_inserter(m_buffer) );
	if( m_doVolumeCalc ) {
		uint64_t left, right;
		sumAbsValues( buf, left, right );
		m_volLeftSum += left;
		m_volRightSum += right;
		if( m_buffer.size() >= 4 ) {
			m_volLeftLog = logify( m_volLeftSum / ( m_buffer.size() >> 2 ) );
			m_volRightLog = logify( m_volRightSum / ( m_buffer.size() >> 2 ) );
		}
		else {
			m_volLeftLog = 0;
			m_volRightLog = 0;
		}
	}
}

size_t AudioFifo::pullData( AudioFrameBuffer& data, size_t size )
{
	boost::mutex::scoped_lock lock( m_bufferMutex );
	if( size > m_buffer.size() ) {
		logger()->trace( L4CXX_LOCATION, "Buffer underrun: Requested %d frames while only %d frames in queue", size, m_buffer.size() );
		size = m_buffer.size();
	}
	if( !data ) {
		data.reset( new AudioFrameBuffer::element_type( size ) );
	}
	if( size>data->size() ) {
		data->resize(size);
	}
	std::copy_n( m_buffer.begin(), size, data->begin() );
	m_buffer.erase_begin(size);
	logger()->trace( L4CXX_LOCATION, "Pulled %d frames, %d frames left", size, m_buffer.size() );
	
	if( data->size() != size ) {
		logger()->error( L4CXX_LOCATION, "Copied %d frames into a buffer with %d frames", size, data->size() );
	}
	if( m_doVolumeCalc ) {
		uint64_t left, right;
		sumAbsValues( data, left, right );
		m_volLeftSum -= left;
		m_volRightSum -= right;
		if( m_buffer.size() >= 4 ) {
			m_volLeftLog = logify( m_volLeftSum / ( m_buffer.size() >> 2 ) );
			m_volRightLog = logify( m_volRightSum / ( m_buffer.size() >> 2 ) );
		}
		else {
			m_volLeftLog = 0;
			m_volRightLog = 0;
		}
	}
	lock.unlock();
	m_bufferChanged.notify_one();
	return size;
}

size_t AudioFifo::queuedLength() const
{
	return m_buffer.size();
}

bool AudioFifo::isEmpty() const
{
	return m_buffer.empty();
}

uint16_t AudioFifo::volumeRight() const
{
	return m_volRightLog;
}

uint16_t AudioFifo::volumeLeft() const
{
	return m_volLeftLog;
}

void AudioFifo::setCapacity( size_t len )
{
	boost::mutex::scoped_lock lock( m_bufferMutex );
	BOOST_ASSERT_MSG( len >= 256, "Capacity too low (minimum 256)" );
	m_buffer.set_capacity(len);
}

size_t AudioFifo::capacity() const
{
	boost::mutex::scoped_lock lock( m_bufferMutex );
	return m_buffer.capacity();
}

light4cxx::Logger* AudioFifo::logger()
{
	return light4cxx::Logger::get( "audio.fifo" );
}

/**
 * @}
 */
