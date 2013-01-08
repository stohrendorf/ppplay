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

namespace
{
light4cxx::Logger* logger = light4cxx::Logger::get( "ppg.sdl" );
}

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg
{

namespace
{
/**
 * @struct InstanceData
 * @brief Internal data and functions
 * @details
 * The ABC and visibleABC arrays are compared everytime the screen needs to be redrawn.@n
 * If at least one of the foreground colors, background colors or characters differ, it is redrawn,
 * otherwise it is skipped. See ppg::SDLScreen::drawThis().@n
 * This reduces the graphical overhead significantly.
 */
struct InstanceData
{
	DISABLE_COPY(InstanceData)
	/**
	 * @brief Maps DOS color values to their on-screen representation
	 * @see ppg::Color
	 */
	Uint32 dosColors[17];
	
	inline Uint32 mapColor(ppg::Color c) const
	{
		return dosColors[static_cast<int>(c)];
	}
	
	/**
	 * @brief Contains the chars to be displayed
	 * @see visibleChars
	 */
	char* chars;
	/**
	 * @brief Contains the chars currently visible to determine what needs to be drawn
	 * @see chars
	 */
	char* visibleChars;
	/**
	 * @brief Contains the foreground colors to be displayed
	 */
	Color* colorsF;
	/**
	 * @brief Contains the foreground colors currently visible to determine what needs to be drawn
	 */
	Color* visibleColorsF;
	/**
	 * @brief Contains the background colors to be displayed
	 */
	Color* colorsB;
	/**
	 * @brief Contains the background colors currently visible to determine what needs to be drawn
	 */
	Color* visibleColorsB;
	/**
	 * @brief The current SDL screen surface instance
	 */
	SDL_Surface* screenSurface;
	
	SDL_Surface* backgroundLayer;
	SDL_Surface* pixelLayer;
	SDL_Surface* foregroundLayer;
	
	SDLScreen* screen;
	
	size_t charWidth;
	size_t charHeight;
	
	constexpr InstanceData()
	: dosColors{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	, chars(nullptr), visibleChars(nullptr)
	, colorsF(nullptr), visibleColorsF(nullptr)
	, colorsB(nullptr), visibleColorsB(nullptr)
	, screenSurface(nullptr)
	, backgroundLayer(nullptr), pixelLayer(nullptr), foregroundLayer(nullptr)
	, screen(nullptr)
	, charWidth(0), charHeight(0)
	{
	}
	
	~InstanceData()
	{
		reset();
	}
	
	inline void reset()
	{
		delete[] chars;
		chars = nullptr;
		
		delete[] visibleChars;
		visibleChars = nullptr;
		
		delete[] colorsF;
		colorsF = nullptr;
		
		delete[] visibleColorsF;
		visibleColorsF = nullptr;
		
		delete[] colorsB;
		colorsB = nullptr;
		
		delete[] visibleColorsB;
		visibleColorsB = nullptr;
		
		SDL_FreeSurface( backgroundLayer );
		backgroundLayer = nullptr;
		
		SDL_FreeSurface( pixelLayer );
		pixelLayer = nullptr;
		
		SDL_FreeSurface( foregroundLayer );
		foregroundLayer = nullptr;
	}
	
	bool contains(int x, int y) const
	{
		return ( x >= 0 ) && ( y >= 0 ) && ( y < screenSurface->h ) && ( x < screenSurface->w );
	}
	
	/**
	 * @brief Draw a pixel
	 * @param[in,out] surface Surface
	 * @param[in] x X position
	 * @param[in] y Y position
	 * @param[in] color %Screen color value
	 */
	inline bool setPixel(SDL_Surface* surface, int x, int y, Uint32 color) const
	{
		if(contains(x,y)) {
			if(SDL_MUSTLOCK(surface))  {
				SDL_LockSurface( surface );
			}
			reinterpret_cast<Uint32*>( surface->pixels )[ y * surface->pitch / sizeof(Uint32) + x] = color;
			if(SDL_MUSTLOCK(surface))  {
				SDL_UnlockSurface( surface );
			}
			return true;
		}
		return false;
	}
	
	void clearPixels(Color c = Color::None)
	{
		clearSurface( pixelLayer, c );
	}

	void clearSurface(SDL_Surface* surface, Color c) const
	{
		if(SDL_MUSTLOCK(surface))  {
			SDL_LockSurface( surface );
		}
		std::fill_n(reinterpret_cast<Uint32*>(surface->pixels), surface->h * surface->pitch / sizeof(Uint32), mapColor(c));
		if(SDL_MUSTLOCK(surface))  {
			SDL_UnlockSurface( surface );
		}
	}
	
	bool init(int charWidth, int charHeight, const std::string& title);
	
	/**
	 * @brief Draw an 8x8 char
	 * @param[in] x Left position
	 * @param[in] y Top position
	 * @param[in] c Char to draw
	 * @param[in] foreground Foreground color
	 * @param[in] background Background color
	 * @param[in] opaque Set to @c false to draw a transparent char
	 */
	inline void drawChar( int x, int y, char c, Uint32 foreground, Uint32 background, bool opaque = true)
	{
		x <<= 3;
		y <<= 4;
		for( int py = 0; py < 16; py++ ) {
			for( int px = 0; px < 8; px++ ) {
				if( plFont816[uint8_t(c)][py] & ( 0x80 >> px ) ) {
					setPixel( foregroundLayer, x + px, y + py, foreground );
				}
				else {
					setPixel( foregroundLayer, x + px, y + py, mapColor(Color::None) );
				}
				if( opaque ) {
					setPixel( backgroundLayer, x + px, y + py, background );
				}
			}
		}
	}
	
	void clear( char c, ppg::Color foreground, ppg::Color background );
	void redraw( bool showMouse, int cursorX, int cursorY );
};

bool InstanceData::init( int charWidth, int charHeight, const std::string& title )
{
	if(screenSurface != nullptr) {
		return false;
	}
	if( !SDL_WasInit( SDL_INIT_VIDEO ) ) {
		if( SDL_Init( SDL_INIT_VIDEO ) == -1 ) {
			BOOST_THROW_EXCEPTION( std::runtime_error( "Initialization of SDL Video surface failed" ) );
		}
	}
	screenSurface = SDL_SetVideoMode( charWidth * 8, charHeight * 16, 32, SDL_DOUBLEBUF );
	if( !screenSurface ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( "Screen Initialization failed" ) );
	}
	{
		char videoDrv[256];
		if( SDL_VideoDriverName( videoDrv, 255 ) ) {
			logger->info( L4CXX_LOCATION, "Using video driver '%s'", videoDrv );
		}
	}
	SDL_WM_SetCaption( title.c_str(), nullptr );
	
	backgroundLayer = SDL_CreateRGBSurface(SDL_SWSURFACE,              charWidth*8, charHeight*16, 32, 0xff, 0xff<<8, 0xff<<16, 0       );
	pixelLayer      = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, charWidth*8, charHeight*16, 32, 0xff, 0xff<<8, 0xff<<16, 0xff<<24);
	foregroundLayer = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, charWidth*8, charHeight*16, 32, 0xff, 0xff<<8, 0xff<<16, 0xff<<24);
	clearSurface( backgroundLayer, Color::Black );
	
	dosColors[static_cast<int>( Color::None )]        = SDL_MapRGBA( pixelLayer->format, 0x00, 0x00, 0x00, 0 ); // transparent
	dosColors[static_cast<int>( Color::Black )]       = SDL_MapRGB( pixelLayer->format, 0x00, 0x00, 0x00 ); // black
	dosColors[static_cast<int>( Color::Blue )]        = SDL_MapRGB( pixelLayer->format, 0x00, 0x00, 0xaa ); // blue
	dosColors[static_cast<int>( Color::Green )]       = SDL_MapRGB( pixelLayer->format, 0x00, 0xaa, 0x00 ); // green
	dosColors[static_cast<int>( Color::Aqua )]        = SDL_MapRGB( pixelLayer->format, 0x00, 0xaa, 0xaa ); // aqua
	dosColors[static_cast<int>( Color::Red )]         = SDL_MapRGB( pixelLayer->format, 0xaa, 0x00, 0x00 ); // red
	dosColors[static_cast<int>( Color::Purple )]      = SDL_MapRGB( pixelLayer->format, 0xaa, 0x00, 0xaa ); // purple
	dosColors[static_cast<int>( Color::Brown )]       = SDL_MapRGB( pixelLayer->format, 0xaa, 0x55, 0x00 ); // brown
	dosColors[static_cast<int>( Color::White )]       = SDL_MapRGB( pixelLayer->format, 0xaa, 0xaa, 0xaa ); // white
	dosColors[static_cast<int>( Color::Gray )]        = SDL_MapRGB( pixelLayer->format, 0x55, 0x55, 0x55 ); // gray
	dosColors[static_cast<int>( Color::LightBlue )]   = SDL_MapRGB( pixelLayer->format, 0x55, 0x55, 0xff ); // light blue
	dosColors[static_cast<int>( Color::LightGreen )]  = SDL_MapRGB( pixelLayer->format, 0x55, 0xff, 0x55 ); // light green
	dosColors[static_cast<int>( Color::LightAqua )]   = SDL_MapRGB( pixelLayer->format, 0x55, 0xff, 0xff ); // light aqua
	dosColors[static_cast<int>( Color::LightRed )]    = SDL_MapRGB( pixelLayer->format, 0xff, 0x55, 0x55 ); // light red (orange?)
	dosColors[static_cast<int>( Color::LightPurple )] = SDL_MapRGB( pixelLayer->format, 0xff, 0x55, 0xff ); // light purple
	dosColors[static_cast<int>( Color::Yellow )]      = SDL_MapRGB( pixelLayer->format, 0xff, 0xff, 0x55 ); // yellow
	dosColors[static_cast<int>( Color::BrightWhite )] = SDL_MapRGB( pixelLayer->format, 0xff, 0xff, 0xff ); // bright white
	
	this->charWidth = charWidth;
	this->charHeight = charHeight;
	const int size = charWidth * charHeight;
	
	chars = new char[size];
	std::fill_n( chars, size, ' ' );
	
	visibleChars = new char[size];
	std::fill_n( visibleChars, size, ' ' );
	
	colorsF = new Color[size];
	std::fill_n( colorsF, size, Color::White );
	
	visibleColorsF = new Color[size];
	std::fill_n( visibleColorsF, size, Color::White );
	
	colorsB = new Color[size];
	std::fill_n( colorsB, size, Color::Black );
	
	visibleColorsB = new Color[size];
	std::fill_n( visibleColorsB, size, Color::Black );

	return true;
}

