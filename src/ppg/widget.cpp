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

#include "widget.h"
#include <light4cxx/logger.h>

#include <algorithm>
#include <boost/assert.hpp>

namespace ppg
{

Widget::Widget( Widget* parent ) :
	m_visible( false ),
	m_parent( parent ),
	m_area( 0, 0, 0, 0 ),
	m_children(),
	m_autodelete( true )
{
	if( parent ) {
		parent->m_children.push_back( this );
	}
}

Widget::~Widget()
{
	// delete the children
	List backup = m_children;
	for( Widget * tmp : backup ) {
		if( tmp->m_autodelete ) {
			delete tmp;
		}
		else {
			// the child widget doesn't destroy itself here, so we got to
			// remove it manually here
			tmp->m_parent = nullptr;
			m_children.remove(tmp);
		}
	}
	// the children remove themselves
	BOOST_ASSERT_MSG( m_children.empty(), stringFmt("Widget expected to have no children, but %d are left", m_children.size()).c_str() );
	// remove this widget from the parent's children list
	if( m_parent ) {
#ifndef NDEBUG
		if( m_parent->m_children.cend() == std::find(m_parent->m_children.cbegin(), m_parent->m_children.cend(), this) ) {
			light4cxx::Logger::get("ppg.widget")->error(L4CXX_LOCATION, "The parent of the widget does not contain the widget itself");
		}
#endif
		m_parent->m_children.remove( this );
	}
}

int Widget::setLeft( int x, bool absolute )
{
	if( absolute ) {
		mapToAbsolute( &x, nullptr );
	}
	int w = m_area.width();
	m_area.setLeft( x );
	m_area.setWidth( w );
	return m_area.left();
}

int Widget::setTop( int y, bool absolute )
{
	if( absolute ) {
		mapToAbsolute( nullptr, &y );
	}
	int h = m_area.height();
	m_area.setTop( y );
	m_area.setHeight( h );
	return m_area.top();
}

bool Widget::setPosition( int x, int y, bool absolute )
{
	setLeft( x, absolute );
	setTop( y, absolute );
	return ( m_area.left() != x ) || ( m_area.top() != y );
}

bool Widget::setPosition( const Point& pos, bool absolute )
{
	return setPosition( pos.x(), pos.y(), absolute );
}

int Widget::setWidth( int w )
{
	if( w > 0 ) {
		m_area.setWidth( w );
	}
	return m_area.width();
}

int Widget::setHeight( int h )
{
	if( h > 0 ) {
		m_area.setHeight( h );
	}
	return m_area.height();
}

bool Widget::setSize( int w, int h )
{
	setWidth( w );
	setHeight( h );
	return ( m_area.width() != w ) || ( m_area.height() != h );
}
bool Widget::setSize( const Point& pt )
{
	return setSize( pt.x(), pt.y() );
}

void Widget::draw()
{
	if( !isVisible() )
		return;
	// draw from bottom to top so that top elements are drawn over bottom ones
	for( auto revIt = m_children.rbegin(); revIt != m_children.rend(); revIt++ ) {
		Widget* w = *revIt;
		if( !w || !w->isVisible() ) {
			continue;
		}
		w->draw();
	}
	drawThis();
}

Rect Widget::area() const
{
	return m_area;
}

Rect& Widget::area()
{
	return m_area;
}

void Widget::show()
{
	m_visible = true;
}

void Widget::hide()
{
	m_visible = false;
}

void Widget::drawChar( int x, int y, char c )
{
	if( !m_parent ) {
		return;
	}
	mapToParent( &x, &y );
	if( !m_area.contains( x, y ) ) {
		return;
	}
	m_parent->drawChar( x, y, c );
}

void Widget::setFgColorAt( int x, int y, Color c )
{
	if( !m_parent ) {
		return;
	}
	mapToParent( &x, &y );
	if( !m_area.contains( x, y ) ) {
		return;
	}
	m_parent->setFgColorAt( x, y, c );
}

void Widget::setBgColorAt( int x, int y, Color c )
{
	if( !m_parent ) {
		return;
	}
	mapToParent( &x, &y );
	if( !m_area.contains( x, y ) ) {
		return;
	}
	m_parent->setBgColorAt( x, y, c );
}

void Widget::mapToParent( int* x, int* y ) const
{
	if( !m_parent ) {
		return;
	}
	if( x != nullptr ) {
		*x += m_area.left();
	}
	if( y != nullptr ) {
		*y += m_area.top();
	}
}
void Widget::mapToParent( ppg::Point* pt ) const
{
	if( !m_parent || !pt ) {
		return;
	}
	*pt += m_area.topLeft();
}

void Widget::mapToAbsolute( int* x, int* y ) const
{
	if( !m_parent ) {
		return;
	}
	m_parent->mapToAbsolute( x, y );
}
void Widget::mapToAbsolute( ppg::Point* pt ) const
{
	if( !m_parent || !pt ) {
		return;
	}
	mapToParent( pt );
	if( !m_parent ) {
		return;
	}
	m_parent->mapToAbsolute( pt );
}

Widget* Widget::getTopParent() const
{
	if( !m_parent ) {
		return nullptr;
	}
	return m_parent->getTopParent();
}

void Widget::toTop( Widget* vp )
{
	if( std::find( m_children.begin(), m_children.end(), vp ) == m_children.end() )
		return;
	m_children.remove( vp );
	m_children.push_front( vp );
}

bool Widget::onMouseMove( int x, int y )
{
	for( Widget * current : m_children ) {
		if( !current ) {
			continue;
		}
		Rect currentArea = current->area();
		if( current->onMouseMove( x - currentArea.left(), y - currentArea.top() ) ) {
			return true;
		}
	}
	return false;
}

void Widget::setAutoDelete( bool value )
{
	m_autodelete = value;
}

bool Widget::isVisible() const
{
	return m_visible;
}

} // namespace ppg


