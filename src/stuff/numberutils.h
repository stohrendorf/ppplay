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

#ifndef NUMBERUTILS_H
#define NUMBERUTILS_H

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
inline constexpr bool inRange( const T v, const T a, const T b )
{
	return ( v >= a ) && ( v <= b );
}

/**
 * @brief Get low nibble of a byte
 * @param[in] x Value
 * @return Low nibble of @a x
 * @note Time-critical
 */
inline constexpr uint8_t lowNibble( uint8_t x )
{
	return x & 0x0f;
}

/**
 * @brief Get high nibble of a byte
 * @param[in] x Value
 * @return High nibble of @a x
 * @note Time-critical
 */
inline constexpr uint8_t highNibble( uint8_t x )
{
	return x >> 4;
}

/**
 * @brief Swap the bytes of @a data
 * @param[in,out] data Data to swap
 * @param[in] size Size of @a data
 * @details
 * Reverts the bytes in @a data
 */
void swapEndian( char data[], size_t size );

/**
 * @overload
 * @brief Swap the bytes of @a data
 * @tparam T Type of @a data
 * @param[in,out] data Data to swap
 */
template<class T>
void swapEndian( T* data )
{
	swapEndian( reinterpret_cast<char*>( data ), sizeof( T ) );
}

/**
 * @brief If one of the arguments is zero, set both to the other one
 * @param[in,out] oldFx If this is 0, use @a newFx
 * @param[in,out] newFx If this is 0, use @a oldFx
 */
inline void reuseIfZero( uint8_t& oldFx, uint8_t& newFx )
{
	if( newFx == 0 ) {
		newFx = oldFx;
	}
	else {
		oldFx = newFx;
	}
}

/**
 * @brief If @a newFx is not 0, assign it to @a oldFx
 * @param[in,out] oldFx If @a newFx is not 0, assign @a newFx to this
 * @param[in] newFx If this is not 0, assign it to @a oldFx
 */
inline void reuseIfZeroEx( uint8_t& oldFx, uint8_t newFx )
{
	if( newFx != 0 ) {
		oldFx = newFx;
	}
}

/**
 * @brief Works like reuseIfZero(), but uses nibbles instead
 * @param[in,out] oldFx Old data
 * @param[in,out] newFx New data
 */
inline void reuseNibblesIfZero( uint8_t& oldFx, uint8_t& newFx )
{
	if( newFx == 0 ) {
		newFx = oldFx;
	}
	else if( highNibble( newFx ) == 0 ) {
		oldFx = ( newFx & 0x0f ) | ( oldFx & 0xf0 );
	}
	else if( lowNibble( newFx ) == 0 ) {
		oldFx = ( newFx & 0xf0 ) | ( oldFx & 0x0f );
	}
	else {
		oldFx = newFx;
	}
	newFx = oldFx;
}

/**
 * @}
 */
}

#endif
