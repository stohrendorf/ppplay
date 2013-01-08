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

#ifndef PPG_SDLSCREEN_H
#define PPG_SDLSCREEN_H

#include "widget.h"

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg
{
/**
 * @class SDLScreen
 * @brief The virtual DOS screen (SDL implementation)
 */
class PPPLAY_PPG_EXPORT SDLScreen : public Widget
{
	DISABLE_COPY( SDLScreen )
private:
	virtual void drawThis();
	int m_cursorX; //!< @brief Cursor X position
	int m_cursorY; //!< @brief Cursor Y position
public:
	static SDLScreen* instance();
	/**
	 * @brief Create a new virtual DOS screen
	 * @param[in] w Width in characters
	 * @param[in] h Height in characters
	 * @param[in] title Title of the screen
	 */
	SDLScreen( int w, int h, const std::string& title );
	virtual ~SDLScreen();
	/**
	 * @brief Clear the screen
	 * @param[in] c Character to overwrite the screen with
	 * @param[in] foreground Foreground color
	 * @param[in] background Background color
	 */
	void clear( uint8_t c, Color foreground, Color background );
	virtual void drawChar( int x, int y, char c );
	void drawPixel(int x, int y, Color c);
	void clearPixels(Color c = Color::None);
	virtual void setFgColorAt( int x, int y, Color c );
	virtual void setBgColorAt( int x, int y, Color c );
	virtual bool onMouseMove( int x, int y );
	/**
	 * @brief Whether the screen has the mouse focus
	 * @retval true if the screen has the mouse focus
	 */
	bool hasMouseFocus() const;
	
};
} // namespace ppg

/**
 * @}
 */

#endif
