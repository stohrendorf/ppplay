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

#include <vector>
//#include <cstring>

/**
 * @defgroup Ppg The PeePeeGUI Definitions
 * @brief This module contains definitions for the PeePeeGUI User Interface
 * @todo Check all @c operator= for self-assignments (strong guarantee?)
 */

class PpgContainer;

/**
 * @class PpgWidget
 * @ingroup Ppg
 * @brief The abstract base class for all PeePeeGUI classes
 * @details
 * Every PeePeeGUI class is derived from this class.
 */
class PpgWidget {
	private:
		bool m_isContainer; //!< @brief This is @c true if this widget is a container
	protected:
		std::string m_name; //!< @brief (Unique) Name of the instance
		std::string m_type; //!< @brief Class type. Empty for ::PpgWidget
		bool m_visible; //!< @brief @c false if this widget and it's children should not be drawn
		PpgWidget *m_parent; //!< @brief Pointer to the parent widget, or @c NULL if it's the top widget
		int m_left; //!< @brief Relative X position of this widget to the parent
		int m_top; //!< @brief Relative Y position of this widget to the parent
		int m_bottom; //!< @brief Relative bottom position of this widget to the parent
		int m_right; //!< @brief Relative right position of this widget to the parent
		std::vector<PpgWidget*> m_children; //!< @brief Children within this container @see PpgWidget::aIsContainer
		virtual void drawThis() throw(PpgException); //!< @brief Internal drawing method, called by PppWidet::draw() @see draw()
		void isFinalNode() throw() { m_isContainer = false; } //!< @brief Call this if your widget should not contain other widgets
	public:
		/**
		* @brief Value that marks a non-changing color
		 */
		static const int ESC_NOCHANGE = 0xff;
		/**
		 * @brief Constructor
		 * @param[in] name Name of this widget
		 */
		PpgWidget(const std::string &name) throw();
		/**
		 * @brief Copy constructor
		 * @param[in] src Source to copy from
		 */
		PpgWidget(const PpgWidget& src) throw();
		/**
		 * @brief Copy operator
		 * @param[in] src Source to copy from
		 * @return Reference to *this
		 */
		PpgWidget &operator=(const PpgWidget &src) throw();
		/**
		 * @brief Destructor. No operation.
		 */
		virtual ~PpgWidget() throw();
		/**
		 * @brief Get PpgWidget::aName
		 * @return Widget's name
		 */
		virtual std::string getName() const throw();
		/**
		 * @brief Get PppWidget::aType
		 * @return Widget's type
		 */
		virtual std::string getType() const throw();
		/**
		 * @brief Calls PpgWidget::drawThis(), but only when PpgWidget::aVisible is @c true
		 */
		virtual void draw() throw(PpgException);
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
		 * @return PpgWidget::aVisible
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
		 * @brief Sets PpgWidget::aLeft, adjusts PpgWidget::aRight
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
		virtual int setWidth(const int w) throw(PpgException);
		/**
		 * @brief Adjusts PpgWidget::aBottom
		 * @param[in] h Wanted height
		 * @return New height
		 * @exception PpgException is thrown if @c h\<=0
		 */
		virtual int setHeight(const int h) throw(PpgException);
		/**
		 * @brief Adjusts PpgWidget::aRight and PpgWidget::aBottom
		 * @param[in] w Width
		 * @param[in] h Height
		 * @return @c false if nothing happened (no changes in width and height)
		 * @exception PpgException See PpgWidget::setWidth and PpgWidget::setHeight
		 */
		virtual bool setDimensions(int w, int h) throw(PpgException);
		/**
		 * @brief Get PpgWidget::aLeft
		 * @return Relative left position
		 */
		virtual int getLeft() const throw() __attribute__((const));
		/**
		 * @brief Get PpgWidget::aTop
		 * @return Relative top position
		 */
		virtual int getTop() const throw() __attribute__((const));
		/**
		 * @brief Get PpgWidget::aRight
		 * @return Relative right position
		 */
		virtual int getRight() const throw() __attribute__((const));
		/**
		 * @brief Get PpgWidget::aBottom
		 * @return Relative bottom position
		 */
		virtual int getBottom() const throw() __attribute__((const));
		/**
		 * @brief Get width
		 * @return Widget's width
		 */
		virtual int getWidth() const throw() __attribute__((const));
		/**
		 * @brief Get height
		 * @return Widget's height
		 */
		virtual int getHeight() const throw() __attribute__((const));
		/**
		 * @brief Get the top parent
		 * @return Pointer to the top parent
		 */
		virtual PpgWidget *getTopParent() throw();
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
		 * @brief Sets the widget's parent container
		 * @param[in,out] newParent Widget's parent container
		 * @param[in] caller Pointer to the caller to prevent infinite cyclic calls
		 */
		virtual void setParent(PpgWidget *newParent, PpgWidget *caller = NULL) throw();
		/**
		 * @brief Returns ::aIsContainer
		 * @return ::aIsContainer;
		 */
		bool isContainer() { return m_isContainer; }
		/**
		 * @brief Get an element by it's named path, i.e. @code "/mainwindow/subwindow/labelXY" @endcode
		 * @tparam T Widget's type to retrieve
		 * @param[in] path Path to the widget
		 * @return Pointer to the child, or @c NULL if no compatible widget was found
		 * @see PpgWidget::aName
		 */
		template<class T>
		T *getByPath(const std::string &path) throw();
		/**
		 * @brief Adds a child to this container
		 * @param[in,out] child Child to add, sets it's PpgElement::aParent value to @c this
		 * @pre @c child!=NULL
		 * @exception PpgException is thrown if @a child is @c NULL
		 */
		virtual void addChild(PpgWidget &child) throw(PpgException);
		/**
		 * @brief Removes @a child from this container's element list, doesn't change its PpgElement::aParent value
		 * @param child Pointer to the element to remove from PpgWidget::aChildren
		 * @return @c true if @a child was found and removed, @c false otherwise
		 */
		virtual bool removeChild(PpgWidget *child) throw();
		/**
		 * @brief Returns the top level child
		 * @return Top level child
		 * @see PpgWidget::toTop
		 */
		virtual PpgWidget *firstChild() throw();
		/**
		 * @brief Move an element to the top
		 * @param[in] name Name of the element to move to the top
		 */
		virtual void toTop(const std::string &name) throw();
		/**
		 * @brief Move an element to the top
		 * @param[in] vp Pointer to the element to move to the top
		 */
		virtual void toTop(PpgWidget &vp) throw();
		/**
		 * @brief Move an element to the top
		 * @param[in] zOrder Z-Order index of the element to move to the top
		 */
		virtual void toTop(unsigned int zOrder) throw();
};

