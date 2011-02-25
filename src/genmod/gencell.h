/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#ifndef GENCELL_H
#define GENCELL_H

/**
 * @file
 * @ingroup GenMod
 * @brief General Pattern and Track definitions
 */

#include "genbase.h"

namespace ppp {

	/**
	 * @class GenCell
	 * @ingroup GenMod
	 * @brief A single note cell
	 */
	class GenCell : public ISerializable {
		public:
			typedef std::shared_ptr<GenCell> Ptr; //!< @brief Class pointer
			typedef std::vector<Ptr> Vector;
		private:
			bool m_active; //!< @brief Is this cell used/relevant?
		public:
			/**
			 * @brief Constructor, sets m_active to @c false
			 */
			GenCell() throw();
			/**
			 * @brief Destructor, does nothing
			 */
			virtual ~GenCell();
			/**
			 * @brief Reset the cell so that it is practically "unused"
			 */
			virtual void reset() throw();
			/**
			 * @brief Is this cell active/used?
			 * @return m_active
			 */
			bool isActive() const throw();
			/**
			 * @brief Get the tracker-like string representation of this cell
			 * @return Tracker-like string
			 */
			virtual std::string trackerString() const throw() = 0;
			virtual IArchive& serialize( IArchive* data );
			void setActive( bool a ) throw() {
				m_active = a;
			}
	};

} // namespace ppp

#endif
