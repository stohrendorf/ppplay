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

#include <SDL.h>
#include <SDL_endian.h>

#include <string>
#include <memory>
#include <vector>

#include "ppgbase.h"
#include "ppglabel.h"

/**
 * @ingroup Ppg
 * @brief Dos Color Values
 */
extern Uint32 dosColors[16];

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
class PpgStereoPeakBar : public PpgLabel {
	protected:
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
		PpgStereoPeakBar(const std::string &name, int width, int max, int interLen, bool showPeak) throw(PpgException);
		/**
		 * @brief Copy constructor
		 * @param[in] src Source to copy from
		 */
		PpgStereoPeakBar(const PpgStereoPeakBar &src) throw(PpgException);
		/**
		 * @brief Copy operator
		 * @param[in] src Source to copy from
		 * @return Reference to *this
		 */
		PpgStereoPeakBar &operator=(const PpgStereoPeakBar &src) throw();
		/**
		 * @brief Destructor
		 */
		virtual ~PpgStereoPeakBar() throw();
		/**
		 * @brief Shift values into the interpolation array
		 * @param[in] lval Left value
		 * @param[in] rval Right value
		 */
		void shift(int lval, int rval) throw(PpgException);
		/**
		 * @brief Shift fractional values into the interpolation array
		 * @param[in] lval Left value
		 * @param[in] rval Right value
		 * @pre @code (0<=lval<=1)&&(0<=rval<=1) @endcode
		 */
		void shiftFrac(float lval, float rval) throw(PpgException);
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

/**
 * @class PpgScreen
 * @ingroup Ppg
 * @brief The virtual DOS screen
 */
class PpgScreen : public PpgWidget {
	protected:
		std::shared_ptr< std::vector<unsigned char> > m_pixelOverlay; //!< @brief Pixel overlay buffer
		int m_pixW; //!< @brief Pixel overlay buffer width
		int m_pixH; //!< @brief Pixel overlay buffer height
		/**
		 * @brief Draw an 8x8 char
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] c Char to drawBgColor
		 * @param[in] foreground Foreground color
		 * @param[in] background Background color
		 * @param[in] opaque Set to @c false to draw a transparent char
		 */
		void drawChar8(int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque = true) throw() __attribute__((hot));
		/**
		 * @copydoc PpgScreen::drawChar8
		 * @brief Draw an 8x16 char
		 */
		void drawChar16(int x, int y, uint8_t c, Uint32 foreground, Uint32 background, bool opaque = true) throw() __attribute__((hot));
		virtual void drawThis() throw(PpgException);
	public:
		/**
		 * @brief Create a new virtual DOS screen
		 * @param[in] w Width in characters
		 * @param[in] h Height in characters
		 * @param[in] title Title of the screen
		 */
		PpgScreen(const int w, const int h, const std::string& title) throw(PpgException);
		/**
		 * @brief Copy constructor
		 * @param[in] src Source to copy from
		 */
		PpgScreen(const PpgScreen &src) throw(PpgException);
		virtual ~PpgScreen() throw();
		/**
		 * @brief Assignment operator
		 * @param[in] src Source to copy from
		 * @return Reference to *this
		 */
		virtual PpgScreen &operator=(const PpgScreen &src) throw();
		/**
		 * @brief Clear the screen
		 * @param[in] c Character to overwrite the screen with
		 * @param[in] foreground Foreground color
		 * @param[in] background Background color
		 */
		void clear(uint8_t c, uint8_t foreground, uint8_t background ) throw();
		virtual void drawChar(const int x, const int y, const char c) throw();
		virtual void drawFgColor(const int x, const int y, const unsigned char c) throw();
		virtual void drawBgColor(const int x, const int y, const unsigned char c) throw();
		/**
		 * @brief Draw a pixel
		 * @param[in] x Left position
		 * @param[in] y Top position
		 * @param[in] c Dos color code of the pixel
		 */
		inline void drawPixel(const int x, const int y, const unsigned char c) const throw();
		/**
		 * @brief Clear the pixel overlay (make it fully transparent)
		 */
		void clearOverlay();
};

inline void PpgScreen::drawPixel(const int x, const int y, const unsigned char c) const throw() {
	if((x < 0) || (y < 0) || (y >= m_pixH) || (x >= m_pixW))
		return;
	m_pixelOverlay->at(y*m_pixW + x) = c;
}

extern template PpgStereoPeakBar *PpgWidget::getByPath(const std::string &path) throw();
extern template PpgScreen *PpgWidget::getByPath(const std::string &path) throw();

#endif // ppgH
