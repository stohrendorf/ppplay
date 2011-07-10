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


#ifndef SDLTIMER_H
#define SDLTIMER_H

#include "itimer.h"

class SDLTimer : public ITimer
{
	DISABLE_COPY(SDLTimer)
private:
	uint32_t m_interval;
	struct _SDL_TimerID* m_id;
	static uint32_t callback(uint32_t interval, void* userdata);
public:
	SDLTimer(uint32_t interval);
    virtual ~SDLTimer();
    virtual uint32_t interval() const;
};

#endif // SDLTIMER_H
