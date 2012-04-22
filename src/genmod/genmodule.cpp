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
 * @ingroup GenMod
 * @{
 */

#include "genmodule.h"

#include "genbase.h"
#include "genorder.h"
#include "stream/memarchive.h"
#include "stuff/scopedcall.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace ppp
{

GenModule::GenModule( int maxRpt ) :
	m_metaInfo(),
	m_orders(),
	m_state(),
	m_songs(),
	m_maxRepeat(maxRpt),
	m_initialState(new MemArchive()),
	m_mutex()
{
	BOOST_ASSERT_MSG( maxRpt != 0, "Maximum repeat count may not be 0" );
}

GenModule::~GenModule()
{
	deleteAll(m_orders);
}

IArchive& GenModule::serialize( IArchive* data )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	for( GenOrder* order : m_orders ) {
		data->archive( order );
	}
	data->archive( &m_state );
	return *data;
}

std::string GenModule::filename()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	boost::filesystem::path path( m_metaInfo.filename );
	BOOST_ASSERT_MSG( path.has_filename(), "File path does not contain a file name" );
	return path.filename().native();
}

std::string GenModule::trimmedTitle() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return boost::algorithm::trim_copy( m_metaInfo.title );
}

uint32_t GenModule::timeElapsed() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return static_cast<uint32_t>( m_state.playedFrames / frequency() );
}

size_t GenModule::length() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songs.current().length;
}

bool GenModule::isMultiSong() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songs.size() > 1;
}

size_t GenModule::internal_getAudioData( AudioFrameBuffer& buffer, size_t size )
{
	if( !buffer ) {
		buffer.reset( new AudioFrameBuffer::element_type );
	}
	buffer->reserve( size );
	buffer->resize( 0 );
	AudioFrameBuffer tmpBuf;
	while( buffer->size() < size ) {
		size_t size = buildTick( &tmpBuf );
		if( !tmpBuf || tmpBuf->empty() || size==0 ) {
			logger()->debug(L4CXX_LOCATION, "buildTick() returned 0");
			return 0;
		}
		buffer->insert( buffer->end(), tmpBuf->begin(), tmpBuf->end() );
	}
	return buffer->size();
}

size_t GenModule::internal_preferredBufferSize() const
{
	return tickBufferLength();
}

void GenModule::addOrder( GenOrder* o )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_orders.push_back( o );
}

GenOrder* GenModule::orderAt( size_t idx )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if(idx >= m_orders.size()) {
		logger()->error(L4CXX_LOCATION, "Requested order index out of range: %d >= %d", idx, m_orders.size());
		throw std::out_of_range("Requested order index out of range");
	}
	return m_orders.at( idx );
}

size_t GenModule::orderCount() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_orders.size();
}

int GenModule::maxRepeat() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_maxRepeat;
}

bool GenModule::setOrder( size_t o, bool estimateOnly, bool forceSave )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	bool orderChanged = (o != m_state.order) || forceSave;
	if( orderChanged ) {
		orderAt( m_state.order )->increasePlaybackCount();
	}
	m_state.order = o;
	if( o >= orderCount() ) {
		return false;
	}
	m_state.pattern = orderAt( m_state.order)->index();
	if( orderChanged ) {
		logger()->info(L4CXX_LOCATION, "Order change%s to %d (pattern %d)", (forceSave ? " (forced save)" : ""), m_state.order, m_state.pattern );
		try {
			if(!estimateOnly) {
				if(!m_songs.current().states.atEnd()) {
					IArchive::Ptr state = m_songs.current().states.next();
					state->archive( this ).finishLoad();
				}
				else {
					m_songs.current().states.append(new MemArchive())->archive(this).finishSave();
					m_songs.current().states.next();
				}
			}
		}
		catch( ... ) {
			BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
		}
	}
	return m_state.order < orderCount();
}

void GenModule::setRow( int16_t r )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_state.row = r;
}

void GenModule::nextTick()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT_MSG( m_state.speed != 0, "Data corruption: speed==0" );
	m_state.tick = ( m_state.tick + 1 ) % m_state.speed;
}

void GenModule::setTempo( uint8_t t )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( t == 0 ) return;
	m_state.tempo = t;
}

void GenModule::setSpeed( uint8_t s )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( s == 0 ) return;
	m_state.speed = s;
}

uint16_t GenModule::songCount() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songs.size();
}

int16_t GenModule::currentSongIndex() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songs.where();
}

