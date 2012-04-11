/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef SONGINFO_H
#define SONGINFO_H

#include "stream/stateiterator.h"

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

/**
 * @struct SongInfo
 * @brief Information about a sub-song within a module
 */
struct SongInfo
{
	explicit SongInfo(const StateIterator& it) : states(it), length(0)
	{
	}
	
	//! @brief States for seeking
	StateIterator states;
	//! @brief Length in sample frames
	size_t length;
};

/**
 * @}
 */

}

#endif