void InstanceData::clear(char c, ppg::Color foreground, ppg::Color background)
{
	const size_t size = charWidth * charHeight;
	std::fill_n( chars, size, c );
	std::fill_n( colorsF, size, foreground );
	std::fill_n( colorsB, size, background );
	clearPixels();
}

void InstanceData::redraw(bool showMouse, int cursorX, int cursorY)
{
	if( SDL_MUSTLOCK( screenSurface ) ) {
		if( SDL_LockSurface( screenSurface ) < 0 ) {
			return;
		}
	}
	for( size_t y = 0; y < charHeight; y++ ) {
		for( size_t x = 0; x < charWidth; x++ ) {
			int o = x + y * charWidth;
			if( chars[o] != visibleChars[o] || colorsF[o] != visibleColorsF[o] || colorsB[o] != visibleColorsB[o] ) {
				drawChar( x, y, chars[o], mapColor( colorsF[o] ), mapColor( colorsB[o] ), true );
			}
		}
	}
	{
		size_t size = charWidth*charHeight;
		std::copy_n( chars, size, visibleChars );
		std::copy_n( colorsF, size, visibleColorsF );
		std::copy_n( colorsB, size, visibleColorsB );
	}
	if( showMouse && contains( cursorX, cursorY ) ) {
		size_t ofs = cursorX + cursorY * charWidth;
		Uint32 c1 = mapColor( ~colorsF[ofs] );
		Uint32 c2 = mapColor( ~colorsB[ofs] );
		drawChar( cursorX, cursorY, chars[ofs], c1, c2, true );
		visibleColorsF[ofs] = ~colorsF[ofs];
		visibleColorsB[ofs] = ~colorsB[ofs];
	}
	SDL_BlitSurface( backgroundLayer, nullptr, screenSurface, nullptr );
	SDL_BlitSurface( pixelLayer,      nullptr, screenSurface, nullptr );
	SDL_BlitSurface( foregroundLayer, nullptr, screenSurface, nullptr );
	if( SDL_MUSTLOCK( screenSurface ) ) {
		SDL_UnlockSurface( screenSurface );
	}
	if( SDL_Flip( screenSurface ) == -1 ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( "Flip failed" ) );
	}
}

