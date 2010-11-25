/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
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

#include "ppg.h"
#include <algorithm>
#include <vector>

namespace ppg {

static Uint32 gDosColors[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static std::shared_ptr< std::vector<char> > gaChars;
static std::shared_ptr< std::vector<uint8_t> > gaColorsF;
static std::shared_ptr< std::vector<uint8_t> > gaColorsB;
static SDL_Surface *gaScreen = NULL;

static inline void DrawPixel( const int x, const int y, const Uint32 color ) throw() {
	if (( x < 0 ) || ( y < 0 ) || ( y >= gaScreen->h ) || ( x >= gaScreen->w ) )
		return;
	reinterpret_cast<Uint32*>( gaScreen->pixels )[(( y*gaScreen->pitch ) >> 2 ) + x] = color;
}

static inline void DrawPixelRGB( const int x, const int y, const Uint8 R, const Uint8 G, const Uint8 B ) throw( Exception ) {
	DrawPixel( x, y, SDL_MapRGB( gaScreen->format, R, G, B ) );
}

void Screen::clearOverlay() {
	std::fill_n(m_pixelOverlay->begin(), gaScreen->w*gaScreen->h, 0xff);
}

Screen::Screen( const int w, const int h, const std::string &title ) throw( Exception ) : Widget( NULL ), m_pixelOverlay(), m_pixW(0), m_pixH(0) {
	if ( gaScreen ) {
		std::cerr << "Uh-oh... creating a second screen o.O" << std::endl << std::flush;
	}
	else {
		gaScreen = SDL_SetVideoMode( w * 8, h * 16, 32, SDL_SWSURFACE );
		if ( !gaScreen ) {
			PPG_THROW( "Screen Initialization failed" );
		}
		m_pixW = gaScreen->w;
		m_pixH = gaScreen->h;
		setTop(0);
		setLeft(0);
		setPosition(0, 0);
		setSize(gaScreen->w/8, gaScreen->h/16);
		m_pixelOverlay.reset(new std::vector<uint8_t>(gaScreen->w*gaScreen->h, 0xff));
		clearOverlay();
		SDL_WM_SetCaption( title.c_str(), NULL );
		gDosColors[dcBlack]       = SDL_MapRGB( gaScreen->format, 0x00, 0x00, 0x00 ); // black
		gDosColors[dcBlue]        = SDL_MapRGB( gaScreen->format, 0x00, 0x00, 0xaa ); // blue
		gDosColors[dcGreen]       = SDL_MapRGB( gaScreen->format, 0x00, 0xaa, 0x00 ); // green
		gDosColors[dcAqua]        = SDL_MapRGB( gaScreen->format, 0x00, 0xaa, 0xaa ); // aqua
		gDosColors[dcRed]         = SDL_MapRGB( gaScreen->format, 0xaa, 0x00, 0x00 ); // red
		gDosColors[dcPurple]      = SDL_MapRGB( gaScreen->format, 0xaa, 0x00, 0xaa ); // purple
		gDosColors[dcBrown]       = SDL_MapRGB( gaScreen->format, 0xaa, 0x55, 0x00 ); // brown
		gDosColors[dcWhite]       = SDL_MapRGB( gaScreen->format, 0xaa, 0xaa, 0xaa ); // white
		gDosColors[dcGray]        = SDL_MapRGB( gaScreen->format, 0x55, 0x55, 0x55 ); // gray
		gDosColors[dcLightBlue]   = SDL_MapRGB( gaScreen->format, 0x55, 0x55, 0xff ); // light blue
		gDosColors[dcLightGreen]  = SDL_MapRGB( gaScreen->format, 0x55, 0xff, 0x55 ); // light green
		gDosColors[dcLightAqua]   = SDL_MapRGB( gaScreen->format, 0x55, 0xff, 0xff ); // light aqua
		gDosColors[dcLightRed]    = SDL_MapRGB( gaScreen->format, 0xff, 0x55, 0x55 ); // light red (orange?)
		gDosColors[dcLightPurple] = SDL_MapRGB( gaScreen->format, 0xff, 0x55, 0xff ); // light purple
		gDosColors[dcYellow]      = SDL_MapRGB( gaScreen->format, 0xff, 0xff, 0x55 ); // yellow
		gDosColors[dcBrightWhite] = SDL_MapRGB( gaScreen->format, 0xff, 0xff, 0xff ); // bright white
		int size = area().width() * area().height();
		gaChars.reset( new std::vector<char>(size) );
		gaColorsF.reset( new std::vector<uint8_t>(size) );
		gaColorsB.reset( new std::vector<uint8_t>(size) );
		clear( ' ', dcWhite, dcBlack );
	}
	setPosition(0,0);
	setSize(gaScreen->w/8, gaScreen->h/16);
}

Screen::~Screen() throw() {
}

#include "pfonts.inc"

void Screen::drawChar8( int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque ) throw() {
	x <<= 3;
	y <<= 3;
	for ( unsigned char py = 0; py < 8; py++ ) {
		for ( unsigned char px = 0; px < 8; px++ ) {
			if ( plFont88[c][py] & ( 0x80 >> px ) )
				DrawPixel( x + px, y + py, foreground );
			else if ( opaque )
				DrawPixel( x + px, y + py, background );
		}
	}
}

void Screen::drawChar16( int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque ) throw() {
	x <<= 3;
	y <<= 4;
	for ( unsigned char py = 0; py < 16; py++ ) {
		for ( unsigned char px = 0; px < 8; px++ ) {
			if ( plFont816[c][py] & ( 0x80 >> px ) )
				DrawPixel( x + px, y + py, foreground );
			else if ( opaque )
				DrawPixel( x + px, y + py, background );
		}
	}
}

void Screen::clear( uint8_t c, uint8_t foreground, uint8_t background ) throw() {
	std::size_t size = area().width()*area().height();
	std::fill_n(gaChars->begin(), size, c);
	std::fill_n(gaColorsF->begin(), size, foreground);
	std::fill_n(gaColorsB->begin(), size, background);
}

void Screen::drawThis() throw( Exception ) {
	if ( SDL_MUSTLOCK( gaScreen ) ) {
		if ( SDL_LockSurface( gaScreen ) < 0 )
			return;
	}
	int w = area().width();
	int h = area().height();
	for ( int y = 0; y < h; y++ ) {
		for ( int x = 0; x < w; x++ ) {
			drawChar16( x, y, gaChars->at(x+y*w), gDosColors[gaColorsF->at(x+y*w)], gDosColors[gaColorsB->at(x+y*w)], true );
		}
	}
	h = gaScreen->h;
	w = gaScreen->w;
	for ( int y = 0; y < h; y++ ) {
		for ( int x = 0; x < w; x++ ) {
			int pos = w * y + x;
			if ( m_pixelOverlay->at(pos) == 0xff )
				continue;
			DrawPixel( x, y, gDosColors[m_pixelOverlay->at(pos)] );
		}
	}
	if ( SDL_MUSTLOCK( gaScreen ) ) {
		SDL_UnlockSurface( gaScreen );
	}
	if ( SDL_Flip( gaScreen ) == -1 )
		PPG_THROW( "Flip failed" );
}

void Screen::drawChar( const int x, const int y, const char c ) throw() {
	int w = area().width();
	int h = area().height();
	if (( x < 0 ) || ( x >= w ) || ( y < 0 ) || ( y >= h ) )
		return;
	gaChars->at(x+y*w) = c;
}

void Screen::drawFgColor( const int x, const int y, const unsigned char c ) throw() {
	int w = area().width();
	int h = area().height();
	if (( x < 0 ) || ( x >= w ) || ( y < 0 ) || ( y >= h ) )
		return;
	gaColorsF->at(x+y*w) = c;
}

void Screen::drawBgColor( const int x, const int y, const unsigned char c ) throw() {
	int w = area().width();
	int h = area().height();
	if (( x < 0 ) || ( x >= w ) || ( y < 0 ) || ( y >= h ) )
		return;
	gaColorsB->at(x+y*w) = c;
}

StereoPeakBar::StereoPeakBar( Widget*parent, int width, int max, int interLen, bool showPeak ) throw( Exception ) : Label( parent, " " ),
		m_interArrL(), m_interArrR(), m_max( 0 ), m_barLength( 0 ), m_showPeak( false ),
		m_peakPosL( 0 ), m_peakPosR( 0 ), m_peakFalloffSpeedL( 0 ), m_peakFalloffSpeedR( 0 ) {
	PPG_TEST( width < 8 || interLen < 1 );
	m_interArrL = std::vector<int>(interLen, 0);
	m_interArrR = std::vector<int>(interLen, 0);
	m_showPeak = showPeak;
	m_barLength = width;
	m_peakPosL = m_peakFalloffSpeedL = 0;
	m_peakPosR = m_peakFalloffSpeedR = 0;
	m_max = max;
	std::string txt = "[";
	for ( int i = 0; i < width*2; i++ ) {
		txt += " ";
		if ( i + 1 == width )
			txt += "|";
	}
	txt += "]";
	setText(txt);
	setSize( length(), area().height() );
	int mpoint = length() / 2;
	for ( int i = 0; i < width; i++ ) {
		switch ( i*4 / width ) {
			case 3:
				setFgColor( mpoint + i + 1, dcLightRed );
				setFgColor( mpoint - i - 1, dcLightRed );
				break;
			case 2:
				setFgColor( mpoint + i + 1, dcYellow );
				setFgColor( mpoint - i - 1, dcYellow );
				break;
			case 1:
				setFgColor( mpoint + i + 1, dcGreen );
				setFgColor( mpoint - i - 1, dcGreen );
				break;
			case 0:
				setFgColor( mpoint + i + 1, dcLightBlue );
				setFgColor( mpoint - i - 1, dcLightBlue );
				break;
		}
	}
	setFgColor( 0, dcGreen );
	setFgColor( mpoint, dcGreen );
	setFgColor( length() - 1, dcGreen );
}

StereoPeakBar::~StereoPeakBar() throw() {
}

void StereoPeakBar::shift( int lval, int rval ) throw( Exception ) {
	PPG_TEST( m_interArrL.size()==0 || m_interArrR.size()!=m_interArrL.size() );
	m_interArrL.erase(m_interArrL.begin());
	m_interArrR.erase(m_interArrR.begin());
	m_interArrL.push_back(lval);
	m_interArrR.push_back(rval);
	if ( !m_showPeak )
		return;
	if ( getValLeft() > m_peakPosL ) {
		m_peakFalloffSpeedL = 0;
		m_peakPosL = getValLeft();
	}
	else {
		m_peakFalloffSpeedL++;
		m_peakPosL = std::max( 0, m_peakPosL - ( m_peakFalloffSpeedL >> 2 ) );
		if ( m_peakPosL == 0 )
			m_peakFalloffSpeedL = 0;
	}
	if ( getValRight() > m_peakPosR ) {
		m_peakFalloffSpeedR = 0;
		m_peakPosR = getValRight();
	}
	else {
		m_peakFalloffSpeedR++;
		m_peakPosR = std::max( 0, m_peakPosR - ( m_peakFalloffSpeedR >> 2 ) );
		if ( m_peakPosR == 0 )
			m_peakFalloffSpeedR = 0;
	}
	int valL = getValLeft();
	int valR = getValRight();
	int mpoint = length() / 2;
	for ( int i = 0; i < m_barLength; i++ ) {
		if ( valL > i*m_max / m_barLength )
			(*this)[mpoint-1-i] = static_cast<char>( 0xfe );
		else
			(*this)[mpoint-1-i] = static_cast<char>( 0xf9 );
		if (( m_peakPosL != 0 ) && m_showPeak )
			(*this)[mpoint-1-m_peakPosL*m_barLength/m_max] = static_cast<char>( 0x04 );
		if ( valR > i*m_max / m_barLength )
			(*this)[mpoint+1+i] = static_cast<char>( 0xfe );
		else
			(*this)[mpoint+1+i] = static_cast<char>( 0xf9 );
		if (( m_peakPosR != 0 ) && m_showPeak )
			(*this)[mpoint+1+m_peakPosR*m_barLength/m_max] = static_cast<char>( 0x04 );
	}
}

void StereoPeakBar::shiftFrac( const float lval, const float rval ) throw( Exception ) {
	shift( static_cast<int>( lval*m_max ), static_cast<int>( rval*m_max ) );
}

int StereoPeakBar::getValLeft() const throw() {
	int res = 0;
	int div = 0;
	int i = 0;
	std::for_each(
			m_interArrL.begin(),
			m_interArrL.end(),
			[&res, &div, &i](int v){ i++; res+=v*i; div+=i; }
		);
	return res / div;
}

int StereoPeakBar::getValRight() const throw() {
	int res = 0;
	int div = 0;
	int i = 0;
	std::for_each(
			m_interArrR.begin(),
			m_interArrR.end(),
			[&res, &div, &i](int v){ i++; res+=v*i; div+=i; }
		);
	return res / div;
}

} // namespace ppg
