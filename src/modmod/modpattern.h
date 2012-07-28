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

class ModCell;

class ModPattern
{
	DISABLE_COPY( ModPattern )
private:
	std::vector<std::vector<ModCell*>> m_channels; //!< @brief Channels in this pattern
	/**
	 * @brief Create a cell
	 * @param[in] chanIdx Channel index
	 * @param[in] row Row index
	 * @return Pointer to the new cell
	 */
	ModCell* createCell( uint16_t chanIdx, int16_t row );
public:
	/**
	 * @brief Constructor
	 */
	ModPattern();
	~ModPattern();
	/**
	 * @brief Load the pattern from a stream
	 * @param[in] str The stream to load from
	 * @return @c true on success
	 */
	bool load( Stream* str, uint8_t numChans );
	/**
	 * @brief Get a cell from the pattern
	 * @param[in] chanIdx Channel index
	 * @param[in] row Row index
	 * @return Pointer to the cell or nullptr
	 */
	ModCell* cellAt( uint16_t chanIdx, int16_t row );
};

}
}

/**
 * @}
 */

#endif
