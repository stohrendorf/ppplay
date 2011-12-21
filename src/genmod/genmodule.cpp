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
	m_currentSongIndex( 0 ),
	m_initialState(new MemArchive())
{
	BOOST_ASSERT( maxRpt != 0 );
}

GenModule::~GenModule() = default;

IArchive& GenModule::serialize( IArchive* data )
{
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
	boost::filesystem::path path( m_filename );
	BOOST_ASSERT( path.has_filename() );
	return path.filename().native();
}

std::string GenModule::title() const
{
	return m_title;
}

std::string GenModule::trimmedTitle() const
{
	return boost::algorithm::trim_copy( m_title );
}

uint32_t GenModule::timeElapsed() const
{
	BOOST_ASSERT( frequency() != 0 );
	return static_cast<uint32_t>( m_playedFrames / frequency() );
}

size_t GenModule::length() const
{
	return m_songLengths.at( m_currentSongIndex );
}

std::string GenModule::trackerInfo() const
{
	return m_trackerInfo;
}

bool GenModule::isMultiSong() const
{
	return m_songs.size() > 1;
}

void GenModule::removeEmptySongs()
{
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
	IAudioSource::LockGuard guard( this );
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
	return tickBufferLength();
}

void GenModule::setGlobalVolume( int16_t v )
{
	m_globalVolume = v;
}

void GenModule::setPosition( size_t p )
{
	m_playedFrames = p;
}

void GenModule::addOrder( const GenOrder::Ptr& o )
{
	m_orders.push_back( o );
}

std::string GenModule::filename() const
{
	return m_filename;
}

void GenModule::setFilename( const std::string& f )
{
	m_filename = f;
}

void GenModule::setTrackerInfo( const std::string& t )
{
	m_trackerInfo = t;
}

GenOrder::Ptr GenModule::orderAt( size_t idx ) const
{
	//if( idx>=m_orders.size() ) {
	//LOG_ERROR("%u >= %u", idx, m_orders.size());
	//}
	BOOST_ASSERT( idx < m_orders.size() );
	return m_orders.at( idx );
}

size_t GenModule::patternIndex() const
{
	return m_pattern;
}

void GenModule::setPatternIndex( size_t i )
{
	m_pattern = i;
}

size_t GenModule::orderCount() const
{
	return m_orders.size();
}

void GenModule::setCurrentSongIndex( uint16_t t )
{
	m_currentSongIndex = t;
}

void GenModule::setTitle( const std::string& t )
{
	m_title = t;
}

StateIterator& GenModule::multiSongAt( size_t idx )
{
	return m_songs.at( idx );
}

size_t& GenModule::multiSongLengthAt( size_t idx )
{
	return m_songLengths.at( idx );
}

void GenModule::addMultiSong( const StateIterator& t )
{
	m_songs.push_back( t );
	m_songLengths.push_back( 0 );
}

uint16_t GenModule::maxRepeat() const
{
	return m_maxRepeat;
}

void GenModule::setOrder( size_t o )
{
	m_order = o;
}

void GenModule::setRow( int16_t r )
{
	m_row = r;
}

void GenModule::nextTick()
{
	BOOST_ASSERT( m_speed != 0 );
	m_tick = ( m_tick + 1 ) % m_speed;
}

void GenModule::setTempo( uint8_t t )
{
	if( t == 0 ) return;
	m_tempo = t;
}

void GenModule::setSpeed( uint8_t s )
{
	if( s == 0 ) return;
	m_speed = s;
}

size_t GenModule::position() const
{
	return m_playedFrames;
}

uint16_t GenModule::songCount() const
{
	return m_songs.size();
}

uint16_t GenModule::currentSongIndex() const
{
	return m_currentSongIndex;
}

uint16_t GenModule::tickBufferLength() const
{
	BOOST_ASSERT( m_tempo != 0 );
	return frequency() * 5 / ( m_tempo << 1 );
}

uint8_t GenModule::tick() const
{
	return m_tick;
}

light4cxx::Logger::Ptr GenModule::logger()
{
	return light4cxx::Logger::get( "module" );
}

int16_t GenModule::speed() const
{
	return m_speed;
}

int16_t GenModule::tempo() const
{
	return m_tempo;
}

int16_t GenModule::globalVolume() const
{
	return m_globalVolume;
}

size_t GenModule::order() const
{
	return m_order;
}

int16_t GenModule::row() const
{
	return m_row;
}

void GenModule::loadInitialState()
{
	logger()->info(L4CXX_LOCATION, "Loading initial state");
	m_initialState->archive( this ).finishLoad();
}

void GenModule::saveInitialState()
{
	logger()->info(L4CXX_LOCATION, "Storing initial state");
	m_initialState->archive( this ).finishSave();
}

bool GenModule::jumpNextOrder()
{
	IAudioSource::LockGuard guard( this );
	IArchive::Ptr next = multiSongAt( currentSongIndex() ).nextState();
	if( next ) {
		logger()->debug(L4CXX_LOCATION, "Already preprocessed - loading");
		next->archive( this ).finishLoad();
		return true;
	}
	// maybe not processed yet, so try to jump to the next order...
	logger()->debug(L4CXX_LOCATION, "Not preprocessed yet");
	size_t ord = order();
	setBusy(true);
	do {
		AudioFrameBuffer buf;
		buildTick(&buf);
		if(!buf) {
			return false;
		}
	} while(ord == order());
	setBusy(false);
	return true;
}

bool GenModule::jumpPrevOrder()
{
	IAudioSource::LockGuard guard( this );
	IArchive::Ptr next = multiSongAt( currentSongIndex() ).prevState();
	if( !next ) {
		return false;
	}
	next->archive( this ).finishLoad();
	return true;
}

bool GenModule::isBusy() const
{
	return IAudioSource::isBusy();
}

void GenModule::setBusy( bool value )
{
	IAudioSource::setBusy(value);
}

void GenModule::notifyOrderChanged( bool estimateOnly )
{
	try {
		if(!estimateOnly) {
			IAudioSource::TryLockGuard guard( this );
			IArchive::Ptr state = multiSongAt( currentSongIndex() ).nextState();
			if(state) {
				state->archive( this ).finishLoad();
			}
			else {
				multiSongAt( currentSongIndex() ).newState()->archive( this ).finishSave();
				multiSongAt( currentSongIndex() ).nextState();
			}
		}
		logger()->info(L4CXX_LOCATION, boost::format("Order changed to %d (pattern %d)") % order() % patternIndex() );
	}
	catch( ... ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
}

}

/**
 * @}
 */
