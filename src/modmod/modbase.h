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

#ifndef MODBASE_H
#define MODBASE_H

#include <array>
#include <cstdint>

/**
 * @ingroup modmod
 * @{
 */

namespace ppp
{
namespace mod
{
/**
 * @brief Contains all periods of the notes
 * @details
 * The first index is the finetune value. It is unsigned, as seen in
 * the raw module data.@n
 * The second index is the note index.
 *
 * Example:
 * @code
 * uint16_t period = fullPeriods[m_finetune][m_noteIndex];
 * @endcode
 * @note Though non-standard, also octaves 0 and 4 are included
 */
extern std::array<std::array<uint16_t, 12 * 5>, 16> fullPeriods;
extern uint16_t noteIndexToPeriod( uint8_t index );
extern uint8_t periodToNoteIndex( uint16_t period );
extern uint16_t findPeriod( uint16_t period, uint8_t finetune );
}
}

/**
 * @}
 */

#endif
