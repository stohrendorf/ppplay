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

#ifndef S3MPATTERN_H
#define S3MPATTERN_H

/**
 * @ingroup S3mMod
 * @{
 */

#include "light4cxx/logger.h"

class BinStream;

namespace ppp
{
namespace s3m
{

class S3mCell;

/**
 * @class S3mPattern
 * @brief Pattern class for S3M Patterns
 */
class S3mPattern
{
	DISABLE_COPY( S3mPattern )
private:
	std::vector<std::vector<S3mCell*>> m_channels; //!< @brief Channels in this pattern
	/**
	 * @brief Create a cell
	 * @param[in] chanIdx Channel index
	 * @param[in] row Row index
	 * @return Pointer to the new cell
	 */
	S3mCell* createCell( uint16_t chanIdx, int16_t row );
public:
	//! @brief Constructor
	S3mPattern();
	~S3mPattern();
	/**
	 * @brief Load the cell from a stream
	 * @param[in] str The stream to load from
	 * @param[in] pos Position within @a str
	 * @return @c true on success
	 */
	bool load( BinStream& str, size_t pos );
	/**
	 * @brief Get a cell from the pattern
	 * @param[in] chanIdx Channel index
	 * @param[in] row Row index
	 * @return Pointer to the cell or nullptr
	 */
	S3mCell* cellAt( uint16_t chanIdx, int16_t row );
protected:
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".s3m"
	 */
	static light4cxx::Logger* logger();
};

} // namespace s3m
} // namespace ppp

/**
 * @}
 */

#endif