template<class T>
T *PpgWidget::getByPath(const std::string &path) throw() {
	static_assert(__is_base_of(PpgWidget,T), "Error: Template Parameter T of template function 'T* PpgWidget::getByPath(string)' is not a derived child of PpgWidget" );
	//PPG_TEST(!isContainer());
	if(!isContainer())
		return NULL;
	if (path.length() == 0)
		return NULL;
	size_t pos = path.find("/");
	if (pos == std::string::npos) {
		for (unsigned int i = 0; i < m_children.size(); i++) {
			if (m_children[i]->getName() == path) {
				return dynamic_cast<T*>(m_children[i]);
			}
		}
	}
	else if (pos == 0) {
		PpgWidget *tp = getTopParent();
		if(!tp)
			return NULL;
		if(!tp->isContainer())
			return NULL;
		return tp->getByPath<T>(std::string(path, 1));
	}
	else {
		std::string subName(path, pos + 1);
		for (unsigned int i = 0; i < m_children.size(); i++) {
			PpgWidget *con = dynamic_cast<T*>(m_children[i]);
			if (con) {
				T *res = con->getByPath<T>(subName);
				if (res)
					return res;
			}
		}
	}
	return NULL;
}

extern template class std::vector<PpgWidget*>;

#endif // ppgbaseH
