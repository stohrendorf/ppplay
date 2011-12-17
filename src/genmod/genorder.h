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

#ifndef GENORDER_H
#define GENORDER_H

/**
 * @ingroup GenMod
 * @{
 */

#include "stream/iserializable.h"
#include "stuff/utils.h"

#include "light4cxx/logger.h"

#include <memory>

namespace ppp
{

/**
 * @class GenOrder
 * @brief An order list item
 */
class GenOrder : public ISerializable
{
	DISABLE_COPY( GenOrder )
	GenOrder() = delete;
public:
	typedef std::shared_ptr<GenOrder> Ptr; //!< @brief Class pointer
	typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
private:
	uint8_t m_index; //!< @brief Pattern index of this order
	uint8_t m_playbackCount; //!< @brief Playback count of this order
public:
	/**
	 * @brief Constructor
	 * @param[in] idx Order index
	 */
	GenOrder( uint8_t idx );
	/**
	 * @brief Return the pattern index associated with this order
	 * @return m_index
	 */
	uint8_t index() const;
	/**
	 * @brief Set the pattern index
	 * @param[in] n New index
	 */
	void setIndex( uint8_t n );
	virtual IArchive& serialize( IArchive* data );
	/**
	 * @brief Get the playback count of this order
	 * @return m_playbackCount
	 */
	uint8_t playbackCount() const;
	/**
	 * @brief Increase the playback count
	 * @return The new value of m_playbackCount
	 */
	uint8_t increasePlaybackCount();
protected:
	/**
	 * @brief Get the logger
	 * @return Logger with name "order"
	 */
	static light4cxx::Logger::Ptr logger();
};

}

/**
 * @}
 */

#endif
