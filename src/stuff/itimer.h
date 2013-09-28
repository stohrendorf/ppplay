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


#ifndef PPPLAY_ITIMER_H
#define PPPLAY_ITIMER_H

#include "utils.h"

#include <cstdint>

/**
 * @ingroup Common
 * @{
 */

/**
 * @interface ITimer
 * @brief Timer interface
 */
class ITimer
{
    DISABLE_COPY( ITimer )
public:
    /**
     * @brief Default constructor
     */
    ITimer() = default;
    /**
     * @brief Virtual default destructor
     */
    inline virtual ~ITimer();
    /**
     * @brief The timer interval in milliseconds
     * @return The timer interval in milliseconds
     */
    virtual uint32_t interval() const = 0;
    /**
     * @brief Timer handler, called every interval() milliseconds
     */
    virtual void onTimer() = 0;
};

inline ITimer::~ITimer() = default;

/**
 * @}
 */

#endif // TIMER_H
