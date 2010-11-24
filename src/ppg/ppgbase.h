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

#ifndef ppgbaseH
#define ppgbaseH

#include "ppgexcept.h"
#include "utils.h"

#include <list>

namespace ppg {

/**
 * @defgroup Ppg The PeePeeGUI Definitions
 * @brief This module contains definitions for the PeePeeGUI User Interface
 * @todo Check all @c operator= for self-assignments (strong guarantee?)
 */

class Container;

/**
 * @class Widget
 * @ingroup Ppg
 * @brief The abstract base class for all PeePeeGUI classes
 * @details
 * Every PeePeeGUI class is derived from this class.
 */
class Widget {
	private:
		Widget(const Widget&) = delete;
		Widget& operator=(const Widget&) = delete;
	private:
		bool m_visible; //!< @brief @c false if this widget and it's children should not be drawn
		Widget *m_parent; //!< @brief Pointer to the parent widget, or @c NULL if it's the top widget
		int m_left; //!< @brief Relative X position of this widget to the parent
		int m_top; //!< @brief Relative Y position of this widget to the parent
		int m_bottom; //!< @brief Relative bottom position of this widget to the parent
		int m_right; //!< @brief Relative right position of this widget to the parent
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
		virtual void draw() throw(Exception);
		/**
		 * @brief Make this widget visible
		 */
		virtual void show() throw();
		/**
		 * @brief Make this widget invisible
		 */
		virtual void hide() throw();
		/**
		 * @brief Draw a char relative to the widget's position
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] c The char to draw
		 */
		virtual void drawChar(const int x, const int y, const char c) throw();
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
		virtual void drawFgColor(const int x, const int y, const unsigned char c) throw();
		/**
		 * @brief Sets the background color relative to the widget's position
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] c Dos Color Value
		 */
		virtual void drawBgColor(const int x, const int y, const unsigned char c) throw();
		/**
		 * @brief Sets ppg::Widget::aLeft, adjusts PpgWidget::aRight
		 * @param[in] x Left position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return New left position
		 */
		virtual int setLeft(int x, const bool absolute = false) throw();
		/**
		 * @brief Sets PpgWidget::aTop, adjusts PpgWidget::aBottom
		 * @param[in] y Top position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return New top position
		 */
		virtual int setTop(int y, const bool absolute = false) throw();
		/**
		 * @brief Sets PpgWidget::aLeft and PpgWidget::aTop, adjusts PpgWidget::aRight and PpgWidget::aBottom
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return @c false if nothing happened (no changes in PpgWidget::aLeft or PpgWidget::aTop)
		 * @warning The returned value is (in most cases) not valid if @a absolute is @c true
		 */
		virtual bool setPosition(int x, int y, const bool absolute = false) throw();
		/**
		 * @brief Sets PpgWidget::aRight, adjusts PpgWidget::aLeft
		 * @param[in] x Right position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return New right value
		 */
		virtual int setRight(int x, const bool absolute = false) throw();
		/**
		 * @brief Sets PpgWidget::aBottom, adjusts PpgWidget::aTop
		 * @param[in] y Bottom position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return New bottom value
		 */
		virtual int setBottom(int y, const bool absolute = false) throw();
		/**
		 * @brief Sets PpgWidget::aRight and PpgWidget::aBottom, ajusts PpgWidget::aLeft and PpgWidget::aTop
		 * @param[in] x Right position
		 * @param[in] y Bottom position
		 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
		 * @return @c false if nothing happened (no changes in PpgWidget::aBottom or PpgWidget::aRight)
		 * @warning The returned value is (in most cases) not valid if @a absolute is @c true
		 */
		virtual bool setBottomRight(int x, int y, const bool absolute = false) throw();
		/**
		 * @brief Adjusts PpgWidget::aRight
		 * @param[in] w Wanted width
		 * @return New width
		 * @exception PpgException is thrown if @c w\<=0
		 */
		virtual int setWidth(const int w) throw(Exception);
		/**
		 * @brief Adjusts PpgWidget::aBottom
		 * @param[in] h Wanted height
		 * @return New height
		 * @exception PpgException is thrown if @c h\<=0
		 */
		virtual int setHeight(const int h) throw(Exception);
		/**
		 * @brief Adjusts PpgWidget::aRight and PpgWidget::aBottom
		 * @param[in] w Width
		 * @param[in] h Height
		 * @return @c false if nothing happened (no changes in width and height)
		 * @exception PpgException See PpgWidget::setWidth and PpgWidget::setHeight
		 */
		virtual bool setDimensions(int w, int h) throw(Exception);
		/**
		 * @brief Get PpgWidget::aLeft
		 * @return Relative left position
		 */
		virtual int getLeft() const throw();
		/**
		 * @brief Get PpgWidget::aTop
		 * @return Relative top position
		 */
		virtual int getTop() const throw();
		/**
		 * @brief Get PpgWidget::aRight
		 * @return Relative right position
		 */
		virtual int getRight() const throw();
		/**
		 * @brief Get PpgWidget::aBottom
		 * @return Relative bottom position
		 */
		virtual int getBottom() const throw();
		/**
		 * @brief Get width
		 * @return Widget's width
		 */
		virtual int getWidth() const throw();
		/**
		 * @brief Get height
		 * @return Widget's height
		 */
		virtual int getHeight() const throw();
		/**
		 * @brief Get the top parent
		 * @return Pointer to the top parent
		 */
		virtual Widget *getTopParent() throw();
		/**
		 * @brief Map widget's coordinates to the parent ones
		 * @param[in,out] x Left coordinate
		 * @param[in,out] y Top coordinate
		 */
		virtual void mapToParent(int *x, int *y) throw();
		/**
		 * @brief Map widget's coordinates to the top parent ones
		 * @param[in,out] x Left coordinate
		 * @param[in,out] y Top coordinate
		 */
		virtual void mapToAbsolute(int *x, int *y) throw();
		/**
		 * @brief Move an element to the top
		 * @param[in] vp Pointer to the element to move to the top
		 */
		virtual void toTop(ppg::Widget* vp) throw();
};

} // namespace ppg

#endif // ppgbaseH
