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

#include "widget.h"
#include <algorithm>

#include "logger/logger.h"

namespace ppg {

	Point::Point() : m_x( 0 ), m_y( 0 ) {
	}
	Point::Point( int x, int y ) : m_x( x ), m_y( y ) {
	}
	const Point& Point::operator+=( const Point& rhs ) {
		m_x += rhs.m_x;
		m_y += rhs.m_y;
		return *this;
	}
	const Point Point::operator+( const Point& rhs ) const {
		return Point( m_x + rhs.m_x, m_y + rhs.m_y );
	}
	const Point& Point::operator-=( const Point& rhs ) {
		m_x -= rhs.m_x;
		m_y -= rhs.m_y;
		return *this;
	}
	const Point Point::operator-( const Point& rhs ) const {
		return Point( m_x - rhs.m_x, m_y - rhs.m_y );
	}
	int Point::x() const {
		return m_x;
	}
	int Point::y() const {
		return m_y;
	}
	void Point::setX( int x ) {
		m_x = x;
	}
	void Point::setY( int y ) {
		m_y = y;
	}

	Rect::Rect( int x, int y, int width, int height ) : m_topLeft( x, y ), m_bottomRight( x + width - 1, y + height - 1 ) {
	}
	int Rect::top() const {
		return m_topLeft.y();
	}
	void Rect::setTop( int top ) {
		m_topLeft.setY( top );
	}
	int Rect::left() const {
		return m_topLeft.x();
	}
	void Rect::setLeft( int left ) {
		m_topLeft.setX( left );
	}
	int Rect::bottom() const {
		return m_bottomRight.y();
	}
	void Rect::setBottom( int bottom ) {
		m_bottomRight.setY( bottom );
	}
	int Rect::right() const {
		return m_bottomRight.x();
	}
	void Rect::setRight( int right ) {
		m_bottomRight.setX( right );
	}
	int Rect::width() const {
		return m_bottomRight.x() - m_topLeft.x() + 1;
	}
	void Rect::setWidth( int width ) {
		m_bottomRight.setX( m_topLeft.x() + width - 1 );
	}
	int Rect::height() const {
		return m_bottomRight.y() - m_topLeft.y() + 1;
	}
	void Rect::setHeight( int height ) {
		m_bottomRight.setY( m_topLeft.y() + height - 1 );
	}
	Point Rect::topLeft() const {
		return m_topLeft;
	}
	Point& Rect::topLeft() {
		return m_topLeft;
	}
	Point Rect::bottomRight() const {
		return m_bottomRight;
	}
	Point& Rect::bottomRight() {
		return m_bottomRight;
	}
	Point Rect::size() const {
		return Point( width(), height() );
	}


	Widget::Widget( Widget* parent ) :
		m_visible( false ),
		m_parent( parent ),
		m_area( 0, 0, 0, 0 ),
		m_children() {
		if( parent )
			parent->m_children.push_back( this );
	}

	Widget::~Widget() throw() {
		if( m_parent )
			m_parent->m_children.remove( this );
		Widget::List backup = m_children;
		std::for_each(
		    backup.begin(),
		    backup.end(),
		[]( Widget * w ) {
			delete w;
		}
		);
	}

	int Widget::setLeft( int x, bool absolute ) throw() {
		if( absolute )
			mapToAbsolute( &x, NULL );
		int w = m_area.width();
		m_area.setLeft( x );
		m_area.setWidth( w );
		return m_area.left();
	}

	int Widget::setTop( int y, bool absolute ) throw() {
		if( absolute )
			mapToAbsolute( NULL, &y );
		int h = m_area.height();
		m_area.setTop( y );
		m_area.setHeight( h );
		return m_area.top();
	}

	bool Widget::setPosition( int x, int y, bool absolute ) throw() {
		setLeft( x, absolute );
		setTop( y, absolute );
		return ( m_area.left() != x ) || ( m_area.top() != y );
	}

	bool Widget::setPosition( const Point& pos, bool absolute ) {
		return setPosition( pos.x(), pos.y(), absolute );
	}

	int Widget::setWidth( int w ) throw( Exception ) {
		PPG_TEST( w <= 0 );
		m_area.setWidth( w );
		return m_area.width();
	}

	int Widget::setHeight( int h ) throw( Exception ) {
		PPG_TEST( h <= 0 );
		m_area.setHeight( h );
		return m_area.height();
	}

	bool Widget::setSize( int w, int h ) throw( Exception ) {
		setWidth( w );
		setHeight( h );
		return ( m_area.width() != w ) || ( m_area.height() != h );
	}
	bool Widget::setSize( const Point& pt ) throw( Exception ) {
		return setSize( pt.x(), pt.y() );
	}

	void Widget::draw() throw( Exception ) {
		if( !isVisible() )
			return;
		// draw from bottom to top so that top elements are drawn over bottom ones
		for( Widget::List::reverse_iterator revIt = m_children.rbegin(); revIt != m_children.rend(); revIt++ ) {
			Widget* w = *revIt;
			if( !w->isVisible() )
				continue;
			w->draw();
		}
		drawThis();
	}

	Rect Widget::area() const {
		return m_area;
	}
	Rect& Widget::area() {
		return m_area;
	}

	void Widget::show() throw() {
		m_visible = true;
	}

	void Widget::hide() throw() {
		m_visible = false;
	}

	bool Widget::isVisible() const throw() {
		return m_visible;
	}

	void Widget::drawChar( int x, int y, char c ) throw() {
		if( !m_parent )
			return;
		mapToParent( &x, &y );
		if( !m_area.contains( x, y ) )
			return;
		m_parent->drawChar( x, y, c );
	}

	void Widget::drawFgColor( int x, int y, uint8_t c ) throw() {
		if( !m_parent )
			return;
		mapToParent( &x, &y );
		if( !m_area.contains( x, y ) )
			return;
		m_parent->drawFgColor( x, y, c );
	}

	void Widget::drawBgColor( int x, int y, uint8_t c ) throw() {
		if( !m_parent )
			return;
		mapToParent( &x, &y );
		if( !m_area.contains( x, y ) )
			return;
		m_parent->drawBgColor( x, y, c );
	}

	void Widget::mapToParent( int* x, int* y ) const {
		if( !m_parent )
			return;
		if( x != NULL )
			*x += m_area.left();
		if( y != NULL )
			*y += m_area.top();
	}
	void Widget::mapToParent( ppg::Point* pt ) const {
		if( !m_parent || !pt )
			return;
		*pt += m_area.topLeft();
	}

	void Widget::mapToAbsolute( int* x, int* y ) const {
		if( !m_parent )
			return;
		mapToParent( x, y );
		m_parent->mapToAbsolute( x, y );
	}
	void Widget::mapToAbsolute( ppg::Point* pt ) const {
		if( !m_parent || !pt )
			return;
		mapToParent( pt );
		m_parent->mapToAbsolute( pt );
	}

	Widget* Widget::getTopParent() const {
		if( !m_parent )
			return NULL;
		return m_parent->getTopParent();
	}

	void Widget::toTop( ppg::Widget* vp ) throw() {
		if( std::find( m_children.begin(), m_children.end(), vp ) == m_children.end() )
			return;
		m_children.remove( vp );
		m_children.push_front( vp );
	}

	bool Widget::onMouseMove( int x, int y ) {
		for( Widget::List::iterator it = m_children.begin(); it != m_children.end(); it++ ) {
			Widget* current = *it;
			if( !current )
				continue;
			Rect currentArea = current->area();
			if( current->onMouseMove( x - currentArea.left(), y - currentArea.top() ) )
				return true;
		}
		return false;
	}


} // namespace ppg
