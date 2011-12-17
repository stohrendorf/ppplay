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

#ifndef POINT_H
#define POINT_H

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
class Point
{
private:
	int m_x; //!< @brief X position
	int m_y; //!< @brief Y position
public:
	/**
	 * @brief Default constructor, sets m_x and m_y to 0
	 */
	Point();
	/**
	 * @overload
	 * @brief Initializing constructor
	 * @param[in] x Value for m_x
	 * @param[in] y Value for m_y
	 */
	Point( int x, int y );
	/**
	 * @brief Adds the coordinates of @a rhs to this
	 * @param[in] rhs The coordinates to add
	 * @return Reference to *this
	 */
	const Point& operator+=( const Point& rhs );
	/**
	 * @brief Add two points
	 * @param[in] rhs The coordinates to add
	 * @return New Point
	 */
	const Point operator+( const Point& rhs ) const;
	/**
	 * @brief Subtracts the coordinates of @a rhs from this
	 * @param[in] rhs The coordinates to subtract
	 * @return Reference to *this
	 */
	const Point& operator-=( const Point& rhs );
	/**
	 * @brief Subtract two points
	 * @param[in] rhs The coordinates to subtract
	 * @return New Point
	 */
	const Point operator-( const Point& rhs ) const;
	/**
	 * @brief Get the X coordinate
	 * @return The X coordinate
	 */
	int x() const;
	/**
	 * @brief Get the Y coordinate
	 * @return The Y coordinate
	 */
	int y() const;
	/**
	 * @brief Set the X coordinate
	 * @param[in] x The new X coordinate
	 */
	void setX( int x );
	/**
	 * @brief Set the Y coordinate
	 * @param[in] y The new Y coordinate
	 */
	void setY( int y );
};

}

/**
 * @}
 */

#endif
