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

/**
 * @ingroup ModMod
 * @{
 */

#include "modcell.h"

namespace ppp {
namespace mod {

class ModPattern
{
	DISABLE_COPY(ModPattern)
	public:
		typedef std::shared_ptr<ModPattern> Ptr; //!< @brief Class pointer
		typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
	private:
		std::vector<ModCell::Vector> m_channels; //!< @brief Channels in this pattern
		/**
		 * @brief Create a cell
		 * @param[in] chanIdx Channel index
		 * @param[in] row Row index
		 * @return Pointer to the new cell
		 */
		ModCell::Ptr createCell(uint16_t chanIdx, int16_t row);
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
		bool load(BinStream& str, uint8_t numChans);
		/**
		 * @brief Get a cell from the pattern
		 * @param[in] chanIdx Channel index
		 * @param[in] row Row index
		 * @return Pointer to the cell or nullptr
		 */
		ModCell::Ptr cellAt(uint16_t chanIdx, int16_t row);
};

}
}

/**
 * @}
 */

#endif
