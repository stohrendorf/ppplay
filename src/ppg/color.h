/*
    PPPlay - an old-fashioned module player
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

#ifndef PPG_PPG_H
#define PPG_PPG_H

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg
{

/**
 * @brief Enumeration values for Dos Color Mappings
 */
enum class Color : unsigned char
{
    Black, Blue, Green, Aqua, Red, Purple, Brown, White,
    Gray, LightBlue, LightGreen, LightAqua, LightRed, LightPurple, Yellow, BrightWhite,
    None
};

/**
 * @brief Invert a color
 * @param[in] c Color to invert
 * @return Inverted color
 * @note This is DOS's default algorithm
 */
inline constexpr Color operator~( Color c ) noexcept {
    return static_cast<Color>( ( static_cast<int>( c ) & 7 ) ^ 7 );
}
} // namespace ppg

/**
 * @}
 */

#endif // ppgH
