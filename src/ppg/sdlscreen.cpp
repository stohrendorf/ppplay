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

#include <boost/exception/all.hpp>

#include "sdlscreen.h"
#include "fonts.h"

#include "light4cxx/logger.h"

#include <SDL.h>

#include <boost/format.hpp>

namespace
{
light4cxx::Logger* logger = light4cxx::Logger::get("ppg.sdl");
}

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg
{
namespace
{
/**
 * @struct InstanceData
 * @brief Internal data and functions
 * @details
 * The ABC and visibleABC arrays are compared element-wise everytime the screen needs to be redrawn
 * to determine which parts have to be updated. If at least one of the foreground colors, background
 * colors or characters differ, it is redrawn, otherwise it is skipped. See ppg::SDLScreen::drawThis().
 *
 * This reduces the graphical overhead significantly.
 *
 * Please note that the layers are constantly locked.
 */
struct InstanceData final
{
private:
    DISABLE_COPY(InstanceData)
        /**
         * @brief Maps DOS color values to their on-screen representation
         * @see ppg::Color
         */
        Uint32 dosColors[17]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    inline Uint32 mapColor(ppg::Color c) const
    {
        return dosColors[static_cast<int>(c)];
    }

    struct Char
    {
        char chr = ' ';
        Color foreground = Color::None;
        Color background = Color::None;

        Char() = default;

        Char(char c, Color fg, Color bg)
            : chr{ c }
            , foreground{ fg }
            , background{ bg }
        {
        }

        bool operator!=(const Char& rhs) const noexcept
        {
            return chr != rhs.chr || foreground != rhs.foreground || background != rhs.background;
        }
    };

    struct CharCell
    {
        Char chr{ ' ', Color::White, Color::Black };
        Char visible{ ' ', Color::White, Color::Black };

        bool isDirty() const noexcept
        {
            return chr != visible;
        }
    };

    std::vector<CharCell> chars{};

public:
    /**
     * @brief The current SDL screen surface instance
     */
    SDL_Window* mainWindow = nullptr;

private:
    SDL_Renderer* mainRenderer = nullptr;

    SDL_Texture* backgroundLayer = nullptr;
    SDL_Texture* pixelLayer = nullptr;
    SDL_Texture* foregroundLayer = nullptr;

public:
    SDLScreen* screen = nullptr;

private:
    size_t charWidth = 0;
    size_t charHeight = 0;

    int windowWidth = 0;
    int windowHeight = 0;

public:
    InstanceData() = default;

    ~InstanceData()
    {
        reset();
    }

    void setChar(int x, int y, char c)
    {
        chars[x + y * charWidth].chr.chr = c;
    }

    void setFgColor(int x, int y, Color c)
    {
        chars[x + y * charWidth].chr.foreground = c;
    }

    void setBgColor(int x, int y, Color c)
    {
        chars[x + y * charWidth].chr.background = c;
    }

    bool isValid() const
    {
        return !chars.empty();
    }

    inline void reset()
    {
        if(chars.empty())
        {
            return;
        }

        chars.clear();

        SDL_DestroyTexture(backgroundLayer);
        backgroundLayer = nullptr;
        SDL_DestroyTexture(pixelLayer);
        pixelLayer = nullptr;
        SDL_DestroyTexture(foregroundLayer);
        foregroundLayer = nullptr;

        SDL_DestroyRenderer(mainRenderer);
        mainRenderer = nullptr;
        SDL_DestroyWindow(mainWindow);
        mainWindow = nullptr;
    }

    bool contains(int x, int y) const
    {
        return (x >= 0) && (y >= 0) && (y < windowHeight) && (x < windowWidth);
    }

private:
    int pixelLockPitch = -1;
    Uint32* pixelLockPixels = nullptr;

    /**
     * @brief Draw a pixel
     * @param[in,out] surface Surface
     * @param[in] x X position
     * @param[in] y Y position
     * @param[in] color %Screen color value
     */
    inline bool setPixel(SDL_Texture* texture, int x, int y, Uint32 color)
    {
        if(texture != nullptr && contains(x, y))
        {
            void* pixels;
            int pitch;
            if(SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to lock texture"));
            setPixel(x, y, color, static_cast<Uint32*>(pixels), pitch);
            SDL_UnlockTexture(texture);
            return true;
        }
        return false;
    }

    inline void setPixel(int x, int y, Uint32 color, Uint32* pixels, int pitch)
    {
        pixels[y * pitch / sizeof(Uint32) + x] = color;
    }

public:
    void clearPixels(Color c = Color::None)
    {
        clearTexture(pixelLayer, c);
    }

    void setPixel(int x, int y, Color color)
    {
        if(!contains(x, y))
            return;

        if(pixelLockPitch < 0 || pixelLockPixels == nullptr)
            BOOST_THROW_EXCEPTION(std::runtime_error("Pixel data must be locked before updating"));

        setPixel(x, y, mapColor(color), static_cast<Uint32*>(pixelLockPixels), pixelLockPitch);
    }

    void lockPixels()
    {
        if(SDL_LockTexture(pixelLayer, nullptr, reinterpret_cast<void**>(&pixelLockPixels), &pixelLockPitch) != 0)
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to lock pixel layer"));
    }

    void unlockPixels()
    {
        SDL_UnlockTexture(pixelLayer);
        pixelLockPixels = nullptr;
        pixelLockPitch = -1;
    }

private:
    void clearTexture(SDL_Texture* texture, Color c) const
    {
        Uint32 fmt;
        int access, w, h;
        SDL_QueryTexture(texture, &fmt, &access, &w, &h);

        void* pixels;
        int pitch;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        size_t count = h * pitch / sizeof(Uint32);
        Uint32 color = mapColor(c);
        std::fill_n(reinterpret_cast<Uint32*>(pixels), count, color);
        SDL_UnlockTexture(texture);
    }

public:
    bool init(int charWidth, int charHeight, const std::string& title);

private:
    /**
     * @brief Draw an 8x16 char
     * @param[in] x Left position
     * @param[in] y Top position
     * @param[in] c Char to draw
     * @param[in] foreground Foreground color
     * @param[in] background Background color
     * @param[in] opaque Set to @c false to draw a transparent char
     */
    inline void drawChar(int x, int y, char c, Uint32 foreground, Uint32 background, Uint32* fgPixels, int fgPitch, Uint32* bgPixels, int bgPitch)
    {
        x <<= 3;
        y <<= 4;
        for(int py = 0; py < 16; py++)
        {
            for(int px = 0; px < 8; px++)
            {
                if(plFont816[uint8_t(c)][py] & (0x80 >> px))
                {
                    setPixel(x + px, y + py, foreground, fgPixels, fgPitch);
                }
                else
                {
                    setPixel(x + px, y + py, mapColor(Color::None), fgPixels, fgPitch);
                }

                setPixel(x + px, y + py, background, bgPixels, bgPitch);
            }
        }
    }

public:
    void clear(char c, ppg::Color foreground, ppg::Color background);
    void redraw(bool showMouse, int cursorX, int cursorY);
};

bool InstanceData::init(int charWidth, int charHeight, const std::string& title)
{
    if(mainWindow != nullptr)
    {
        return false;
    }
    if(!SDL_WasInit(SDL_INIT_VIDEO))
    {
        if(SDL_Init(SDL_INIT_VIDEO) == -1)
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Initialization of SDL Video surface failed"));
        }
    }
    windowWidth = charWidth * 8;
    windowHeight = charHeight * 16;
    this->charWidth = charWidth;
    this->charHeight = charHeight;

    if(SDL_CreateWindowAndRenderer(windowWidth, windowHeight, 0, &mainWindow, &mainRenderer) != 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Screen Initialization failed"));
    }
    SDL_SetWindowTitle(mainWindow, title.c_str());
    if(!mainWindow)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Window Initialization failed"));
    }
    if(!mainRenderer)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Renderer Initialization failed"));
    }

    if(const char* videoDrv = SDL_GetCurrentVideoDriver())
    {
        logger->info(L4CXX_LOCATION, "Using video driver '%s'", videoDrv);
    }

    backgroundLayer = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight);
    clearTexture(backgroundLayer, Color::Black);
    SDL_SetTextureBlendMode(backgroundLayer, SDL_BLENDMODE_NONE);
    pixelLayer = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight);
    clearTexture(pixelLayer, Color::None);
    SDL_SetTextureBlendMode(pixelLayer, SDL_BLENDMODE_BLEND);
    foregroundLayer = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight);
    clearTexture(foregroundLayer, Color::Black);
    SDL_SetTextureBlendMode(foregroundLayer, SDL_BLENDMODE_BLEND);

    dosColors[static_cast<int>(Color::None)] = 0x00000000; // transparent
    dosColors[static_cast<int>(Color::Black)] = 0x000000ff; // black
    dosColors[static_cast<int>(Color::Blue)] = 0x0000aaff; // blue
    dosColors[static_cast<int>(Color::Green)] = 0x00aa00ff; // green
    dosColors[static_cast<int>(Color::Aqua)] = 0x00aaaaff; // aqua
    dosColors[static_cast<int>(Color::Red)] = 0xaa0000ff; // red
    dosColors[static_cast<int>(Color::Purple)] = 0xaa00aaff; // purple
    dosColors[static_cast<int>(Color::Brown)] = 0xaa5500ff; // brown
    dosColors[static_cast<int>(Color::White)] = 0xaaaaaaff; // white
    dosColors[static_cast<int>(Color::Gray)] = 0x555555ff; // gray
    dosColors[static_cast<int>(Color::LightBlue)] = 0x5555ffff; // light blue
    dosColors[static_cast<int>(Color::LightGreen)] = 0x55ff55ff; // light green
    dosColors[static_cast<int>(Color::LightAqua)] = 0x55ffffff; // light aqua
    dosColors[static_cast<int>(Color::LightRed)] = 0xff5555ff; // light red (orange?)
    dosColors[static_cast<int>(Color::LightPurple)] = 0xff55ffff; // light purple
    dosColors[static_cast<int>(Color::Yellow)] = 0xffff55ff; // yellow
    dosColors[static_cast<int>(Color::BrightWhite)] = 0xffffffff; // bright white

    chars.resize(charWidth * charHeight);

    return true;
}

