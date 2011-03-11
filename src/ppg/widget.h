/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#ifndef WIDGET_H
#define WIDGET_H

#include "ppgexcept.h"
#include "stuff/utils.h"

#include <list>

namespace ppg {

	/**
	 * @defgroup Ppg The PeePeeGUI Definitions
	 * @brief This module contains definitions for the PeePeeGUI User Interface
	 */

	/**
	 * @class Point
	 * @ingroup Ppg
	 * @brief Represents a point on screen or in a widget
	 */
	class Point {
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

	/**
	 * @class Rect
	 * @ingroup Ppg
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
			Rect( int x, int y, int width, int height );
			/**
			 * @brief Get the top coordinate
			 * @return The top coordinate
			 */
			int top() const;
			/**
			 * @brief Set the top coordinate
			 * @param[in] top The top coordinate
			 */
			void setTop( int top );
			/**
			 * @brief Get the left coordinate
			 * @return The left coordinate
			 */
			int left() const;
			/**
			 * @brief Set the left coordinate
			 * @param[in] left The left coordinate
			 */
			void setLeft( int left );
			/**
			 * @brief Get the bottom coordinate
			 * @return The bottom coordinate
			 */
			int bottom() const;
			/**
			 * @brief Set the bottom coordinate
			 * @param[in] bottom The bottom coordinate
			 */
			void setBottom( int bottom );
			/**
			 * @brief Get the right coordinate
			 * @return The right coordinate
			 */
			int right() const;
			/**
			 * @brief Set the right coordinate
			 * @param[in] right The right coordinate
			 */
			void setRight( int right );
			/**
			 * @brief Get the area's width
			 * @return The area's width
			 */
			int width() const;
			/**
			 * @brief Set the area's width
			 * @param[in] width The new area's width
			 */
			void setWidth( int width );
			/**
			 * @brief Get the area's height
			 * @return The area's height
			 */
			int height() const;
			/**
			 * @brief Set the area's height
			 * @param[in] height The new area's height
			 */
			void setHeight( int height );
			/**
			 * @brief Get the top left point
			 * @return The top left point
			 */
			Point topLeft() const;
			/**
			 * @overload
			 */
			Point& topLeft();
			/**
			 * @brief Get the bottom right point
			 * @return The bottom right point
			 */
			Point bottomRight() const;
			/**
			 * @overload
			 */
			Point& bottomRight();
			/**
			 * @brief Get the area's size
			 * @return The area's size
			 */
			Point size() const;
			/**
			 * @brief Check if this rect contains a point
			 * @return @c true if the area contains the point
			 */
			bool contains( const Point& pt ) const {
				return contains( pt.x(), pt.y() );
			}
			/**
			 * @overload
			 */
			bool contains( int x, int y ) const {
				return x >= m_topLeft.x() && x <= m_bottomRight.x() && y >= m_topLeft.y() && y <= m_bottomRight.y();
			}
	};

