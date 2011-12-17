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

#include <boost/exception/all.hpp>

#include "sdlscreen.h"
#include "fonts.h"

#include "light4cxx/logger.h"

#include <SDL.h>
#include <SDL_endian.h>

#include <boost/format.hpp>

static light4cxx::Logger::Ptr logger = light4cxx::Logger::get("ppg.sdl");

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg {

	/**
	 * @name Internal data and functions
	 * @{
	 * @details
	 * The g_ABC and g_currentABC arrays are compared everytime the screen needs to be redrawn.@n
	 * If at least one of the foreground colors, background colors or characters differ, it is redrawn,
	 * otherwise it is skipped. See ppg::SDLScreen::drawThis().@n
	 * This reduces the graphical overhead significantly.
	 */
	/**
	 * @brief Maps DOS color values to their on-screen representation
	 * @see ppg::Color
	 */
	static Uint32 g_dosColors[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	/**
	 * @brief Contains the chars to be displayed
	 * @see g_currentChars
	 */
	static char* g_chars = nullptr;
	/**
	 * @brief Contains the chars currently visible to determine what needs to be drawn
	 * @see g_chars
	 */
	static char* g_currentChars = nullptr;
	/**
	 * @brief Contains the foreground colors to be displayed
	 */
	static Color* g_colorsF = nullptr;
	/**
	 * @brief Contains the foreground colors currently visible to determine what needs to be drawn
	 */
	static Color* g_currentColorsF = nullptr;
	/**
	 * @brief Contains the background colors to be displayed
	 */
	static Color* g_colorsB = nullptr;
	/**
	 * @brief Contains the background colors currently visible to determine what needs to be drawn
	 */
	static Color* g_currentColorsB = nullptr;
	/**
	 * @brief The current SDL screen surface instance
	 */
	static SDL_Surface* g_screenSurface = nullptr;
	
	static SDLScreen* g_instance = nullptr;
	
	SDLScreen* SDLScreen::instance()
	{
		return g_instance;
	}


	/**
	 * @brief Draw a pixel
	 * @param[in] x X position
	 * @param[in] y Y position
	 * @param[in] color %Screen color value
	 */
	static inline void g_drawPixel( int x, int y, Uint32 color ) {
		if( ( x < 0 ) || ( y < 0 ) || ( y >= g_screenSurface->h ) || ( x >= g_screenSurface->w ) ) {
			return;
		}
		reinterpret_cast<Uint32*>( g_screenSurface->pixels )[( ( y * g_screenSurface->pitch ) >> 2 ) + x] = color;
	}
	/**
	 * @}
	 */

	SDLScreen::SDLScreen( int w, int h, const std::string& title ) : Widget( nullptr ), m_cursorX( 0 ), m_cursorY( 0 ) {
		BOOST_ASSERT( g_screenSurface == nullptr );
		if( !SDL_WasInit( SDL_INIT_VIDEO ) ) {
			if( SDL_Init( SDL_INIT_VIDEO ) == -1 ) {
				BOOST_THROW_EXCEPTION( std::runtime_error( "Initialization of SDL Video surface failed" ) );
			}
		}
		Uint8 bestBpp = 32;
		Uint32 bestFlags = SDL_DOUBLEBUF;
		{
			const SDL_VideoInfo* info = SDL_GetVideoInfo();
			if( info->vfmt ) {
				bestBpp = info->vfmt->BitsPerPixel;
			}
// 			if( info->hw_available ) {
// 				bestFlags |= SDL_HWSURFACE;
// 			}
// 			else {
// 				bestFlags |= SDL_SWSURFACE;
// 			}
		}
		g_screenSurface = SDL_SetVideoMode( w * 8, h * 16, bestBpp, bestFlags );
		if( !g_screenSurface ) {
			BOOST_THROW_EXCEPTION( std::runtime_error("Screen Initialization failed") );
		}
		{
			char videoDrv[256];
			if( SDL_VideoDriverName( videoDrv, 255 ) ) {
				logger->info( L4CXX_LOCATION, boost::format("Using video driver '%s'")%videoDrv );
			}
		}
		setPosition( 0, 0 );
		setSize( g_screenSurface->w / 8, g_screenSurface->h / 16 );
		SDL_WM_SetCaption( title.c_str(), nullptr );
		g_dosColors[static_cast<int>(Color::Black)]       = SDL_MapRGB( g_screenSurface->format, 0x00, 0x00, 0x00 ); // black
		g_dosColors[static_cast<int>(Color::Blue)]        = SDL_MapRGB( g_screenSurface->format, 0x00, 0x00, 0xaa ); // blue
		g_dosColors[static_cast<int>(Color::Green)]       = SDL_MapRGB( g_screenSurface->format, 0x00, 0xaa, 0x00 ); // green
		g_dosColors[static_cast<int>(Color::Aqua)]        = SDL_MapRGB( g_screenSurface->format, 0x00, 0xaa, 0xaa ); // aqua
		g_dosColors[static_cast<int>(Color::Red)]         = SDL_MapRGB( g_screenSurface->format, 0xaa, 0x00, 0x00 ); // red
		g_dosColors[static_cast<int>(Color::Purple)]      = SDL_MapRGB( g_screenSurface->format, 0xaa, 0x00, 0xaa ); // purple
		g_dosColors[static_cast<int>(Color::Brown)]       = SDL_MapRGB( g_screenSurface->format, 0xaa, 0x55, 0x00 ); // brown
		g_dosColors[static_cast<int>(Color::White)]       = SDL_MapRGB( g_screenSurface->format, 0xaa, 0xaa, 0xaa ); // white
		g_dosColors[static_cast<int>(Color::Gray)]        = SDL_MapRGB( g_screenSurface->format, 0x55, 0x55, 0x55 ); // gray
		g_dosColors[static_cast<int>(Color::LightBlue)]   = SDL_MapRGB( g_screenSurface->format, 0x55, 0x55, 0xff ); // light blue
		g_dosColors[static_cast<int>(Color::LightGreen)]  = SDL_MapRGB( g_screenSurface->format, 0x55, 0xff, 0x55 ); // light green
		g_dosColors[static_cast<int>(Color::LightAqua)]   = SDL_MapRGB( g_screenSurface->format, 0x55, 0xff, 0xff ); // light aqua
		g_dosColors[static_cast<int>(Color::LightRed)]    = SDL_MapRGB( g_screenSurface->format, 0xff, 0x55, 0x55 ); // light red (orange?)
		g_dosColors[static_cast<int>(Color::LightPurple)] = SDL_MapRGB( g_screenSurface->format, 0xff, 0x55, 0xff ); // light purple
		g_dosColors[static_cast<int>(Color::Yellow)]      = SDL_MapRGB( g_screenSurface->format, 0xff, 0xff, 0x55 ); // yellow
		g_dosColors[static_cast<int>(Color::BrightWhite)] = SDL_MapRGB( g_screenSurface->format, 0xff, 0xff, 0xff ); // bright white
		int size = area().width() * area().height();
		g_chars = new char[size];
		g_currentChars = new char[size];
		g_colorsF = new Color[size];
		g_currentColorsF = new Color[size];
		g_colorsB = new Color[size];
		g_currentColorsB = new Color[size];
		clear( ' ', Color::White, Color::Black );
		std::fill_n( g_currentChars, size, ' ' );
		std::fill_n( g_currentColorsF, size, Color::White );
		std::fill_n( g_currentColorsB, size, Color::Black );
		SDL_ShowCursor( 0 );
		g_instance = this;
	}

	SDLScreen::~SDLScreen() {
		g_instance = nullptr;
		delete[] g_chars;
		g_chars = nullptr;
		delete[] g_currentChars;
		g_currentChars = nullptr;
		delete[] g_colorsF;
		g_colorsF = nullptr;
		delete[] g_currentColorsF;
		g_currentColorsF = nullptr;
		delete[] g_colorsB;
		g_colorsB = nullptr;
		delete[] g_currentColorsB;
		g_currentColorsB = nullptr;
	}

	void SDLScreen::drawChar8( int x, int y, uint8_t c, uint32_t foreground, uint32_t background, bool opaque ) {
		x <<= 3;
		y <<= 3;
		for( unsigned char py = 0; py < 8; py++ ) {
			for( unsigned char px = 0; px < 8; px++ ) {
				if( plFont88[c][py] & ( 0x80 >> px ) ) {
					g_drawPixel( x + px, y + py, foreground );
				}
				else if( opaque ) {
					g_drawPixel( x + px, y + py, background );
				}
			}
		}
	}

	void SDLScreen::drawChar16( int x, int y, uint8_t c, uint32_t foreground, uint32_t background, bool opaque ) {
		x <<= 3;
		y <<= 4;
		for( unsigned char py = 0; py < 16; py++ ) {
			for( unsigned char px = 0; px < 8; px++ ) {
				if( plFont816[c][py] & ( 0x80 >> px ) ) {
					g_drawPixel( x + px, y + py, foreground );
				}
				else if( opaque ) {
					g_drawPixel( x + px, y + py, background );
				}
			}
		}
	}

	void SDLScreen::clear( uint8_t c, Color foreground, Color background ) {
		size_t size = area().width() * area().height();
		std::fill_n( g_chars, size, c );
		std::fill_n( g_colorsF, size, foreground );
		std::fill_n( g_colorsB, size, background );
	}

	void SDLScreen::drawThis() {
		if( SDL_MUSTLOCK( g_screenSurface ) ) {
			if( SDL_LockSurface( g_screenSurface ) < 0 ) {
				return;
			}
		}
		int w = area().width();
		int h = area().height();
		for( int y = 0; y < h; y++ ) {
			for( int x = 0; x < w; x++ ) {
				int o = x + y * w;
				if(g_chars[o]!=g_currentChars[o] || g_colorsF[o]!=g_currentColorsF[o] || g_colorsB[o]!=g_currentColorsB[o]) {
					drawChar16( x, y, g_chars[o], g_dosColors[static_cast<int>(g_colorsF[o])], g_dosColors[static_cast<int>(g_colorsB[o])], true );
				}
			}
		}
		{
			size_t size = w * h;
			std::copy(g_chars, g_chars+size, g_currentChars);
			std::copy(g_colorsF, g_colorsF+size, g_currentColorsF);
			std::copy(g_colorsB, g_colorsB+size, g_currentColorsB);
		}
		if( hasMouseFocus() && area().contains( m_cursorX, m_cursorY ) ) {
			size_t ofs = m_cursorX + m_cursorY * w;
			Uint32 c1 = g_dosColors[static_cast<int>(~g_colorsF[ofs])];
			Uint32 c2 = g_dosColors[static_cast<int>(~g_colorsB[ofs])];
			drawChar16( m_cursorX, m_cursorY, g_chars[ofs], c1, c2, true );
			g_currentColorsF[ofs] = ~g_colorsF[ofs];
			g_currentColorsB[ofs] = ~g_colorsB[ofs];
		}
		if( SDL_MUSTLOCK( g_screenSurface ) ) {
			SDL_UnlockSurface( g_screenSurface );
		}
		if( SDL_Flip( g_screenSurface ) == -1 ) {
			BOOST_THROW_EXCEPTION( std::runtime_error("Flip failed") );
		}
	}

	void SDLScreen::drawChar( int x, int y, char c ) {
		if( !area().contains( x, y ) ) {
			logger->error( L4CXX_LOCATION, boost::format("Out of range: %d, %d")%x%y );
			return;
		}
		g_chars[x + y * area().width()] = c;
	}

	void SDLScreen::setFgColorAt( int x, int y, Color c ) {
		if( !area().contains( x, y ) )
			return;
		g_colorsF[x + y * area().width()] = c;
	}

	void SDLScreen::setBgColorAt( int x, int y, Color c ) {
		if( !area().contains( x, y ) )
			return;
		g_colorsB[x + y * area().width()] = c;
	}

	bool SDLScreen::onMouseMove( int x, int y ) {
		m_cursorX = x;
		m_cursorY = y;
		Widget::onMouseMove( x, y );
		return true;
	}
	
	bool SDLScreen::hasMouseFocus() const
	{
		return (SDL_GetAppState()&SDL_APPMOUSEFOCUS)!=0;
	}

} // namespace ppg

/**
 * @}
 */