void InstanceData::clear(char c, ppg::Color foreground, ppg::Color background)
{
    for(CharCell& cell : chars)
    {
        cell.chr.chr = c;
        cell.chr.foreground = foreground;
        cell.chr.background = background;
    }

    clearPixels();
}

void InstanceData::redraw(bool showMouse, int cursorX, int cursorY)
{
    void* fgPixels;
    void* bgPixels;
    int fgPitch;
    int bgPitch;
    if(SDL_LockTexture(foregroundLayer, nullptr, &fgPixels, &fgPitch) != 0)
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to lock foreground texture"));
    if(SDL_LockTexture(backgroundLayer, nullptr, &bgPixels, &bgPitch) != 0)
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to lock background texture"));

    {
        // redraw the screen characters if changed
        size_t ofs = 0;
        for(size_t y = 0; y < charHeight; y++)
        {
            for(size_t x = 0; x < charWidth; x++, ofs++)
            {
                if(chars[ofs].isDirty())
                {
                    drawChar(x, y, chars[ofs].chr.chr, mapColor(chars[ofs].chr.foreground), mapColor(chars[ofs].chr.background), static_cast<Uint32*>(fgPixels), fgPitch, static_cast<Uint32*>(bgPixels), bgPitch);
                    chars[ofs].visible = chars[ofs].chr;
                }
            }
        }
    }

    // show the mouse cursor if applicable
    if(showMouse && contains(cursorX, cursorY))
    {
        size_t ofs = cursorX + cursorY * charWidth;
        Uint32 c1 = mapColor(~chars[ofs].visible.foreground);
        Uint32 c2 = mapColor(~chars[ofs].visible.background);
        drawChar(cursorX, cursorY, chars[ofs].chr.chr, c1, c2, static_cast<Uint32*>(fgPixels), fgPitch, static_cast<Uint32*>(bgPixels), bgPitch);
        chars[ofs].visible.foreground = ~chars[ofs].chr.foreground;
        chars[ofs].visible.background = ~chars[ofs].chr.background;
    }

    SDL_UnlockTexture(foregroundLayer);
    SDL_UnlockTexture(backgroundLayer);

    // now blit the background, pixels, and foreground (in that order) to the screen
    SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(mainRenderer);

    SDL_RenderCopy(mainRenderer, backgroundLayer, nullptr, nullptr);
    SDL_RenderCopy(mainRenderer, pixelLayer, nullptr, nullptr);
    SDL_RenderCopy(mainRenderer, foregroundLayer, nullptr, nullptr);

    SDL_RenderPresent(mainRenderer);
}

