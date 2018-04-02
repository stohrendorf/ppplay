#pragma once

/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include <genmod/ppplay_module_base_export.h>

#include <stream/iserializable.h>

#include <cstdint>
#include <cstdlib>

namespace ppp
{
    /**
     * @ingroup GenMod
     * @{
     */

    /**
     * @struct ModuleState
     * @brief Contains information about the a playback state
     */
    struct PPPLAY_MODULE_BASE_EXPORT ModuleState : public ISerializable
    {
        explicit ModuleState() = default;

        AbstractArchive& serialize(AbstractArchive* data) override;

        //! @brief Speed
        int16_t speed = 0;
        //! @brief Tempo
        int16_t tempo = 0;
        //! @brief Order
        size_t order = 0;
        //! @brief Row
        int16_t row = 0;
        //! @brief Tick index
        int16_t tick = 0;
        //! @brief Global volume
        int16_t globalVolume = 0x40;
        int16_t globalVolumeLimit = 0x40;
        //! @brief Played Sample frames
        size_t playedFrames = 0;
        //! @brief Pattern index of order
        size_t pattern = 0;
    };

    /**
     * @}
     */
}
