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

#include "label.h"
#include <algorithm>

namespace ppg {

	Label::Label( Widget* parent, const std::string& text ) :
		Widget( parent ),
		m_text( text ),
		m_fgColors(),
		m_bgColors(),
		alignment( Alignment::alLeft ) {
		if( length() != 0 )
			setWidth( length() );
		else
			setWidth( 1 );
		Widget::setHeight( 1 );
	}

	Label::~Label() throw() {
	}

	void Label::sizeColorsToMax() {
		int w = area().width();
		m_fgColors.resize( std::max<std::size_t>( m_text.length(), w ), ESC_NOCHANGE );
		m_bgColors.resize( std::max<std::size_t>( m_text.length(), w ), ESC_NOCHANGE );
	}

	Label& Label::operator=( const std::string& ass ) throw() {
		setText( ass );
		return *this;
	}

	void Label::setText( const std::string& txt ) {
		m_text = txt;
		sizeColorsToMax();
	}

	Label& Label::operator+=( const std::string& src ) throw() {
		m_text += src;
		sizeColorsToMax();
		return *this;
	}

	int Label::setHeight( int /*h*/ ) throw( Exception ) {
		return 1;
	}

	int Label::setWidth( int w ) throw( Exception ) {
		Widget::setWidth( w );
		sizeColorsToMax();
		return area().width();
	}

	std::size_t Label::length() const throw() {
		return m_text.length();
	}

	char& Label::operator[]( std::size_t index ) throw( Exception ) {
		PPG_TEST( index >= m_text.length() );
		return m_text[index];
	}

	void Label::setFgColor( std::size_t pos, unsigned char color, std::size_t len ) throw() {
		if( pos >= m_bgColors.size() )
			return;
		if( ( len == 0 ) || ( len + pos > m_bgColors.size() ) )
			len = m_bgColors.size() - pos;
		std::fill_n( &m_fgColors[pos], len, color );
	}

	void Label::setBgColor( std::size_t pos, unsigned char color, std::size_t len ) throw() {
		if( pos >= m_bgColors.size() )
			return;
		if( ( len == 0 ) || ( len + pos > m_bgColors.size() ) )
			len = m_bgColors.size() - pos;
		std::fill_n( &m_bgColors[pos], len, color );
	}

	void Label::drawThis() throw( Exception ) {
		if( length() == 0 )
			return;
		int offset;
		switch( alignment ) {
			case Alignment::alLeft:
				offset = 0;
				break;
			case Alignment::alRight:
				offset = area().width() - length();
				break;
			case Alignment::alCenter:
				offset = ( area().width() - length() ) / 2;
				break;
			default:
				PPG_THROW( "Invalid alignment" );
		}
		int w = area().width();
		for( int localX = offset; localX < w; localX++ ) {
			std::size_t textPos = localX - offset;
			if( textPos >= length() )
				break;
			drawChar( localX, 0, m_text[textPos] );
			if( m_fgColors[textPos] != ESC_NOCHANGE )
				drawFgColor( localX, 0, m_fgColors[textPos] );
			if( m_bgColors[textPos] != ESC_NOCHANGE )
				drawBgColor( localX, 0, m_bgColors[textPos] );
		}
	}

} // namespace ppg
