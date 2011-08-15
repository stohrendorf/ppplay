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

#ifndef IPATTERNCELL_H
#define IPATTERNCELL_H

/**
 * @ingroup GenMod
 * @{
 */

#include "stream/iserializable.h"

#include <log4cxx/logger.h>

#include <vector>
#include <string>
#include <memory>

namespace ppp {

/**
 * @interface IPatternCell
 * @ingroup GenMod
 * @brief General interface for pattern note cells
 */
class IPatternCell : public ISerializable {
	public:
		typedef std::shared_ptr<IPatternCell> Ptr; //!< @brief Class pointer
		typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
		/**
		 * @brief Destructor
		 */
		virtual ~IPatternCell();
		/**
		 * @brief Clears the cell's data
		 */
		virtual void clear() = 0;
		/**
		 * @brief Get the tracker-like string representation of this cell
		 * @return Tracker-like string
		 */
		virtual std::string trackerString() const = 0;
	protected:
		static log4cxx::LoggerPtr logger();
};

} // namespace ppp

/**
 * @}
 */

#endif
