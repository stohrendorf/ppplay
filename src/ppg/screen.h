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

#ifndef SCREEN_H
#define SCREEN_H

#include "widget.h"

namespace ppg {
	/**
	 * @class Screen
	 * @ingroup Ppg
	 * @brief The virtual DOS screen
	 */
	class Screen : public Widget {
			DISABLE_COPY( Screen )
		private:
			/**
			 * @brief Draw an 8x8 char
			 * @param[in] x Left position
			 * @param[in] y Top position
			 * @param[in] c Char to draw
			 * @param[in] foreground Foreground color
			 * @param[in] background Background color
			 * @param[in] opaque Set to @c false to draw a transparent char
			 */
			void drawChar8( int x, int y, uint8_t c, uint32_t foreground, uint32_t background, bool opaque = true );
			/**
			 * @copydoc ppg::Screen::drawChar8
			 * @brief Draw an 8x16 char
			 */
			void drawChar16( int x, int y, uint8_t c, uint32_t foreground, uint32_t background, bool opaque = true );
			virtual void drawThis();
			int m_cursorX; //!< @brief Cursor X position
			int m_cursorY; //!< @brief Cursor Y position
		public:
			/**
			 * @brief Create a new virtual DOS screen
			 * @param[in] w Width in characters
			 * @param[in] h Height in characters
			 * @param[in] title Title of the screen
			 */
			Screen( int w, int h, const std::string& title );
			virtual ~Screen();
			/**
			 * @brief Clear the screen
			 * @param[in] c Character to overwrite the screen with
			 * @param[in] foreground Foreground color
			 * @param[in] background Background color
			 */
			void clear( uint8_t c, Color foreground, Color background );
			virtual void drawChar( int x, int y, char c );
			virtual void setFgColorAt( int x, int y, Color c );
			virtual void setBgColorAt( int x, int y, Color c );
			virtual bool onMouseMove( int x, int y );
	};
} // namespace ppg

#endif