	/**
	 * @class Widget
	 * @ingroup Ppg
	 * @brief The abstract base class for all PeePeeGUI classes
	 * @details
	 * Every PeePeeGUI class is derived from this class.
	 */
	class Widget {
			DISABLE_COPY( Widget )
		public:
			typedef std::list<Widget*> List; //!< @brief List of widgets
		private:
			bool m_visible; //!< @brief @c false if this widget and it's children should not be drawn
			Widget* m_parent; //!< @brief Pointer to the parent widget, or @c NULL if it's the top widget
			Rect m_area; //!< @brief Area of this widget within the parent's space
			List m_children; //!< @brief Children within this widget
			virtual void drawThis() throw( Exception ) = 0; //!< @brief Internal drawing method, called by PppWidet::draw() @see draw()
		public:
			/**
			 * @brief Value that marks a non-changing color
			 */
			static const int ESC_NOCHANGE = 0xff;
			/**
			 * @brief Constructor
			 * @param[in] parent Parent widget
			 */
			explicit Widget( Widget* parent );
			/**
			 * @brief Destructor. Deletes all children.
			 */
			virtual ~Widget() throw();
			/**
			 * @brief Calls ppg::Widget::drawThis(), but only when ppg::Widget::m_visible is @c true
			 */
			void draw() throw( Exception );
			/**
			 * @brief Make this widget visible
			 */
			void show() throw();
			/**
			 * @brief Make this widget invisible
			 */
			void hide() throw();
			/**
			 * @brief Draw a char relative to the widget's position
			 * @param[in] x Left position
			 * @param[in] y Top position
			 * @param[in] c The char to draw
			 */
			virtual void drawChar( int x, int y, char c ) throw();
			/**
			 * @brief Is this widget visible?
			 * @return ppg::Widget::m_visible
			 */
			bool isVisible() const throw();
			/**
			 * @brief Sets the foreground color relative to the widget's position
			 * @param[in] x Left position
			 * @param[in] y Top position
			 * @param[in] c Dos Color Value
			 */
			virtual void drawFgColor( int x, int y, uint8_t c ) throw();
			/**
			 * @brief Sets the background color relative to the widget's position
			 * @param[in] x Left position
			 * @param[in] y Top position
			 * @param[in] c Dos Color Value
			 */
			virtual void drawBgColor( int x, int y, uint8_t c ) throw();
			/**
			 * @brief Sets the widget's left position
			 * @param[in] x Left position
			 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
			 * @return New left position
			 */
			virtual int setLeft( int x, bool absolute = false ) throw();
			/**
			 * @brief Sets the widget's top position
			 * @param[in] y Top position
			 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
			 * @return New top position
			 */
			virtual int setTop( int y, bool absolute = false ) throw();
			/**
			 * @brief Sets the widget's position
			 * @param[in] x Left position
			 * @param[in] y Top position
			 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
			 * @return @c false if the position did not change
			 */
			virtual bool setPosition( int x, int y, bool absolute = false ) throw();
			/**
			 * @overload
			 */
			virtual bool setPosition( const Point& pos, bool absolute = false );
			/**
			 * @brief Sets the widget's width
			 * @param[in] w Wanted width
			 * @return New width
			 * @exception PpgException is thrown if @c w\<=0
			 */
			virtual int setWidth( int w ) throw( Exception );
			/**
			 * @brief Sets the widget's height
			 * @param[in] h Wanted height
			 * @return New height
			 * @exception PpgException is thrown if @c h\<=0
			 */
			virtual int setHeight( int h ) throw( Exception );
			/**
			 * @brief Sets the widget's size
			 * @param[in] w Width
			 * @param[in] h Height
			 * @return @c false if nothing changed
			 */
			virtual bool setSize( int w, int h ) throw( Exception );
			/**
			 * @overload
			 */
			virtual bool setSize( const Point& pt ) throw( Exception );
			/**
			 * @brief Get the widget's area
			 * @return The widget's area
			 */
			Rect area() const;
			/**
			 * @overload
			 */
			Rect& area();
			/**
			 * @brief Get the top parent
			 * @return Pointer to the top parent
			 */
			virtual Widget* getTopParent() const;
			/**
			 * @brief Map widget's coordinates to the parent ones
			 * @param[in,out] x Left coordinate
			 * @param[in,out] y Top coordinate
			 */
			virtual void mapToParent( int* x, int* y ) const;
			/**
			 * @overload
			 */
			virtual void mapToParent( Point* pt ) const;
			/**
			 * @brief Map widget's coordinates to the top parent ones
			 * @param[in,out] x Left coordinate
			 * @param[in,out] y Top coordinate
			 */
			virtual void mapToAbsolute( int* x, int* y ) const;
			/**
			 * @overload
			 */
			virtual void mapToAbsolute( Point* pt ) const;
			/**
			 * @brief Move an element to the top
			 * @param[in] vp Pointer to the element to move to the top
			 */
			virtual void toTop( Widget* vp ) throw();
			/**
			 * @brief Mouse movement event handler
			 * @param[in] x X coordinate
			 * @param[in] y Y coordinate
			 * @return @c true if the event was handled
			 * @note Coordinates are relative to the parent's ones
			 * @details
			 * The default implementation calls the children's event handlers
			 */
			virtual bool onMouseMove( int x, int y );
	};

} // namespace ppg

#endif // ppgwidgetH
