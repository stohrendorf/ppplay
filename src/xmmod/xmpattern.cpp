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

/**
 * @ingroup XmModule
 * @{
 */

#include "xmpattern.h"
#include "stream/stream.h"

#include <boost/assert.hpp>
#include <boost/format.hpp>

namespace ppp
{
namespace xm
{
XmPattern::XmPattern(int16_t chans)
    : Field<XmCell>(chans, 1)
{
}

bool XmPattern::load(Stream* str)
{
    logger()->trace(L4CXX_LOCATION, "Start: %#x", str->pos());
    uint32_t hdrLen;
    *str >> hdrLen;
    logger()->trace(L4CXX_LOCATION, "hdrLen=%d", hdrLen);
    uint8_t packType;
    *str >> packType;
    if( packType != 0 )
    {
        logger()->error(L4CXX_LOCATION, "Unsupported Pattern pack type: %d", int(packType));
        return false;
    }
    uint16_t rows;
    *str >> rows;
    if( rows == 0 )
    {
        // create a 64-row default pattern
        logger()->debug(L4CXX_LOCATION, "Number of rows = 0, creating 64-rows default pattern.");
        reset(width(), 64);
        return true;
    }
    else if( rows < 1 || rows > 256 )
    {
        logger()->warn(L4CXX_LOCATION, "Number of rows out of range: %d", rows);
        return false;
    }
    reset(width(), rows);
    uint16_t packedSize;
    *str >> packedSize;
    logger()->trace(L4CXX_LOCATION, "Header end: %#x", str->pos());
    str->seekrel(hdrLen - 9); // copied from schismtracker
    if( packedSize == 0 )
    {
        return true;
    }
    for( uint16_t row = 0; row < rows; row++ )
    {
        for( uint16_t chan = 0; chan < width(); chan++ )
        {
            if( !at(chan, row).load(str) )
            {
                return false;
            }
        }
    }
    return str->good();
}

std::unique_ptr<XmPattern> XmPattern::createDefaultPattern(int16_t chans)
{
    auto result = std::make_unique<XmPattern>(1);
    result->reset(chans, 64);
    return result;
}

light4cxx::Logger* XmPattern::logger()
{
    return light4cxx::Logger::get("pattern.xm");
}
}
}

/**
 * @}
 */