/*
    PeePeePlayer - an old-fashioned module player
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

#include "genorder.h"

#include "stream/iarchive.h"

/**
 * @ingroup GenMod
 * @{
 */

namespace ppp {

GenOrder::GenOrder(uint8_t idx) : m_index(idx), m_playbackCount(0)
{ }

uint8_t GenOrder::index() const {
	return m_index;
}

void GenOrder::setIndex(uint8_t n) {
	m_index = n;
}

IArchive& GenOrder::serialize(IArchive* data) {
	return *data % m_index % m_playbackCount;
}

uint8_t GenOrder::playbackCount() const
{
	return m_playbackCount;
}

uint8_t GenOrder::increasePlaybackCount()
{
	// prevent overflow
	BOOST_ASSERT( m_playbackCount<0xff );
	return ++m_playbackCount;
}

}

/**
 * @}
 */
