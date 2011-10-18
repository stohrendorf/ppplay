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

#ifndef RECT_H
#define RECT_H

#include "point.h"

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg {
/**
 * @class Rect
 * @brief Represents an area on screen or in a widget
 */
class Rect {
	private:
		Point m_topLeft; //!< @brief The top left point
		Point m_bottomRight; //!< @brief The bottom right point
	public:
		/**
		 * @brief Constructor
		 * @param[in] x Top left X coordinate
		 * @param[in] y Top left Y coordinate
		 * @param[in] width The width of the area
		 * @param[in] height The height of the area
		 */
		Rect(int x, int y, int width, int height);
		/**
		 * @brief Get the top coordinate
		 * @return The top coordinate
		 */
		int top() const;
		/**
		 * @brief Set the top coordinate
		 * @param[in] top The top coordinate
		 */
		void setTop(int top);
		/**
		 * @brief Get the left coordinate
		 * @return The left coordinate
		 */
		int left() const;
		/**
		 * @brief Set the left coordinate
		 * @param[in] left The left coordinate
		 */
		void setLeft(int left);
		/**
		 * @brief Get the bottom coordinate
		 * @return The bottom coordinate
		 */
		int bottom() const;
		/**
		 * @brief Set the bottom coordinate
		 * @param[in] bottom The bottom coordinate
		 */
		void setBottom(int bottom);
		/**
		 * @brief Get the right coordinate
		 * @return The right coordinate
		 */
		int right() const;
		/**
		 * @brief Set the right coordinate
		 * @param[in] right The right coordinate
		 */
		void setRight(int right);
		/**
		 * @brief Get the area's width
		 * @return The area's width
		 */
		int width() const;
		/**
		 * @brief Set the area's width
		 * @param[in] width The new area's width
		 */
		void setWidth(int width);
		/**
		 * @brief Get the area's height
		 * @return The area's height
		 */
		int height() const;
		/**
		 * @brief Set the area's height
		 * @param[in] height The new area's height
		 */
		void setHeight(int height);
		/**
		 * @brief Get the top left point
		 * @return The top left point
		 */
		Point topLeft() const;
		/**
		 * @overload
		 * @return The top left point
		 */
		Point& topLeft();
		/**
		 * @brief Get the bottom right point
		 * @return The bottom right point
		 */
		Point bottomRight() const;
		/**
		 * @overload
		 * @return The bottom right point
		 */
		Point& bottomRight();
		/**
		 * @brief Get the area's size
		 * @return The area's size
		 */
		Point size() const;
		/**
		 * @brief Check if this rect contains a point
		 * @param[in] pt Point to check
		 * @return @c true if the area contains the point
		 */
		bool contains(const Point& pt) const {
			return contains(pt.x(), pt.y());
		}
		/**
		 * @overload
		 * @param[in] x X coordinate of the point
		 * @param[in] y Y coordinate of the point
		 * @return @c true if the area contains the point
		 */
		bool contains(int x, int y) const {
			return x >= m_topLeft.x() && x <= m_bottomRight.x() && y >= m_topLeft.y() && y <= m_bottomRight.y();
		}
};

}

/**
 * @}
 */

#endif
