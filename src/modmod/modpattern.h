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

#ifndef MODPATTERN_H
#define MODPATTERN_H

#include "stuff/utils.h"
#include "stuff/field.h"

#include "modcell.h"

#include <vector>
#include <cstdint>

class Stream;

/**
 * @ingroup ModMod
 * @{
 */

namespace ppp
{
namespace mod
{

class ModPattern : public Field<ModCell>
{
	DISABLE_COPY( ModPattern )
public:
	/**
	 * @brief Constructor
	 */
	ModPattern();
	/**
	 * @brief Load the pattern from a stream
	 * @param[in] str The stream to load from
	 * @return @c true on success
	 */
	bool load( Stream* str, uint8_t numChans );
};

}
}

/**
 * @}
 */

#endif