uint16_t GenModule::tickBufferLength() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT_MSG( m_state.tempo != 0, "Data corruption: tempo==0" );
	return frequency() * 5 / ( m_state.tempo << 1 );
}

light4cxx::Logger::Ptr GenModule::logger()
{
	return light4cxx::Logger::get( "module" );
}

void GenModule::loadInitialState()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	logger()->info(L4CXX_LOCATION, "Loading initial state");
	m_initialState->archive( this ).finishLoad();
}

void GenModule::saveInitialState()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	logger()->info(L4CXX_LOCATION, "Storing initial state");
	m_initialState->archive( this ).finishSave();
}

bool GenModule::jumpNextOrder()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( !m_songs.current().states.atEnd() ) {
		logger()->debug(L4CXX_LOCATION, "Already preprocessed - loading");
		IArchive::Ptr next = m_songs.current().states.next();
		next->archive( this ).finishLoad();
		return true;
	}
	// maybe not processed yet, so try to jump to the next order...
	logger()->debug(L4CXX_LOCATION, "Not preprocessed yet");
	size_t ord = m_state.order;
	bool wasPaused = paused();
	setPaused(true);
	do {
		if(m_state.order>=orderCount()) {
			setPaused( wasPaused );
			return false;
		}
		AudioFrameBuffer buf;
		if(buildTick(&buf)==0 || !buf) {
			setPaused( wasPaused );
			return false;
		}
	} while(ord == m_state.order);
	setPaused( wasPaused );
	return true;
}

bool GenModule::jumpPrevOrder()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( !m_songs.current().states.atFront() ) {
		m_songs.current().states.prev()->archive( this ).finishLoad();
		return true;
	}
	return false;
}

bool GenModule::jumpNextSong()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	bool wasPaused = paused();
	setPaused(true);
	
	logger()->debug(L4CXX_LOCATION, "Trying to jump to next song");
	if( !initialized() ) {
		for( size_t i = 0; i < orderCount(); i++ ) {
			if( !orderAt(i)->isUnplayed() ) {
				continue;
			}
			logger()->debug(L4CXX_LOCATION, "Found unplayed order %d pattern %d", i, 0+orderAt(i)->index());
			m_songs.append();
			++m_songs;
			m_state.playedFrames = 0;
			m_state.pattern = orderAt( i )->index();
			setOrder( i, false );
			setPaused( wasPaused );
			return true;
		}
		setPaused( wasPaused );
		return false;
	}
	if(m_songs.atEnd()) {
		setPaused( wasPaused );
		return false;
	}
	++m_songs;
	m_songs.current().states.revert();
	m_songs.current().states.current()->archive( this ).finishLoad();
	//BOOST_ASSERT_MSG( orderAt( m_state.order ).use_count() > 0, "Current order is a nullptr" );
	m_state.pattern = orderAt( m_state.order )->index();
	setPaused( wasPaused );
	return true;
}

bool GenModule::jumpPrevSong()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( !isMultiSong() ) {
		logger()->info( L4CXX_LOCATION, "This is not a multi-song" );
		return false;
	}
	if( m_songs.atFront() ) {
		logger()->info( L4CXX_LOCATION, "Already on first song" );
		return false;
	}
	--m_songs;
	m_songs.current().states.revert();
	m_songs.current().states.current()->archive( this ).finishLoad();
	//state().pattern = orderAt( m_state.order ).index();
	return true;
}

bool GenModule::internal_initialize( uint32_t )
{
	if( initialized() ) {
		return true;
	}
	saveInitialState();
	
	logger()->info( L4CXX_LOCATION, "Calculating song lengths and preparing seek operations..." );
	while( jumpNextSong() ) {
		logger()->info( L4CXX_LOCATION, "Pre-processing song %d", m_songs.where()+1 );
		while( size_t len = buildTick( nullptr ) ) {
			m_songs.current().length += len;
		}
		logger()->info( L4CXX_LOCATION, "Song preprocessed, trying to jump to the next song." );
	}
	logger()->info( L4CXX_LOCATION, "Lengths calculated, resetting module." );
	loadInitialState();
	// FIXME m_songs.removeEmptySongs();
	m_songs.revert();
	return true;
}

size_t GenModule::buildTick( AudioFrameBuffer* buf )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_buildTick(buf);
}

std::string GenModule::channelCellString( size_t idx ) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_channelCellString(idx);
}

int GenModule::channelCount() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_channelCount();
}

std::string GenModule::channelStatus( size_t idx ) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_channelStatus(idx);
}

}

/**
 * @}
 */
