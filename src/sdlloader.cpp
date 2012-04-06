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

#include <SDL.h>
#include <boost/assert.hpp>

void __attribute__(( constructor )) sdlInit()
{
	// SDL wiki says that the eventthread is only running when video is initialized, so...
	BOOST_ASSERT( SDL_Init( SDL_INIT_EVENTTHREAD|SDL_INIT_VIDEO ) == 0 );
	// set the keyboard repetition
	BOOST_ASSERT( SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL ) == 0 );
	BOOST_ASSERT( atexit( SDL_Quit ) == 0 );
}
