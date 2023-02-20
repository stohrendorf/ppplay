/*
    PPPlay - an old-fashioned module player
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

#ifndef PPG_RECT_H
#define PPG_RECT_H

#include "point.h"

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg
{
/**
 * @class Rect
 * @brief Represents an area on screen or in a widget
 */
class Rect
{
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
  constexpr Rect(int x, int y, int width, int height) noexcept
    :
    m_topLeft( x, y ), m_bottomRight( x + width - 1, y + height - 1 )
  {
  }

  /**
   * @brief Get the top coordinate
   * @return The top coordinate
   */
  constexpr int top() const noexcept
  {
    return m_topLeft.y();
  }

  /**
   * @brief Set the top coordinate
   * @param[in] top The top coordinate
   */
  inline void setTop(int top) noexcept
  {
    m_topLeft.setY( top );
  }

  /**
   * @brief Get the left coordinate
   * @return The left coordinate
   */
  constexpr int left() const noexcept
  {
    return m_topLeft.x();
  }

  /**
   * @brief Set the left coordinate
   * @param[in] left The left coordinate
   */
  inline void setLeft(int left) noexcept
  {
    m_topLeft.setX( left );
  }

  /**
   * @brief Get the bottom coordinate
   * @return The bottom coordinate
   */
  constexpr int bottom() const noexcept
  {
    return m_bottomRight.y();
  }

  /**
   * @brief Set the bottom coordinate
   * @param[in] bottom The bottom coordinate
   */
  inline void setBottom(int bottom) noexcept
  {
    m_bottomRight.setY( bottom );
  }

  /**
   * @brief Get the right coordinate
   * @return The right coordinate
   */
  constexpr int right() const noexcept
  {
    return m_bottomRight.x();
  }

  /**
   * @brief Set the right coordinate
   * @param[in] right The right coordinate
   */
  inline void setRight(int right) noexcept
  {
    m_bottomRight.setX( right );
  }

  /**
   * @brief Get the area's width
   * @return The area's width
   */
  constexpr int width() const noexcept
  {
    return m_bottomRight.x() - m_topLeft.x() + 1;
  }

  /**
   * @brief Set the area's width
   * @param[in] width The new area's width
   */
  inline void setWidth(int width) noexcept
  {
    m_bottomRight.setX( m_topLeft.x() + width - 1 );
  }

  /**
   * @brief Get the area's height
   * @return The area's height
   */
  constexpr int height() const noexcept
  {
    return m_bottomRight.y() - m_topLeft.y() + 1;
  }

  /**
   * @brief Set the area's height
   * @param[in] height The new area's height
   */
  inline void setHeight(int height) noexcept
  {
    m_bottomRight.setY( m_topLeft.y() + height - 1 );
  }

  /**
   * @brief Get the top left point
   * @return The top left point
   */
  constexpr Point topLeft() const noexcept
  {
    return m_topLeft;
  }

  /**
   * @overload
   * @return The top left point
   */
  inline Point& topLeft() noexcept
  {
    return m_topLeft;
  }

  /**
   * @brief Get the bottom right point
   * @return The bottom right point
   */
  constexpr Point bottomRight() const noexcept
  {
    return m_bottomRight;
  }

  /**
   * @overload
   * @return The bottom right point
   */
  inline Point& bottomRight() noexcept
  {
    return m_bottomRight;
  }

  /**
   * @brief Get the area's size
   * @return The area's size
   */
  constexpr Point size() const noexcept
  {
    return { width(), height() };
  }

  /**
   * @brief Check if this rect contains a point
   * @param[in] pt Point to check
   * @return @c true if the area contains the point
   */
  constexpr bool contains(Point pt) const noexcept
  {
    return contains( pt.x(), pt.y() );
  }

  /**
   * @overload
   * @param[in] x X coordinate of the point
   * @param[in] y Y coordinate of the point
   * @return @c true if the area contains the point
   */
  constexpr bool contains(int x, int y) const noexcept
  {
    return x >= m_topLeft.x() && x <= m_bottomRight.x() && y >= m_topLeft.y() && y <= m_bottomRight.y();
  }
};
}

/**
 * @}
 */

#endif
