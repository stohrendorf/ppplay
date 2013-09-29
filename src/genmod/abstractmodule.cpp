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

#include "abstractmodule.h"

#include "genbase.h"
#include "abstractorder.h"
#include "channelstate.h"
#include "stream/memarchive.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>

namespace ppp
{

AbstractModule::AbstractModule( int maxRpt, Sample::Interpolation inter ) :
    m_metaInfo(),
    m_orders(),
    m_state(),
    m_songs(),
    m_maxRepeat( maxRpt ),
    m_initialState( new MemArchive() ),
    m_isPreprocessing( false ),
    m_mutex(),
    m_interpolation( inter )
{
    BOOST_ASSERT_MSG( maxRpt != 0, "Maximum repeat count may not be 0" );
}

AbstractModule::~AbstractModule()
{
    deleteAll( m_orders );
    deleteAll( m_songs );
}

AbstractArchive& AbstractModule::serialize( AbstractArchive* data )
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    for( AbstractOrder * order : m_orders ) {
        data->archive( order );
    }
    data->archive( &m_state );
    return *data;
}

uint32_t AbstractModule::timeElapsed() const
{
    return static_cast<uint32_t>( m_state.playedFrames / frequency() );
}

size_t AbstractModule::length() const
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    return m_songs->length;
}

size_t AbstractModule::internal_getAudioData( AudioFrameBuffer& buffer, size_t size )
{
    if( !buffer ) {
        buffer.reset( new AudioFrameBuffer::element_type );
    }
    buffer->reserve( size );
    if( m_isPreprocessing ) {
        buffer->clear();
        buffer->resize( tickBufferLength() );
        return tickBufferLength();
    }
    buffer->resize( 0 );
    AudioFrameBuffer tmpBuf;
    while( buffer->size() < size ) {
        size_t size = buildTick( &tmpBuf );
        if( !tmpBuf || tmpBuf->empty() || size == 0 ) {
            logger()->debug( L4CXX_LOCATION, "buildTick() returned 0" );
            return 0;
        }
        buffer->insert( buffer->end(), tmpBuf->begin(), tmpBuf->end() );
    }
    return buffer->size();
}

size_t AbstractModule::internal_preferredBufferSize() const
{
    return tickBufferLength();
}

void AbstractModule::addOrder( AbstractOrder* o )
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    m_orders.push_back( o );
}

AbstractOrder* AbstractModule::orderAt( size_t idx )
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    if( idx >= m_orders.size() ) {
        logger()->error( L4CXX_LOCATION, "Requested order index out of range: %d >= %d", idx, m_orders.size() );
        BOOST_THROW_EXCEPTION( std::out_of_range( "Requested order index out of range" ) );
    }
    return m_orders[idx];
}

size_t AbstractModule::orderCount() const noexcept
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    return m_orders.size();
}

int AbstractModule::maxRepeat() const noexcept
{
    return m_maxRepeat;
}

bool AbstractModule::setOrder( size_t newOrder, bool estimateOnly, bool forceSave )
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    bool orderChanged = ( newOrder != m_state.order );
    if( orderChanged && m_state.order < orderCount() ) {
        orderAt( m_state.order )->increasePlaybackCount();
    }
    if( newOrder != m_state.order && newOrder < orderCount() ) {
        orderAt( newOrder )->resetRowPlaybackCounter();
    }
    m_state.order = newOrder;
    if( newOrder >= orderCount() ) {
        return false;
    }
    m_state.pattern = orderAt( m_state.order )->index();
    if( orderChanged || forceSave ) {
        logger()->info( L4CXX_LOCATION, "Order change%s to %d (pattern %d, playback count %d)",
                        ( forceSave ? " (forced save)" : "" ),
                        m_state.order,
                        m_state.pattern,
                        orderAt( m_state.order )->playbackCount() );
        try {
            if( !estimateOnly ) {
                if( !m_songs->states.atEnd() ) {
                    m_songs->states.next()->archive( this ).finishLoad();
                }
                else {
                    m_songs->states.append( new MemArchive() )->archive( this ).finishSave();
                    m_songs->states.next();
                }
            }
        }
        catch( ... ) {
            BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
        }
    }
    return m_state.order < orderCount();
}

void AbstractModule::setRow( int16_t r ) noexcept {
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    m_state.row = r;
}

void AbstractModule::nextTick()
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    BOOST_ASSERT_MSG( m_state.speed != 0, "Data corruption: speed==0" );
    m_state.tick = ( m_state.tick + 1 ) % m_state.speed;
}

void AbstractModule::setTempo( uint8_t t ) noexcept {
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    if( t == 0 ) return;
    m_state.tempo = t;
}

void AbstractModule::setSpeed( uint8_t s ) noexcept {
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    if( s == 0 ) return;
    m_state.speed = s;
}

uint16_t AbstractModule::songCount() const noexcept
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    return m_songs.size();
}

int16_t AbstractModule::currentSongIndex() const
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    return m_songs.where();
}

