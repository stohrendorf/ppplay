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

#ifndef STATEITERATOR_H
#define STATEITERATOR_H

/**
 * @ingroup Common
 * @{
 */

#include "iarchive.h"

#include <memory>

/**
 * @class StateIterator
 * @brief A dynamically growing, indexed IArchive holder
 */
class StateIterator {
	public:
		typedef std::shared_ptr<StateIterator> Ptr; //!< @brief Class pointer
	private:
		IArchive::Vector m_states; //!< @brief The states
		std::size_t m_stateIndex; //!< @brief Current index of the iterator
	public:
		static const uint16_t stopHere = ~0; //!< @brief Const to define unused track
		/**
		 * @brief Constructor
		 */
		StateIterator();
		/**
		 * @brief Creates a new MemArchive and adds it to m_states
		 * @return The created archive
		 */
		IArchive::Ptr newState();
		/**
		 * @brief Increases m_stateIndex and returns the state pointer
		 * @return The state pointer or @c NULL if atEnd() returns @c true
		 */
		IArchive::Ptr nextState();
		/**
		 * @brief Decreases m_stateIndex and returns the state pointer
		 * @return The state pointer or @c NULL if atFront() returns @c true
		 */
		IArchive::Ptr prevState();
		/**
		 * @brief Returns the current state pointer
		 * @return The current state pointer or NULL
		 */
		IArchive::Ptr currentState() const;
		/**
		 * @brief Check if m_stateIndex is beyond the end of m_states
		 * @return @c true if m_stateIndex is at the end
		 */
		bool atEnd() const;
		/**
		 * @brief Check if m_stateIndex is before the front of m_states
		 * @return @c true if m_stateIndex is at the front
		 */
		bool atFront() const;
};

/**
 * @}
 */

#endif
