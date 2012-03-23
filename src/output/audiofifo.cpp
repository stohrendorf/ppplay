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

/**
 * @brief Makes volume values logarithmic
 * @param[in] value Volume to make logarithmic
 * @return A more "natural" feeling value
 * @see AudioFifo::calcVolume()
 */
static uint16_t logify( uint16_t value )
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
static void sumAbsValues( const AudioFrameBuffer& buf, uint64_t& left, uint64_t& right )
{
	left = right = 0;
	for( const BasicSampleFrame & frame : *buf ) {
		left += std::abs( frame.left );
		right += std::abs( frame.right );
	}
}

void AudioFifo::requestThread( AudioFifo* fifo )
{
	BOOST_ASSERT( fifo != nullptr );
	AudioFrameBuffer buffer;
	while( !fifo->m_source.expired() ) {
		IAudioSource::Ptr src = fifo->m_source.lock();
		// continue if no data is available
		if( src->paused() || fifo->m_queuedFrames >= fifo->m_minFrameCount ) {
			logger()->trace(L4CXX_LOCATION, "FIFO filled, waiting..." );
			boost::this_thread::sleep( boost::posix_time::millisec( 10 ) );
			continue;
		}
		// keep it low to avoid blockings
		int size = src->preferredBufferSize();
		if( size == 0 ) {
			size = std::max<int>( fifo->m_minFrameCount / 2, fifo->m_minFrameCount - fifo->m_queuedFrames );
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
		logger()->trace( L4CXX_LOCATION, boost::format( "Adding %4d frames to a %4d-frame queue, minimum is %4d frames" ) % buffer->size() % fifo->m_queuedFrames % fifo->m_minFrameCount );
		fifo->pushBuffer( buffer );
	}
}

AudioFifo::AudioFifo( const IAudioSource::WeakPtr& source, size_t frameCount ) :
	m_queue(), m_queuedFrames( 0 ), m_minFrameCount( frameCount ),
	m_requestThread(), m_source( source ), m_volLeftSum( 0 ), m_volRightSum( 0 ),
	m_mutex()
{
	BOOST_ASSERT( !source.expired() );
	BOOST_ASSERT( m_minFrameCount >= 256 );
	logger()->debug( L4CXX_LOCATION, boost::format( "Created with %d frames minimum" ) % frameCount );
	m_requestThread = boost::thread( requestThread, this );
	//m_requestThread.detach();
}

AudioFifo::~AudioFifo()
{
	m_requestThread.join();
	logger()->trace( L4CXX_LOCATION, "Destroyed" );
}

void AudioFifo::pushBuffer( const AudioFrameBuffer& buf )
{
	//ReadWriteLockable::WriteLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	// copy, because AudioFrameBuffer is a shared_ptr that may be modified
	AudioFrameBuffer cp( new AudioFrameBuffer::element_type );
	*cp = *buf;
	uint64_t left, right;
	sumAbsValues( cp, left, right );
	m_volLeftSum += left;
	m_volRightSum += right;
	m_queuedFrames += cp->size();
	m_queue.push( cp );
}

size_t AudioFifo::getAudioData( AudioFrameBuffer& data, size_t size )
{
	// a modifying function, so we need a unique lock again
	//ReadWriteLockable::WriteLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( size > m_queuedFrames ) {
		logger()->trace( L4CXX_LOCATION, boost::format( "Buffer underrun: Requested %d frames while only %d frames in queue" ) % size % m_queuedFrames );
		size = m_queuedFrames;
	}
	if( !data ) {
		data.reset( new AudioFrameBuffer::element_type( size, {0, 0} ) );
	}
	if( data->size() < size ) {
		data->resize( size, {0, 0} );
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
		logger()->error( L4CXX_LOCATION, boost::format( "Copied %d frames into a buffer with %d frames" ) % copied % data->size() );
	}
	{
		uint64_t left, right;
		sumAbsValues( data, left, right );
		m_volLeftSum -= left;
		m_volRightSum -= right;
	}
	logger()->trace( L4CXX_LOCATION, boost::format( "Pulled %d frames, %d frames left" ) % copied % m_queuedFrames );
	return copied;
}

size_t AudioFifo::queuedLength() const
{
	//ReadWriteLockable::ReadLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_queuedFrames;
}

bool AudioFifo::isEmpty() const
{
	//ReadWriteLockable::ReadLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_queuedFrames == 0;
}

uint16_t AudioFifo::volumeRight() const
{
	//ReadWriteLockable::ReadLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( m_queuedFrames == 0 ) {
		return 0;
	}
	return logify( m_volRightSum / ( m_queuedFrames >> 2 ) );
}

uint16_t AudioFifo::volumeLeft() const
{
	//ReadWriteLockable::ReadLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( m_queuedFrames == 0 ) {
		return 0;
	}
	return logify( m_volLeftSum / ( m_queuedFrames >> 2 ) );
}

void AudioFifo::setMinFrameCount( size_t len )
{
	//ReadWriteLockable::WriteLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT( len >= 256 );
	m_minFrameCount = len;
}

size_t AudioFifo::queuedChunkCount() const
{
	//ReadWriteLockable::ReadLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_queue.size();
}

size_t AudioFifo::minFrameCount() const
{
	//ReadWriteLockable::ReadLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_minFrameCount;
}

bool AudioFifo::initialize( uint32_t frequency )
{
	//ReadWriteLockable::WriteLock lock(&m_readWriteLockable);
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	logger()->trace( L4CXX_LOCATION, "Initializing" );
	IAudioSource::Ptr source( m_source.lock() );
	BOOST_ASSERT( !m_source.expired() );
	return source->initialize( frequency );
}

light4cxx::Logger::Ptr AudioFifo::logger()
{
	return light4cxx::Logger::get( "audio.fifo" );
}

/**
 * @}
 */
