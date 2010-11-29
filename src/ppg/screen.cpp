#include <SDL.h>
#include <SDL_endian.h>

#include "screen.h"
#include "ppg.h"

#include "logger.h"

namespace ppg {

static Uint32 g_dosColors[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static char* g_chars = NULL;
static uint8_t* g_colorsF = NULL;
static uint8_t* g_colorsB = NULL;
static uint8_t* g_pixelOverlay = NULL; //!< @brief Pixel overlay buffer
static SDL_Surface* g_screenSurface = NULL;

static inline void g_drawPixel( int x, int y, Uint32 color ) throw() {
	if (( x < 0 ) || ( y < 0 ) || ( y >= g_screenSurface->h ) || ( x >= g_screenSurface->w ) )
		return;
	reinterpret_cast<Uint32*>( g_screenSurface->pixels )[(( y*g_screenSurface->pitch ) >> 2 ) + x] = color;
}

static inline void g_drawPixelRGB( int x, int y, Uint8 R, Uint8 G, Uint8 B ) throw( Exception ) {
	g_drawPixel( x, y, SDL_MapRGB( g_screenSurface->format, R, G, B ) );
}

void Screen::clearOverlay() {
	std::fill_n(g_pixelOverlay, g_screenSurface->w*g_screenSurface->h, 0xff);
}

Screen::Screen( int w, int h, const std::string &title ) throw( Exception ) : Widget( NULL ) {
	PPG_TEST(g_screenSurface != NULL);
	g_screenSurface = SDL_SetVideoMode( w * 8, h * 16, 32, SDL_SWSURFACE );
	if ( !g_screenSurface ) {
		PPG_THROW( "Screen Initialization failed" );
	}
	setPosition(0, 0);
	setSize(g_screenSurface->w/8, g_screenSurface->h/16);
	g_pixelOverlay = new uint8_t[g_screenSurface->w*g_screenSurface->h];
	clearOverlay();
	SDL_WM_SetCaption( title.c_str(), NULL );
	g_dosColors[dcBlack]       = SDL_MapRGB( g_screenSurface->format, 0x00, 0x00, 0x00 ); // black
	g_dosColors[dcBlue]        = SDL_MapRGB( g_screenSurface->format, 0x00, 0x00, 0xaa ); // blue
	g_dosColors[dcGreen]       = SDL_MapRGB( g_screenSurface->format, 0x00, 0xaa, 0x00 ); // green
	g_dosColors[dcAqua]        = SDL_MapRGB( g_screenSurface->format, 0x00, 0xaa, 0xaa ); // aqua
	g_dosColors[dcRed]         = SDL_MapRGB( g_screenSurface->format, 0xaa, 0x00, 0x00 ); // red
	g_dosColors[dcPurple]      = SDL_MapRGB( g_screenSurface->format, 0xaa, 0x00, 0xaa ); // purple
	g_dosColors[dcBrown]       = SDL_MapRGB( g_screenSurface->format, 0xaa, 0x55, 0x00 ); // brown
	g_dosColors[dcWhite]       = SDL_MapRGB( g_screenSurface->format, 0xaa, 0xaa, 0xaa ); // white
	g_dosColors[dcGray]        = SDL_MapRGB( g_screenSurface->format, 0x55, 0x55, 0x55 ); // gray
	g_dosColors[dcLightBlue]   = SDL_MapRGB( g_screenSurface->format, 0x55, 0x55, 0xff ); // light blue
	g_dosColors[dcLightGreen]  = SDL_MapRGB( g_screenSurface->format, 0x55, 0xff, 0x55 ); // light green
	g_dosColors[dcLightAqua]   = SDL_MapRGB( g_screenSurface->format, 0x55, 0xff, 0xff ); // light aqua
	g_dosColors[dcLightRed]    = SDL_MapRGB( g_screenSurface->format, 0xff, 0x55, 0x55 ); // light red (orange?)
	g_dosColors[dcLightPurple] = SDL_MapRGB( g_screenSurface->format, 0xff, 0x55, 0xff ); // light purple
	g_dosColors[dcYellow]      = SDL_MapRGB( g_screenSurface->format, 0xff, 0xff, 0x55 ); // yellow
	g_dosColors[dcBrightWhite] = SDL_MapRGB( g_screenSurface->format, 0xff, 0xff, 0xff ); // bright white
	int size = area().width() * area().height();
	g_chars = new char[size];
	g_colorsF = new uint8_t[size];
	g_colorsB = new uint8_t[size];
	clear( ' ', dcWhite, dcBlack );
}

Screen::~Screen() throw() {
	delete[] g_pixelOverlay;
	g_pixelOverlay = NULL;
	delete[] g_chars;
	g_chars = NULL;
	delete[] g_colorsF;
	g_colorsF = NULL;
	delete[] g_colorsB;
	g_colorsB = NULL;
}

#include "pfonts.inc"

void Screen::drawChar8( int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque ) throw() {
	x <<= 3;
	y <<= 3;
	for ( unsigned char py = 0; py < 8; py++ ) {
		for ( unsigned char px = 0; px < 8; px++ ) {
			if ( plFont88[c][py] & ( 0x80 >> px ) )
				g_drawPixel( x + px, y + py, foreground );
			else if ( opaque )
				g_drawPixel( x + px, y + py, background );
		}
	}
}

void Screen::drawChar16( int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque ) throw() {
	x <<= 3;
	y <<= 4;
	for ( unsigned char py = 0; py < 16; py++ ) {
		for ( unsigned char px = 0; px < 8; px++ ) {
			if ( plFont816[c][py] & ( 0x80 >> px ) )
				g_drawPixel( x + px, y + py, foreground );
			else if ( opaque )
				g_drawPixel( x + px, y + py, background );
		}
	}
}

void Screen::clear( uint8_t c, uint8_t foreground, uint8_t background ) throw() {
	std::size_t size = area().width()*area().height();
	std::fill_n(g_chars, size, c);
	std::fill_n(g_colorsF, size, foreground);
	std::fill_n(g_colorsB, size, background);
}

void Screen::drawThis() throw( Exception ) {
	if ( SDL_MUSTLOCK( g_screenSurface ) ) {
		if ( SDL_LockSurface( g_screenSurface ) < 0 )
			return;
	}
	int w = area().width();
	int h = area().height();
	for ( int y = 0; y < h; y++ ) {
		for ( int x = 0; x < w; x++ ) {
			drawChar16( x, y, g_chars[x+y*w], g_dosColors[g_colorsF[x+y*w]], g_dosColors[g_colorsB[x+y*w]], true );
		}
	}
	h = g_screenSurface->h;
	w = g_screenSurface->w;
	int pos = -1;
	for ( int y = 0; y < h; y++ ) {
		for ( int x = 0; x < w; x++ ) {
			pos++;
			if ( g_pixelOverlay[pos] == 0xff )
				continue;
			g_drawPixel( x, y, g_dosColors[g_pixelOverlay[pos]] );
		}
	}
	if ( SDL_MUSTLOCK( g_screenSurface ) ) {
		SDL_UnlockSurface( g_screenSurface );
	}
	if ( SDL_Flip( g_screenSurface ) == -1 )
		PPG_THROW( "Flip failed" );
}

void Screen::drawChar( int x, int y, char c ) throw() {
	if(!area().contains(x,y)) {
		LOG_ERROR("Out of range: %d,%d", x, y);
		return;
	}
	g_chars[x+y*area().width()] = c;
}

void Screen::drawFgColor( int x, int y, uint8_t c ) throw() {
	if(!area().contains(x,y))
		return;
	g_colorsF[x+y*area().width()] = c;
}

void Screen::drawBgColor( int x, int y, uint8_t c ) throw() {
	if(!area().contains(x,y))
		return;
	g_colorsB[x+y*area().width()] = c;
}

void Screen::drawPixel(int x, int y, uint8_t color) {
	if(!area().contains(x/8,y/16))
		return;
	g_pixelOverlay[x+y*g_screenSurface->w] = color;
}


} // namespace ppg