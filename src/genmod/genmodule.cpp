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

#include "genbase.h"
#include "genmodule.h"
#include "stream/memarchive.h"
#include "stuff/scopedcall.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace ppp
{

GenModule::GenModule( uint8_t maxRpt ) :
	m_filename(),
	m_title(),
	m_trackerInfo(),
	m_orders(),
	m_speed( 0 ),
	m_tempo( 0 ),
	m_order( 0 ),
	m_row( 0 ),
	m_tick( 0 ),
	m_globalVolume( 0x40 ),
	m_playedFrames( 0 ),
	m_pattern( 0 ),
	m_maxRepeat( maxRpt ),
	m_songs(),
	m_songLengths(),
	m_currentSongIndex( -1 ),
	m_initialState(new MemArchive()),
	m_mutex()
{
	BOOST_ASSERT( maxRpt != 0 );
}

GenModule::~GenModule() = default;

IArchive& GenModule::serialize( IArchive* data )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	*data
	% m_speed
	% m_tempo
	% m_order
	% m_row
	% m_tick
	% m_globalVolume
	% m_playedFrames
	% m_pattern
	;
	for( const GenOrder::Ptr & order : m_orders ) {
		data->archive( order.get() );
	}
	return *data;
}

std::string GenModule::filename()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	boost::filesystem::path path( m_filename );
	BOOST_ASSERT( path.has_filename() );
	return path.filename().native();
}

std::string GenModule::title() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_title;
}

std::string GenModule::trimmedTitle() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return boost::algorithm::trim_copy( m_title );
}

uint32_t GenModule::timeElapsed() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT( frequency() != 0 );
	return static_cast<uint32_t>( m_playedFrames / frequency() );
}

size_t GenModule::length() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songLengths.at( m_currentSongIndex );
}

std::string GenModule::trackerInfo() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_trackerInfo;
}

bool GenModule::isMultiSong() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songs.size() > 1;
}

void GenModule::removeEmptySongs()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	logger()->info( L4CXX_LOCATION, "Removing empty songs" );
	std::vector<StateIterator> nTr;
	std::vector<size_t> nTrLen;
	for( size_t i = 0; i < m_songs.size(); i++ ) {
		if( m_songLengths.at( i ) != 0 ) {
			nTr.push_back( m_songs.at( i ) );
			nTrLen.push_back( m_songLengths.at( i ) );
		}
	}
	m_songs = nTr;
	m_songLengths = nTrLen;
	m_currentSongIndex = 0;
}

size_t GenModule::getAudioData( AudioFrameBuffer& buffer, size_t size )
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

size_t GenModule::preferredBufferSize() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return tickBufferLength();
}

void GenModule::setGlobalVolume( int16_t v )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_globalVolume = v;
}

void GenModule::setPosition( size_t p )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_playedFrames = p;
}

void GenModule::addOrder( const GenOrder::Ptr& o )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_orders.push_back( o );
}

std::string GenModule::filename() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_filename;
}

void GenModule::setFilename( const std::string& f )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_filename = f;
}

void GenModule::setTrackerInfo( const std::string& t )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_trackerInfo = t;
}

GenOrder::Ptr GenModule::orderAt( size_t idx ) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if(idx >= m_orders.size()) {
		logger()->error(L4CXX_LOCATION, boost::format("Requested order index out of range: %d >= %d")%idx%m_orders.size());
	}
	BOOST_ASSERT( idx < m_orders.size() );
	return m_orders.at( idx );
}

size_t GenModule::patternIndex() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_pattern;
}

void GenModule::setPatternIndex( size_t i )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_pattern = i;
}

size_t GenModule::orderCount() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_orders.size();
}

void GenModule::setCurrentSongIndex( uint16_t t )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_currentSongIndex = t;
}

void GenModule::setTitle( const std::string& t )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_title = t;
}

StateIterator& GenModule::multiSongAt( size_t idx )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songs.at( idx );
}

size_t& GenModule::multiSongLengthAt( size_t idx )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songLengths.at( idx );
}

void GenModule::addMultiSong( const StateIterator& t )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_songs.push_back( t );
	m_songLengths.push_back( 0 );
}

uint16_t GenModule::maxRepeat() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_maxRepeat;
}

bool GenModule::setOrder( size_t o, bool estimateOnly, bool forceSave )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	bool orderChanged = (o != m_order) || forceSave;
	if( orderChanged ) {
		orderAt( order() )->increasePlaybackCount();
	}
	m_order = o;
	if( orderChanged ) {
		try {
			if(!estimateOnly) {
				IArchive::Ptr state = currentMultiSong().nextState();
				if(state) {
					state->archive( this ).finishLoad();
				}
				else {
					currentMultiSong().newState()->archive( this ).finishSave();
					currentMultiSong().nextState();
				}
			}
			logger()->info(L4CXX_LOCATION, boost::format("Order change%s to %d (pattern %d)") % (forceSave ? " (forced save)" : "") % order() % patternIndex() );
		}
		catch( ... ) {
			BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
		}
	}
	return order() < orderCount();
}

