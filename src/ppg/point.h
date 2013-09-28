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

#ifndef PPG_POINT_H
#define PPG_POINT_H

#include <ppg/ppplay_ppg_export.h>

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg
{
/**
 * @class Point
 * @brief Represents a point on screen or in a widget
 */
class PPPLAY_PPG_EXPORT Point
{
private:
	int m_x; //!< @brief X position
	int m_y; //!< @brief Y position
public:
	/**
	 * @brief Default constructor, sets m_x and m_y to 0
	 */
	constexpr Point() noexcept : m_x( 0 ), m_y( 0 ) {
	}
	/**
	 * @overload
	 * @brief Initializing constructor
	 * @param[in] x Value for m_x
	 * @param[in] y Value for m_y
	 */
	constexpr Point( int x, int y ) noexcept : m_x( x ), m_y( y ) {
	}
	/**
	 * @brief Adds the coordinates of @a rhs to this
	 * @param[in] rhs The coordinates to add
	 * @return Reference to *this
	 */
    const Point& operator+=( const Point& rhs ) noexcept
    {
        m_x += rhs.m_x;
        m_y += rhs.m_y;
        return *this;
    }
	/**
	 * @brief Add two points
	 * @param[in] rhs The coordinates to add
	 * @return New Point
	 */
	constexpr const Point operator+( const Point& rhs ) const noexcept {
		return Point( m_x + rhs.m_x, m_y + rhs.m_y );
	}
	/**
	 * @brief Subtracts the coordinates of @a rhs from this
	 * @param[in] rhs The coordinates to subtract
	 * @return Reference to *this
	 */
    const Point& operator-=( const Point& rhs ) noexcept
    {
        m_x -= rhs.m_x;
        m_y -= rhs.m_y;
        return *this;
    }
    /**
	 * @brief Subtract two points
	 * @param[in] rhs The coordinates to subtract
	 * @return New Point
	 */
	constexpr const Point operator-( const Point& rhs ) const noexcept {
		return Point( m_x - rhs.m_x, m_y - rhs.m_y );
	}
	/**
	 * @brief Get the X coordinate
	 * @return The X coordinate
	 */
	constexpr int x() const noexcept {
		return m_x;
	}
	/**
	 * @brief Get the Y coordinate
	 * @return The Y coordinate
	 */
	constexpr int y() const noexcept {
		return m_y;
	}
	/**
	 * @brief Set the X coordinate
	 * @param[in] x The new X coordinate
	 */
	inline void setX( int x ) noexcept {
		m_x = x;
	}
	/**
	 * @brief Set the Y coordinate
	 * @param[in] y The new Y coordinate
	 */
	inline void setY( int y ) noexcept {
		m_y = y;
	}
};

}

/**
 * @}
 */

#endif
