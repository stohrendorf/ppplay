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

#ifndef PPG_STEREOPEAKBAR_H
#define PPG_STEREOPEAKBAR_H

#include "label.h"

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg
{
/**
 * @class StereoPeakBar
 * @brief A stereo bar with peaks
 */
class PPPLAY_PPG_EXPORT StereoPeakBar : public Label
{
private:
	std::vector<int> m_interArrL; //!< @brief Left bar interpolation array
	std::vector<int> m_interArrR; //!< @brief Right bar interpolation array
	int m_max; //!< @brief Maximum input values @see shift
	int m_barLength; //!< @brief Length of a half bar
	bool m_showPeak; //!< @brief If @c true, the bar shows peak values
	int m_peakPosL; //!< @brief Left peak value
	int m_peakPosR; //!< @brief Right peak value
	int m_peakFalloffSpeedL; //!< @brief Current left peak value falloff speed
	int m_peakFalloffSpeedR; //!< @brief Current right peak value falloff speed
public:
	/**
	 * @brief Constructor
	 * @param[in] parent Parent widget
	 * @param[in] width Width of each bar
	 * @param[in] max Maximum input value
	 * @param[in] interLen Interpolation length
	 * @param[in] showPeak Set to @c true to show the peak values
	 * @pre @c width>=8
	 * @pre @c interLen>=1
	 */
	StereoPeakBar( Widget* parent, int width, int max, int interLen, bool showPeak );
	/**
	 * @brief Destructor
	 */
	virtual ~StereoPeakBar();
	/**
	 * @brief Shift values into the interpolation array
	 * @param[in] lval Left value
	 * @param[in] rval Right value
	 */
	void shift( int lval, int rval );
	/**
	 * @brief Shift fractional values into the interpolation array
	 * @param[in] lval Left value
	 * @param[in] rval Right value
	 * @pre @code (0<=lval<=1)&&(0<=rval<=1) @endcode
	 */
	void shiftFrac( float lval, float rval );
	/**
	 * @brief Get the left value
	 * @return The left value
	 */
	int valueLeft() const;
	/**
	 * @brief Get the right value
	 * @return The right value
	 */
	int valueRight() const;
};

}

/**
 * @}
 */

#endif
