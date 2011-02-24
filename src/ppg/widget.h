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

	class Point {
		private:
			int m_x;
			int m_y;
		public:
			Point();
			Point( int x, int y );
			const Point& operator+=( const Point& rhs );
			const Point operator+( const Point& rhs ) const;
			const Point& operator-=( const Point& rhs );
			const Point operator-( const Point& rhs ) const;
			int x() const;
			int y() const;
			void setX( int x );
			void setY( int y );
	};

	class Rect {
		private:
			Point m_topLeft;
			Point m_bottomRight;
		public:
			Rect( int x, int y, int width, int height );
			int top() const;
			void setTop( int top );
			int left() const;
			void setLeft( int left );
			int bottom() const;
			void setBottom( int bottom );
			int right() const;
			void setRight( int right );
			int width() const;
			void setWidth( int width );
			int height() const;
			void setHeight( int height );
			Point topLeft() const;
			Point& topLeft();
			Point bottomRight() const;
			Point& bottomRight();
			Point size() const;
			bool contains( const Point& pt ) const {
				return contains( pt.x(), pt.y() );
			}
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
			List m_children; //!< @brief Children within this container
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
			 * @brief Sets the widget's position
			 * @param[in] pos Position
			 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
			 * @return @c false if the position did not change
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
			 * @brief Sets the widget's size
			 * @param[in] pt Size
			 * @return @c false if nothing changed
			 */
			virtual bool setSize( const Point& pt ) throw( Exception );
			Rect area() const;
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
			 * @brief Map widget's coordinates to the parent ones
			 * @param[in,out] pt Coordinates to map
			 */
			virtual void mapToParent( Point* pt ) const;
			/**
			 * @brief Map widget's coordinates to the top parent ones
			 * @param[in,out] x Left coordinate
			 * @param[in,out] y Top coordinate
			 */
			virtual void mapToAbsolute( int* x, int* y ) const;
			/**
			 * @brief Map widget's coordinates to the top parent ones
			 * @param[in,out] pt Coordinates to map
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
