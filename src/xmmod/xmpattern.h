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

#ifndef XMPATTERN_H
#define XMPATTERN_H

#include "stuff/utils.h"
#include "light4cxx/logger.h"

/**
 * @ingroup XmModule
 * @{
 */

class BinStream;

namespace ppp
{
namespace xm
{

class XmCell;

/**
 * @class XmPattern
 * @brief XM pattern storage class
 */
class XmPattern
{
	DISABLE_COPY( XmPattern )
	XmPattern() = delete;
private:
	//! @brief Columns in the pattern
	std::vector<std::vector<XmCell*>> m_columns;
	/**
	 * @brief Create a cell if necessary
	 * @param[in] column Column index
	 * @param[in] row Row index
	 * @return Pointer to the cell
	 */
	XmCell* createCell( uint16_t column, uint16_t row );
public:
	/**
	 * @brief Constructor
	 * @param[in] chans Number of channels/columns needed
	 */
	XmPattern( int16_t chans );
	~XmPattern();
	/**
	 * @brief Load the pattern from a stream
	 * @param[in] str Stream to load from
	 * @return @c true on success
	 */
	bool load( BinStream& str );
	/**
	 * @brief Get a cell
	 * @param[in] column Column of the cell
	 * @param[in] row Row of the cell
	 * @return Pointer to the cell or nullptr
	 */
	XmCell* cellAt( uint16_t column, uint16_t row );
	/**
	 * @brief Number of rows in this pattern
	 * @return Number of rows
	 */
	size_t numRows() const;
	/**
	 * @brief Number of channels/columns in this pattern
	 * @return Number of channels
	 */
	size_t numChannels() const;
	/**
	 * @brief Create an empty default pattern with 64 rows
	 * @param[in] chans Number of channels for the new pattern
	 * @return Pointer to the new pattern
	 */
	static XmPattern* createDefaultPattern( int16_t chans );
protected:
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".xm"
	 */
	static light4cxx::Logger* logger();
};

} // namespace xm
} // namespace ppp

/**
 * @}
 */

#endif // XMPATTERN_H
