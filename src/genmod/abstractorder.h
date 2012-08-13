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

#ifndef PPPLAY_ABSTRACTORDER_H
#define PPPLAY_ABSTRACTORDER_H

#include "ppplay_core_export.h"

#include <stream/iserializable.h>
#include <light4cxx/logger.h>

#include <vector>

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

/**
 * @class AbstractOrder
 * @brief An order list item
 */
class PPPLAY_CORE_EXPORT AbstractOrder : public ISerializable
{
	DISABLE_COPY( AbstractOrder )
	AbstractOrder() = delete;
private:
	//! @brief Pattern index of this order
	uint8_t m_index;
	//! @brief Playback count of this order
	int m_playbackCount;
	//! @brief Row playback counter to avoid infinite loops
	std::vector<uint8_t> m_rowPlaybackCounter;
public:
	/**
	 * @brief Constructor
	 * @param[in] idx Order index
	 * @param[in] rowCount Pattern row count
	 */
	AbstractOrder( uint8_t idx );
	/**
	 * @brief Return the pattern index associated with this order
	 * @return m_index
	 */
	uint8_t index() const;
	/**
	 * @brief Set the pattern index and pattern row count
	 * @param[in] index New index
	 * @param[in] rowCount Pattern row count
	 */
	void setIndex( uint8_t index );
	virtual AbstractArchive& serialize( AbstractArchive* data );
	/**
	 * @brief Get the playback count of this order
	 * @return m_playbackCount
	 */
	int playbackCount() const;
	/**
	 * @brief Increase the playback count
	 * @return The new value of m_playbackCount
	 */
	int increasePlaybackCount();
	
	/**
	 * @brief Resets the row playback counter
	 * @param[in] row Row for which the playback counter should be increased
	 * @return The new counter
	 */
	uint8_t increaseRowPlayback(std::size_t row);
	
	/**
	 * @brief Sets the row count for the row playback counter and resets the counter to 0
	 * @param[in] count The row count
	 */
	void resetRowPlaybackCounter();
	
	virtual bool isUnplayed() const = 0;
protected:
	/**
	 * @brief Get the logger
	 * @return Logger with name "order"
	 */
	static light4cxx::Logger* logger();
};

/**
 * @}
 */

}

extern template class std::vector<ppp::AbstractOrder*>;

#endif
