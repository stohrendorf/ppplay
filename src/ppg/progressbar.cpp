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

#include "progressbar.h"

#include <boost/assert.hpp>

/**
 * @ingroup ppg
 * @{
 */

namespace ppg
{

ProgressBar::ProgressBar( Widget* parent, size_t maxVal, int w ): Widget( parent ), m_maxVal( maxVal ), m_value( 0 ), m_fgColor( Color::White ), m_bgColor( Color::None )
{
	BOOST_ASSERT( w > 2 );
	Widget::setHeight( 1 );
	setWidth( w );
}

ProgressBar::~ProgressBar() = default;

size_t ProgressBar::max() const
{
	return m_maxVal;
}

void ProgressBar::setMax( size_t maxVal )
{
	m_maxVal = maxVal;
	if( m_value > m_maxVal ) {
		m_value = m_maxVal;
	}
}

size_t ProgressBar::value() const
{
	return m_value;
}

void ProgressBar::setValue( size_t val )
{
	if( val <= m_maxVal ) {
		m_value = val;
	}
}

void ProgressBar::drawThis()
{
	int w = area().width();
	const int pos = m_maxVal==0 ? 0 : ( w - 2 ) * m_value / m_maxVal;
	for( int i = 0; i < w; i++ ) {
		if( i == 0 ) {
			drawChar( i, 0, '[' );
		}
		else if( i == w - 1 ) {
			drawChar( i, 0, ']' );
		}
		else if( i <= pos ) {
			drawChar( i, 0, static_cast<char>( 0xfe ) );
		}
		else {
			drawChar( i, 0, static_cast<char>( 0xf9 ) );
		}
		if( m_fgColor != Color::None ) {
			setFgColorAt( i, 0, m_fgColor );
		}
		if( m_bgColor != Color::None ) {
			setBgColorAt( i, 0, m_bgColor );
		}
	}
}

int ProgressBar::setHeight( int )
{
	return 1;
}

void ProgressBar::setBgColor( Color c )
{
	m_bgColor = c;
}

void ProgressBar::setFgColor( Color c )
{
	m_fgColor = c;
}

}

/**
 * @}
 */
