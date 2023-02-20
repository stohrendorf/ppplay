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

#include "orderentry.h"

#include "stream/abstractarchive.h"

/**
 * @ingroup GenMod
 * @{
 */

namespace ppp
{
void OrderEntry::setIndex(uint8_t index)
{
  m_index = index;
  m_rowPlaybackCounter.clear();
}

AbstractArchive& OrderEntry::serialize(AbstractArchive* data)
{
  return *data % m_index % m_playbackCount;
}

uint8_t OrderEntry::increaseRowPlayback(std::size_t row)
{
  if( row >= m_rowPlaybackCounter.size() )
  {
    m_rowPlaybackCounter.resize( row + 1, 0 );
  }
  return ++m_rowPlaybackCounter[row];
}

light4cxx::Logger* OrderEntry::logger()
{
  return light4cxx::Logger::get( "order" );
}
}

/**
 * @}
 */
