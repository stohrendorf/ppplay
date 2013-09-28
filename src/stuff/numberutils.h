/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PPPLAY_NUMBERUTILS_H
#define PPPLAY_NUMBERUTILS_H

#include "ppplay_core_export.h"

#include <cstdint>
#include <algorithm>

namespace ppp
{
/**
 * @ingroup Common
 * @{
 */

/**
 * @brief Clip a value @a v in the range from @a a to @a b
 * @tparam T Value's type
 * @param[in] v Value to clip
 * @param[in] a Lower border
 * @param[in] b Upper border
 * @return Clipped value @a v between @a a and @a b
 * @note Time-critical
 */
template<typename T>
inline constexpr const T& clip( const T& v, const T& a, const T& b )
{
    return std::min( b, std::max( v, a ) );
}

/**
 * @brief Check if @a v is between @a a and @a b
 * @tparam T Value's type
 * @param[in] v Value
 * @param[in] a Lower border
 * @param[in] b Upper border
 * @return True if @a v is between @a a and @a b
 * @note Time-critical
 */
template<typename T>
inline constexpr bool between( const T v, const T a, const T b )
{
    return ( v >= a ) && ( v <= b );
}

/**
 * @brief Swap the bytes of @a data
 * @param[in,out] data Data to swap
 * @param[in] size Size of @a data
 * @details
 * Reverts the bytes in @a data
 */
extern PPPLAY_CORE_EXPORT void swapEndian( char data[], size_t size );

/**
 * @overload
 * @brief Swap the bytes of @a data
 * @tparam T Type of @a data
 * @param[in,out] data Data to swap
 */
template<class T>
inline void swapEndian( T* data )
{
    swapEndian( reinterpret_cast<char*>( data ), sizeof( T ) );
}

/**
 * @}
 */
}

#endif