void GenModule::setRow( int16_t r )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_row = r;
}

void GenModule::nextTick()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT( m_speed != 0 );
	m_tick = ( m_tick + 1 ) % m_speed;
}

void GenModule::setTempo( uint8_t t )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( t == 0 ) return;
	m_tempo = t;
}

void GenModule::setSpeed( uint8_t s )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( s == 0 ) return;
	m_speed = s;
}

size_t GenModule::position() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_playedFrames;
}

uint16_t GenModule::songCount() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_songs.size();
}

int16_t GenModule::currentSongIndex() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_currentSongIndex;
}

uint16_t GenModule::tickBufferLength() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	BOOST_ASSERT( m_tempo != 0 );
	return frequency() * 5 / ( m_tempo << 1 );
}

uint8_t GenModule::tick() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_tick;
}

light4cxx::Logger::Ptr GenModule::logger()
{
	return light4cxx::Logger::get( "module" );
}

int16_t GenModule::speed() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_speed;
}

int16_t GenModule::tempo() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_tempo;
}

int16_t GenModule::globalVolume() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_globalVolume;
}

size_t GenModule::order() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_order;
}

int16_t GenModule::row() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_row;
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
	IArchive::Ptr next = currentMultiSong().nextState();
	if( next ) {
		logger()->debug(L4CXX_LOCATION, "Already preprocessed - loading");
		next->archive( this ).finishLoad();
		return true;
	}
	// maybe not processed yet, so try to jump to the next order...
	logger()->debug(L4CXX_LOCATION, "Not preprocessed yet");
	size_t ord = order();
	bool wasPaused = paused();
	setPaused(true);
	do {
		if(order()>=orderCount()) {
			setPaused( wasPaused );
			return false;
		}
		AudioFrameBuffer buf;
		if(buildTick(&buf)==0 || !buf) {
			setPaused( wasPaused );
			return false;
		}
	} while(ord == order());
	setPaused( wasPaused );
	return true;
}

bool GenModule::jumpPrevOrder()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	IArchive::Ptr next = currentMultiSong().prevState();
	if( !next ) {
		return false;
	}
	next->archive( this ).finishLoad();
	return true;
}

bool GenModule::jumpNextSong()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	bool wasPaused = paused();
	setPaused(true);
	
	logger()->debug(L4CXX_LOCATION, "Trying to jump to next song: increasing song index");
	setCurrentSongIndex( currentSongIndex() + 1 );
	if( currentSongIndex() >= songCount() ) {
		logger()->debug(L4CXX_LOCATION, "Trying to jump to next song: found unplayed song");
		for( uint16_t i = 0; i < orderCount(); i++ ) {
			BOOST_ASSERT( orderAt( i ).use_count() > 0 );
			if( orderAt(i)->isUnplayed() ) {
				BOOST_ASSERT( mapOrder( i ).use_count() > 0 );
				addMultiSong( StateIterator() );
				setPatternIndex( mapOrder( i )->index() );
				setOrder( i, false );
				setPosition( 0 );
				setPaused( wasPaused );
				return true;
			}
		}
		//addMultiSong( StateIterator() );
		setPaused( wasPaused );
		return false;
	}
	
	if( !isMultiSong() ) {
		logger()->info( L4CXX_LOCATION, "This is not a multi-song" );
		setPaused( wasPaused );
		return false;
	}
	
	currentMultiSong().gotoFront();
	currentMultiSong().currentState()->archive( this ).finishLoad();
	BOOST_ASSERT( mapOrder( order() ).use_count() > 0 );
	setPatternIndex( mapOrder( order() )->index() );
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
	if( currentSongIndex() == 0 ) {
		logger()->info( L4CXX_LOCATION, "Already on first song" );
		return false;
	}
	setCurrentSongIndex( currentSongIndex() - 1 );
	currentMultiSong().gotoFront();
	currentMultiSong().currentState()->archive( this ).finishLoad();
	GenOrder::Ptr ord = mapOrder( order() );
	BOOST_ASSERT( ord.use_count() > 0 );
	setPatternIndex( ord->index() );
	return true;
}

bool GenModule::initialize( uint32_t frq )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( initialized() ) {
		return true;
	}
	IAudioSource::initialize( frq );
	saveInitialState();
	
	logger()->info( L4CXX_LOCATION, "Calculating song lengths and preparing seek operations..." );
	do {
// 		loadInitialState();
		createMultiSong().newState()->archive( this ).finishSave();
		logger()->info( L4CXX_LOCATION, boost::format( "Pre-processing song %d" ) % currentSongIndex() );
		size_t len;
		do {
			len = buildTick( nullptr );
			multiSongLengthAt( currentSongIndex() ) += len;
		} while( len != 0 );
		logger()->info( L4CXX_LOCATION, "Song preprocessed, trying to jump to the next song." );
	} while( jumpNextSong() );
	logger()->info( L4CXX_LOCATION, "Lengths calculated, resetting module." );
	loadInitialState();
	removeEmptySongs();
	return true;
}

}

/**
 * @}
 */
