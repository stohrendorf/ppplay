/*
    PPPlay - an old-fashioned module player
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
#include "stuff/field.h"
#include "s3mcell.h"

class Stream;

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
  : public Field<S3mCell>
{
public:
  DISABLE_COPY( S3mPattern )

  //! @brief Constructor
  explicit S3mPattern();

  /**
   * @brief Load the cell from a stream
   * @param[in] str The stream to load from
   * @param[in] pos Position within @a str
   * @return @c true on success
   */
  bool load(Stream* str, size_t pos);

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