uint16_t AbstractModule::tickBufferLength() const
{
    BOOST_ASSERT_MSG( m_state.tempo != 0, "Data corruption: tempo==0" );
    return frequency() * 5 / ( m_state.tempo << 1 );
}

light4cxx::Logger* AbstractModule::logger()
{
    return light4cxx::Logger::get( "module" );
}

void AbstractModule::loadInitialState()
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    logger()->info( L4CXX_LOCATION, "Loading initial state" );
    m_initialState->archive( this ).finishLoad();
}

void AbstractModule::saveInitialState()
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    logger()->info( L4CXX_LOCATION, "Storing initial state" );
    m_initialState->archive( this ).finishSave();
}

bool AbstractModule::jumpNextOrder()
{
    std::unique_lock<std::recursive_mutex> lock( m_mutex );
    if( !m_songs->states.atEnd() ) {
        logger()->debug( L4CXX_LOCATION, "Already preprocessed - loading" );
        m_songs->states.next()->archive( this ).finishLoad();
        return true;
    }
    lock.unlock();
    BOOST_ASSERT( !m_isPreprocessing );
    m_isPreprocessing = true;
    // maybe not processed yet, so try to jump to the next order...
    logger()->debug( L4CXX_LOCATION, "Not preprocessed yet" );
    size_t ord = m_state.order;
    do {
        if( m_state.order >= orderCount() ) {
            m_isPreprocessing = false;
            return false;
        }
        AudioFrameBuffer buf;
        if( internal_buildTick( &buf ) == 0 || !buf ) {
            m_isPreprocessing = false;
            return false;
        }
    }
    while( ord == m_state.order );
    m_isPreprocessing = false;
    return true;
}

bool AbstractModule::jumpPrevOrder()
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    if( !m_songs->states.atFront() && !m_songs->states.empty() ) {
        logger()->debug( L4CXX_LOCATION, "Jumping to previous order" );
        m_songs->states.prev()->archive( this ).finishLoad();
        return true;
    }
    else if( !m_songs->states.empty() ) {
        logger()->debug( L4CXX_LOCATION, "Resetting current order" );
        m_songs->states->archive( this ).finishLoad();
        return true;
    }
    else if( m_songs.atFront() ) {
        loadInitialState();
        return true;
    }
    logger()->info( L4CXX_LOCATION, "Failed to jump to previous order" );
    return false;
}

bool AbstractModule::jumpNextSong()
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    m_isPreprocessing = true;

    logger()->debug( L4CXX_LOCATION, "Trying to jump to next song" );
    if( !initialized() ) {
        for( size_t i = 0; i < orderCount(); i++ ) {
            if( !orderAt( i )->isUnplayed() ) {
                continue;
            }
            logger()->debug( L4CXX_LOCATION, "Found unplayed order %d pattern %d", i, int( orderAt( i )->index() ) );
            m_songs.append( new SongInfo );
            ++m_songs;
            m_state.playedFrames = 0;
            m_state.pattern = orderAt( i )->index();
            setOrder( i, false, true );
            m_isPreprocessing = false;
            return true;
        }
        m_isPreprocessing = false;
        return false;
    }
    if( m_songs.atEnd() ) {
        m_isPreprocessing = false;
        return false;
    }
    ++m_songs;
    m_songs->states.revert();
    m_songs->states.current()->archive( this ).finishLoad();
    m_state.pattern = orderAt( m_state.order )->index();
    m_isPreprocessing = false;
    return true;
}

bool AbstractModule::jumpPrevSong()
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    if( songCount() <= 1 ) {
        logger()->info( L4CXX_LOCATION, "This is not a multi-song" );
        return false;
    }
    if( m_songs.atFront() ) {
        logger()->info( L4CXX_LOCATION, "Already on first song" );
        return false;
    }
    --m_songs;
    m_songs->states.revert();
    m_songs->states.current()->archive( this ).finishLoad();
    return true;
}

bool AbstractModule::internal_initialize( uint32_t )
{
    if( initialized() ) {
        return true;
    }
    saveInitialState();

    logger()->info( L4CXX_LOCATION, "Calculating song lengths and preparing seek operations..." );
    while( jumpNextSong() ) {
        logger()->info( L4CXX_LOCATION, "Pre-processing song %d", m_songs.where() + 1 );
        while( size_t len = buildTick( nullptr ) ) {
            m_songs->length += len;
        }
        logger()->info( L4CXX_LOCATION, "Song preprocessed, trying to jump to the next song." );
    }
    logger()->info( L4CXX_LOCATION, "Lengths calculated, resetting module." );
    loadInitialState();
    m_songs.revert();
    return true;
}

size_t AbstractModule::buildTick( AudioFrameBuffer* buf )
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    return internal_buildTick( buf );
}

int AbstractModule::channelCount() const
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    return internal_channelCount();
}

ChannelState AbstractModule::channelStatus( size_t idx ) const
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    return internal_channelStatus( idx );
}

}

/**
 * @}
 */
