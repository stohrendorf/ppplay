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


#include "sdltimer.h"

#include <SDL.h>
#include <boost/assert.hpp>

/**
 * @ingroup common
 * @{
 */

uint32_t SDLTimer::callback( uint32_t interval, void* userdata )
{
	SDLTimer* timer = static_cast<SDLTimer*>( userdata );
	boost::mutex::scoped_lock lock( timer->m_callbackMutex );
	timer->onTimer();
	return interval;
}

SDLTimer::SDLTimer( uint32_t interval ) : ITimer(), m_interval( interval ), m_id( nullptr ), m_callbackMutex()
{
	if( !SDL_WasInit( SDL_INIT_TIMER ) ) {
		BOOST_ASSERT( SDL_InitSubSystem( SDL_INIT_TIMER ) == 0 );
	}
	m_id = SDL_AddTimer( m_interval, callback, this );
}

SDLTimer::~SDLTimer()
{
	SDL_RemoveTimer( m_id );
	boost::mutex::scoped_lock lock( m_callbackMutex );
}

uint32_t SDLTimer::interval() const
{
	return m_interval;
}

/**
 * @}
 */
