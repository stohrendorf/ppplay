/*
    PeePeePlayer - an old-fashioned module player
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

#ifndef PPG_H
#define PPG_H

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg {

	/**
	 * @brief Enumeration values for Dos Color Mappings
	 */
	enum class Color {
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
	inline Color operator~(const Color& c) {
		return static_cast<Color>(( static_cast<int>(c) & 7 ) ^ 7);
	}
} // namespace ppg

/**
 * @}
 */

#endif // ppgH
