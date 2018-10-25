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

#ifndef PPG_SDLSCREEN_H
#define PPG_SDLSCREEN_H

#include "widget.h"
#include <stuff/sdltimer.h>

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
class SDLScreen final
    : public Widget, public SDLTimer
{
private:
    void drawThis() override;

    int m_cursorX; //!< @brief Cursor X position
    int m_cursorY; //!< @brief Cursor Y position

    void onTimer() override;

public:
    DISABLE_COPY(SDLScreen)

    static SDLScreen* instance();

    /**
     * @brief Create a new virtual DOS screen
     * @param[in] w Width in characters
     * @param[in] h Height in characters
     * @param[in] title Title of the screen
     */
    SDLScreen(int w, int h, const std::string& title);

    ~SDLScreen() override;

    /**
     * @brief Clear the screen
     * @param[in] c Character to overwrite the screen with
     * @param[in] foreground Foreground color
     * @param[in] background Background color
     */
    void clear(uint8_t c, Color foreground, Color background);

    void drawChar(int x, int y, char c) override;

    void drawPixel(int x, int y, Color c);

    void drawPixel(int x, int y, uint32_t c);

    static constexpr inline uint32_t fromRgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
    {
        return (uint32_t(r) << 24) | (uint32_t(g) << 16) | (uint32_t(b) << 8) | a;
    }

    void lockPixels();

    void unlockPixels();

    void clearPixels(Color c = Color::None);

    void setFgColorAt(int x, int y, Color c) override;

    void setBgColorAt(int x, int y, Color c) override;

    bool onMouseMove(int x, int y) override;

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
