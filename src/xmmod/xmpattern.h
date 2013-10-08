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

#ifndef XMPATTERN_H
#define XMPATTERN_H

#include "stuff/utils.h"
#include "stuff/field.h"
#include "light4cxx/logger.h"
#include "xmcell.h"

/**
 * @ingroup XmModule
 * @{
 */

class Stream;

namespace ppp
{
namespace xm
{

/**
 * @class XmPattern
 * @brief XM pattern storage class
 */
class XmPattern : public Field<XmCell>
{
    DISABLE_COPY( XmPattern )
public:
    /**
     * @brief Constructor
     * @param[in] chans Number of channels/columns needed
     */
    explicit XmPattern( int16_t chans );
    /**
     * @brief Load the pattern from a stream
     * @param[in] str Stream to load from
     * @return @c true on success
     */
    bool load( Stream* str );
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