InstanceData instanceData;
} // anonymous namespace

SDLScreen* SDLScreen::instance()
{
    return instanceData.screen;
}

SDLScreen::SDLScreen(int w, int h, const std::string& title) : Widget(nullptr), SDLTimer(1000 / 30), m_cursorX(0), m_cursorY(0)
{
    if(!instanceData.init(w, h, title))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("SDL Screen Surface already aquired"));
    }
    Widget::setPosition(0, 0);
    Widget::setSize(w, h);
    SDL_ShowCursor(0);
    instanceData.screen = this;
}

SDLScreen::~SDLScreen()
{
    LockGuard guard(this);
    instanceData.screen = nullptr;
    instanceData.reset();
}

void SDLScreen::clear(uint8_t c, Color foreground, Color background)
{
    LockGuard guard(this);
    instanceData.clear(c, foreground, background);
}

void SDLScreen::drawThis()
{
    LockGuard guard(this);
    instanceData.redraw(hasMouseFocus(), m_cursorX, m_cursorY);
}

void SDLScreen::drawChar(int x, int y, char c)
{
    LockGuard guard(this);
    if(!instanceData.contains(x, y))
    {
        logger->error(L4CXX_LOCATION, "Out of range: %d, %d", x, y);
        return;
    }
    instanceData.setChar(x, y, c);
}

