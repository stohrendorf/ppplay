/*
    PPPlay - an old-fashioned module player
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

#ifndef PPPLAY_SDLTIMER_H
#define PPPLAY_SDLTIMER_H

#include "itimer.h"

#include <mutex>

/**
 * @ingroup common
 * @{
 */

/**
 * @class SDLTimer
 * @brief ITimer specialization for SDL
 */
class SDLTimer
    : public ITimer
{
private:
    /**
     * @brief Timer interval for this timer
     */
    uint32_t m_interval;
    /**
     * @brief The internal SDL Timer ID
     */
    int m_id;
    std::mutex m_callbackMutex;

    /**
     * @brief SDL Timer callback handler
     * @param[in] interval Timer interval
     * @param[in] userdata Pointer to the owning SDLTimer
     * @return @a interval
     * @details
     * Calls ITimer::onTimer()
     */
    static uint32_t callback(uint32_t interval, void* userdata);

public:
    DISABLE_COPY(SDLTimer)

    /**
     * @brief Constructor
     * @param[in] interval Desired timer interval in milliseconds
     */
    explicit SDLTimer(uint32_t interval);

    ~SDLTimer() override;

    uint32_t interval() const override;
};

/**
 * @}
 */

#endif // SDLTIMER_H
