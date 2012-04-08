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

#ifndef MODULESTATE_H
#define MODULESTATE_H

#include "stream/iserializable.h"

#include <cstdint>
#include <cstddef>

/**
 * @ingroup GenMod
 * @{
 */

namespace ppp
{

/**
 * @struct ModuleState
 * @brief Contains information about the a playback state
 */
struct ModuleState : public ISerializable
{
	explicit constexpr ModuleState() :
		speed( 0 ),
		tempo( 0 ),
		order( 0 ),
		row( 0 ),
		tick( 0 ),
		globalVolume( 0x40 ),
		playedFrames( 0 ),
		pattern( 0 )
	{
	}
	virtual IArchive& serialize( IArchive* data );

	//! @brief Speed
	int16_t speed;
	//! @brief Tempo
	int16_t tempo;
	//! @brief Order
	size_t order;
	//! @brief Row
	int16_t row;
	//! @brief Tick index
	int16_t tick;
	//! @brief Global volume
	int16_t globalVolume;
	//! @brief Played Sample frames
	size_t playedFrames;
	//! @brief Pattern index of order
	size_t pattern;
};

}

/**
 * @}
 */

#endif
