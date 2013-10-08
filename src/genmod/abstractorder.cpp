/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "abstractorder.h"

#include "stream/abstractarchive.h"

/**
 * @ingroup GenMod
 * @{
 */

namespace ppp
{

AbstractOrder::AbstractOrder( uint8_t idx ) noexcept :
m_index( idx ), m_playbackCount( 0 ), m_rowPlaybackCounter()
{ }

uint8_t AbstractOrder::index() const noexcept
{
    return m_index;
}

void AbstractOrder::setIndex( uint8_t index )
{
    m_index = index;
    m_rowPlaybackCounter.clear();
}

AbstractArchive& AbstractOrder::serialize( AbstractArchive* data )
{
    return *data % m_index % m_playbackCount;
}

int AbstractOrder::playbackCount() const noexcept
{
    return m_playbackCount;
}

int AbstractOrder::increasePlaybackCount() noexcept {
    return ++m_playbackCount;
}

uint8_t AbstractOrder::increaseRowPlayback( std::size_t row )
{
    if( row >= m_rowPlaybackCounter.size() ) {
        m_rowPlaybackCounter.resize( row + 1, 0 );
    }
    return ++m_rowPlaybackCounter[row];
}

void AbstractOrder::resetRowPlaybackCounter()
{
    m_rowPlaybackCounter.clear();
}


light4cxx::Logger* AbstractOrder::logger()
{
    return light4cxx::Logger::get( "order" );
}

}

/**
 * @}
 */
