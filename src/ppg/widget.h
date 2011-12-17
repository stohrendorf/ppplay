/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

/**
 * @ingroup Ppg
 * @{
 */

#include "stuff/utils.h"
#include "rect.h"
#include "color.h"

#include <list>
#include <memory>

namespace ppg
{

/**
 * @class Widget
 * @brief The abstract base class for all PeePeeGUI classes
 * @details
 * Every PeePeeGUI class is derived from this class.
 *
 * If you create widgets on the stack (so to speak @e not by using @c operator @c new), make
 * sure to call @code Widget::setAutoDelete(false); @endcode
 *
 * Every widget "owns" its children, so that it @c deletes them when it gets destructed. You may prevent
 * auto-deletion by the procedure described above.
 */
class Widget
{
	DISABLE_COPY( Widget )
public:
	typedef std::list<Widget*> List; //!< @brief List of widgets
private:
	bool m_visible; //!< @brief @c false if this widget and it's children should not be drawn
	Widget* m_parent; //!< @brief Pointer to the parent widget, or @c nullptr if it's the top widget
	Rect m_area; //!< @brief Area of this widget within the parent's space
	List m_children; //!< @brief Children within this widget
	bool m_autodelete; //!< @brief If @c true, the parent widget should delete this when destroyed. Default is @c true
	virtual void drawThis() = 0; //!< @brief Internal drawing method, called by PppWidet::draw() @see draw()
public:
	/**
	 * @brief Constructor
	 * @param[in] parent Parent widget
	 */
	explicit Widget( Widget* parent );
	/**
	 * @brief Destructor. Deletes all children.
	 */
	virtual ~Widget();
	/**
	 * @brief Calls drawThis(), but only when m_visible is @c true
	 */
	void draw();
	/**
	 * @brief Make this widget visible
	 */
	void show();
	/**
	 * @brief Make this widget invisible
	 */
	void hide();
	/**
	 * @brief Draw a char relative to the widget's position
	 * @param[in] x Left position
	 * @param[in] y Top position
	 * @param[in] c The char to draw
	 */
	virtual void drawChar( int x, int y, char c );
	/**
	 * @brief Is this widget visible?
	 * @return m_visible
	 */
	bool isVisible() const;
	/**
	 * @brief Sets the foreground color relative to the widget's position
	 * @param[in] x Left position
	 * @param[in] y Top position
	 * @param[in] c Dos Color Value
	 */
	virtual void setFgColorAt( int x, int y, Color c );
	/**
	 * @brief Sets the background color relative to the widget's position
	 * @param[in] x Left position
	 * @param[in] y Top position
	 * @param[in] c Dos Color Value
	 */
	virtual void setBgColorAt( int x, int y, Color c );
	/**
	 * @brief Sets the widget's left position
	 * @param[in] x Left position
	 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
	 * @return New left position
	 */
	virtual int setLeft( int x, bool absolute = false );
	/**
	 * @brief Sets the widget's top position
	 * @param[in] y Top position
	 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
	 * @return New top position
	 */
	virtual int setTop( int y, bool absolute = false );
	/**
	 * @brief Sets the widget's position
	 * @param[in] x Left position
	 * @param[in] y Top position
	 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
	 * @return @c false if the position did not change
	 */
	virtual bool setPosition( int x, int y, bool absolute = false );
	/**
	 * @overload
	 * @param[in] pos Position
	 * @param[in] absolute Set to @c true to calculate the position relative to the top parent widget
	 * @return @c false if the position did not change
	 */
	virtual bool setPosition( const Point& pos, bool absolute = false );
	/**
	 * @brief Sets the widget's width
	 * @param[in] w Wanted width
	 * @return New width
	 * @pre @c w>0
	 */
	virtual int setWidth( int w );
	/**
	 * @brief Sets the widget's height
	 * @param[in] h Wanted height
	 * @return New height
	 * @pre @c h>0
	 */
	virtual int setHeight( int h );
	/**
	 * @brief Sets the widget's size
	 * @param[in] w Width
	 * @param[in] h Height
	 * @return @c false if nothing changed
	 * @pre @c w>0
	 * @pre @c h>0
	 */
	virtual bool setSize( int w, int h );
	/**
	 * @overload
	 * @param[in] pt New size
	 * @return @c false if nothing changed
	 */
	virtual bool setSize( const Point& pt );
	/**
	 * @brief Get the widget's area
	 * @return The widget's area
	 */
	Rect area() const;
	/**
	 * @overload
	 * @return The widget's area
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
	 * @param[in] pt Coordinate
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
	 * @param[in] pt Coordinate
	 */
	virtual void mapToAbsolute( Point* pt ) const;
	/**
	 * @brief Move an element to the top
	 * @param[in] vp Pointer to the element to move to the top
	 */
	virtual void toTop( Widget* vp );
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
	/**
	 * @brief Sets the value of m_autoDelete
	 * @param[in] value The new value
	 */
	void setAutoDelete( bool value );
};

} // namespace ppg

/**
 * @}
 */

#endif // ppgwidgetH
