/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
 * @file
 * @ingroup Ppg
 * @brief PeePeeGUI Classes
 */

#ifndef ppgH
#define ppgH

#include <string>
#include <memory>
#include <vector>

#include "widget.h"
#include "label.h"

namespace ppg {

/**
 * @ingroup Ppg
 * @brief Enumeration values for Dos Color Mappings
 * @see dosColors
 */
enum {
	dcBlack = 0x00, dcBlue, dcGreen, dcAqua, dcRed, dcPurple, dcBrown, dcWhite,
	dcGray, dcLightBlue, dcLightGreen, dcLightAqua, dcLightRed, dcLightPurple, dcYellow, dcBrightWhite
};

/**
 * @class PpgStereoPeakBar
 * @ingroup Ppg
 * @brief A stereo bar with peaks
 */
class StereoPeakBar : public Label {
	private:
		std::vector<int> m_interArrL; //!< @brief Left bar interpolation array
		std::vector<int> m_interArrR; //!< @brief Right bar interpolation array
		//int m_interLen; //!< @brief Interpolation array size
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
		 * @param[in] name Unique name
		 * @param[in] width Width of each bar
		 * @param[in] max Maximum input value
		 * @param[in] interLen Interpolation length
		 * @param[in] showPeak Set to @c true to show the peak values
		 * @pre @c width>=8
		 * @pre @c interLen>=1
		 */
		StereoPeakBar(Widget*parent, int width, int max, int interLen, bool showPeak) throw(Exception);
		/**
		 * @brief Destructor
		 */
		virtual ~StereoPeakBar() throw();
		/**
		 * @brief Shift values into the interpolation array
		 * @param[in] lval Left value
		 * @param[in] rval Right value
		 */
		void shift(int lval, int rval) throw(Exception);
		/**
		 * @brief Shift fractional values into the interpolation array
		 * @param[in] lval Left value
		 * @param[in] rval Right value
		 * @pre @code (0<=lval<=1)&&(0<=rval<=1) @endcode
		 */
		void shiftFrac(float lval, float rval) throw(Exception);
		/**
		 * @brief Get the right value
		 * @return The right value
		 */
		int getValLeft() const throw();
		/**
		 * @brief Get the left value
		 * @return The left value
		 */
		int getValRight() const throw();
};

} // namespace ppg

#endif // ppgH