void SDLScreen::setFgColorAt(int x, int y, Color c)
{
    LockGuard guard(this);
    if(!instanceData.contains(x, y))
    {
        return;
    }
    instanceData.setFgColor(x, y, c);
}

void SDLScreen::setBgColorAt(int x, int y, Color c)
{
    LockGuard guard(this);
    if(!instanceData.contains(x, y))
    {
        return;
    }
    instanceData.setBgColor(x, y, c);
}

bool SDLScreen::onMouseMove(int x, int y)
{
    m_cursorX = x;
    m_cursorY = y;
    Widget::onMouseMove(x, y);
    return true;
}

bool SDLScreen::hasMouseFocus() const
{
    LockGuard guard(this);
    return (SDL_GetWindowFlags(instanceData.mainWindow)&SDL_WINDOW_MOUSE_FOCUS) != 0;
}

void SDLScreen::drawPixel(int x, int y, Color c)
{
    LockGuard guard(this);
    instanceData.setPixel(x, y, c);
}

void SDLScreen::lockPixels()
{
    LockGuard guard(this);
    instanceData.lockPixels();
}

void SDLScreen::unlockPixels()
{
    LockGuard guard(this);
    instanceData.unlockPixels();
}

void SDLScreen::clearPixels(Color c)
{
    LockGuard guard(this);
    instanceData.clearPixels(c);
}

void SDLScreen::onTimer()
{
    LockGuard guard(this);
    // chars will be the first deleted, visibleColorsB will be last initialized
    if(!instanceData.isValid())
        return;
    clear(' ', ppg::Color::White, ppg::Color::Black);
    clearPixels();
    draw();
}
} // namespace ppg

/**
 * @}
 */