InstanceData instanceData;

} // anonymous namespace

SDLScreen* SDLScreen::instance()
{
	return instanceData.screen;
}

SDLScreen::SDLScreen( int w, int h, const std::string& title ) : Widget( nullptr ), m_cursorX( 0 ), m_cursorY( 0 )
{
	if( !instanceData.init(w,h,title) ) {
		throw std::runtime_error( "SDL Screen Surface already aquired" );
	}
	setPosition( 0, 0 );
	setSize( instanceData.screenSurface->w / 8, instanceData.screenSurface->h / 16 );
	SDL_ShowCursor( 0 );
	instanceData.screen = this;
}

SDLScreen::~SDLScreen()
{
	instanceData.screen = nullptr;
	instanceData.reset();
}

void SDLScreen::clear( uint8_t c, Color foreground, Color background )
{
	instanceData.clear(c, foreground, background);
}

void SDLScreen::drawThis()
{
	instanceData.redraw(hasMouseFocus(), m_cursorX, m_cursorY);
}

void SDLScreen::drawChar( int x, int y, char c )
{
	if( !instanceData.contains( x, y ) ) {
		logger->error( L4CXX_LOCATION, "Out of range: %d, %d", x, y );
		return;
	}
	instanceData.chars[x + y * instanceData.charWidth] = c;
}

void SDLScreen::setFgColorAt( int x, int y, Color c )
{
	if( !instanceData.contains( x, y ) ) {
		return;
	}
	instanceData.colorsF[x + y * instanceData.charWidth] = c;
}

void SDLScreen::setBgColorAt( int x, int y, Color c )
{
	if( !instanceData.contains( x, y ) ) {
		return;
	}
	instanceData.colorsB[x + y * instanceData.charWidth] = c;
}

bool SDLScreen::onMouseMove( int x, int y )
{
	m_cursorX = x;
	m_cursorY = y;
	Widget::onMouseMove( x, y );
	return true;
}

bool SDLScreen::hasMouseFocus() const
{
	return ( SDL_GetAppState()&SDL_APPMOUSEFOCUS ) != 0;
}

void SDLScreen::drawPixel( int x, int y, Color c )
{
	instanceData.setPixel(instanceData.pixelLayer, x, y, instanceData.mapColor(c));
}

void SDLScreen::clearPixels( Color c )
{
	instanceData.clearPixels(c);
}

} // namespace ppg

/**
 * @}
 */
