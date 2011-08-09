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


#ifndef ITIMER_H
#define ITIMER_H

#include "utils.h"

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
	DISABLE_COPY(ITimer)
public:
	ITimer() = default;
	virtual ~ITimer();
	virtual uint32_t interval() const = 0;
	virtual void onTimer() = 0;
};

/**
 * @}
 */

#endif // TIMER_H