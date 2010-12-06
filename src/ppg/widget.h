/***************************************************************************
 *   Copyright (C) 2009 by Syron                                         *
 *   mr.syron@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef ppgwidgetH
#define ppgwidgetH

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
		Point(int x, int y);
		const Point& operator+=(const Point& rhs);
		const Point operator+(const Point& rhs) const;
		const Point& operator-=(const Point& rhs);
		const Point operator-(const Point& rhs) const;
		int x() const;
		int y() const;
		void setX(int x);
		void setY(int y);
};

class Rect {
	private:
		Point m_topLeft;
		Point m_bottomRight;
	public:
		Rect(int x, int y, int width, int height);
		int top() const;
		void setTop(int top);
		int left() const;
		void setLeft(int left);
		int bottom() const;
		void setBottom(int bottom);
		int right() const;
		void setRight(int right);
		int width() const;
		void setWidth(int width);
		int height() const;
		void setHeight(int height);
		Point topLeft() const;
		Point& topLeft();
		Point bottomRight() const;
		Point& bottomRight();
		Point size() const;
		bool contains(const Point &pt) const { return contains(pt.x(), pt.y()); }
		bool contains(int x, int y) const { return x>=m_topLeft.x() && x<=m_bottomRight.x() && y>=m_topLeft.y() && y<=m_bottomRight.y(); }
};

/**
 * @class Widget
 * @ingroup Ppg
 * @brief The abstract base class for all PeePeeGUI classes
 * @details
 * Every PeePeeGUI class is derived from this class.
 */
class Widget {
		DISABLE_COPY(Widget)
	private:
		bool m_visible; //!< @brief @c false if this widget and it's children should not be drawn
		Widget *m_parent; //!< @brief Pointer to the parent widget, or @c NULL if it's the top widget
		Rect m_area;
		std::list<Widget*> m_children; //!< @brief Children within this container
		virtual void drawThis() throw(Exception) = 0; //!< @brief Internal drawing method, called by PppWidet::draw() @see draw()
	public:
		/**
		* @brief Value that marks a non-changing color
		 */
		static const int ESC_NOCHANGE = 0xff;
		/**
		 * @brief Constructor
		 * @param[in] name Name of this widget
		 */
		explicit Widget(Widget* parent);
		/**
		 * @brief Destructor. No operation.
		 */
		virtual ~Widget() throw();
		/**
		 * @brief Calls ppg::Widget::drawThis(), but only when ppg::Widget::aVisible is @c true
		 */
		void draw() throw(Exception);
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
		virtual void drawChar(int x, int y, char c) throw();
		/**
		 * @brief Is this widget visible?
		 * @return ppg::Widget::aVisible
		 */
		bool isVisible() const throw();
		/**
		 * @brief Sets the foreground color relative to the widget's position
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] c Dos Color Value
		 */
		virtual void drawFgColor(int x, int y, unsigned char c) throw();
		/**
		 * @brief Sets the background color relative to the widget's position
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] c Dos Color Value
		 */
		virtual void drawBgColor(int x, int y, unsigned char c) throw();
		/**
		 * @brief Sets ppg::Widget::aLeft, adjusts PpgWidget::aRight
		 * @param[in] x Left position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return New left position
		 */
		virtual int setLeft(int x, bool absolute = false) throw();
		/**
		 * @brief Sets PpgWidget::aTop, adjusts PpgWidget::aBottom
		 * @param[in] y Top position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return New top position
		 */
		virtual int setTop(int y, bool absolute = false) throw();
		/**
		 * @brief Sets PpgWidget::aLeft and PpgWidget::aTop, adjusts PpgWidget::aRight and PpgWidget::aBottom
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return @c false if nothing happened (no changes in PpgWidget::aLeft or PpgWidget::aTop)
		 * @warning The returned value is (in most cases) not valid if @a absolute is @c true
		 */
		virtual bool setPosition(int x, int y, bool absolute = false) throw();
		virtual bool setPosition(const Point& pos, bool absolute = false);
		/**
		 * @brief Adjusts PpgWidget::aRight
		 * @param[in] w Wanted width
		 * @return New width
		 * @exception PpgException is thrown if @c w\<=0
		 */
		virtual int setWidth(int w) throw(Exception);
		/**
		 * @brief Adjusts PpgWidget::aBottom
		 * @param[in] h Wanted height
		 * @return New height
		 * @exception PpgException is thrown if @c h\<=0
		 */
		virtual int setHeight(int h) throw(Exception);
		/**
		 * @brief Adjusts PpgWidget::aRight and PpgWidget::aBottom
		 * @param[in] w Width
		 * @param[in] h Height
		 * @return @c false if nothing happened (no changes in width and height)
		 * @exception PpgException See PpgWidget::setWidth and PpgWidget::setHeight
		 */
		virtual bool setSize(int w, int h) throw(Exception);
		virtual bool setSize(const Point& pt) throw(Exception);
		Rect area() const;
		Rect& area();
		/**
		 * @brief Get the top parent
		 * @return Pointer to the top parent
		 */
		virtual Widget *getTopParent() const;
		/**
		 * @brief Map widget's coordinates to the parent ones
		 * @param[in,out] x Left coordinate
		 * @param[in,out] y Top coordinate
		 */
		virtual void mapToParent(int *x, int *y) const;
		virtual void mapToParent(Point* pt) const;
		/**
		 * @brief Map widget's coordinates to the top parent ones
		 * @param[in,out] x Left coordinate
		 * @param[in,out] y Top coordinate
		 */
		virtual void mapToAbsolute(int *x, int *y) const;
		virtual void mapToAbsolute(Point* pt) const;
		/**
		 * @brief Move an element to the top
		 * @param[in] vp Pointer to the element to move to the top
		 */
		virtual void toTop(ppg::Widget* vp) throw();
};

} // namespace ppg

#endif // ppgwidgetH
