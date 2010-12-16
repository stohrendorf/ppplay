/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef genpatternH
#define genpatternH

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
			 * @brief Constructor, sets aActive = @c false
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
			 * @return #aActive
			 */
			bool isActive() const throw();
			/**
			 * @brief Get the tracker-like string representation of this cell
			 * @return Tracker-like string
			 */
			virtual std::string trackerString() const throw() = 0;
			virtual IArchive& serialize(IArchive* data);
			void setActive(bool a) throw() { m_active = a; }
	};

} // namespace ppp

#endif
