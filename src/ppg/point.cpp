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

#include "point.h"

namespace ppg
{

Point::Point() : m_x( 0 ), m_y( 0 )
{
}

Point::Point( int x, int y ) : m_x( x ), m_y( y )
{
}

const Point& Point::operator+=( const Point& rhs )
{
	m_x += rhs.m_x;
	m_y += rhs.m_y;
	return *this;
}

const Point Point::operator+( const Point& rhs ) const
{
	return Point( m_x + rhs.m_x, m_y + rhs.m_y );
}

const Point& Point::operator-=( const Point& rhs )
{
	m_x -= rhs.m_x;
	m_y -= rhs.m_y;
	return *this;
}

const Point Point::operator-( const Point& rhs ) const
{
	return Point( m_x - rhs.m_x, m_y - rhs.m_y );
}

int Point::x() const
{
	return m_x;
}

int Point::y() const
{
	return m_y;
}

void Point::setX( int x )
{
	m_x = x;
}

void Point::setY( int y )
{
	m_y = y;
}

}
