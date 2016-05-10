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

#include "label.h"

#include <algorithm>

namespace ppg
{
Label::Label(Widget* parent, const std::string& text) :
    Widget(parent),
    m_text(text),
    m_fgColors(),
    m_bgColors(),
    alignment(Alignment::Left)
{
    if(length() != 0)
    {
        Label::setWidth(length());
    }
    else
    {
        Label::setWidth(1);
    }
    Widget::setHeight(1);
}

Label::~Label() = default;

void Label::sizeColorsToMax()
{
    LockGuard guard(this);
    int w = area().width();
    m_fgColors.resize(std::max<size_t>(m_text.length(), w), Color::None);
    m_bgColors.resize(std::max<size_t>(m_text.length(), w), Color::None);
}

void Label::setText(const std::string& txt)
{
    LockGuard guard(this);
    m_text = txt;
    sizeColorsToMax();
}

namespace
{
/**
 * @brief Extract a color string
 * @param[in] str The string to extract the color string from
 * @param[in] start The start within @a str to search for the color string
 * @return The "{embraced}" string or an empty string if nothing found
 */
std::string getColorString(const std::string& str, size_t start)
{
    if(start >= str.length() || str[start] != '{')
    {
        return std::string();
    }
    size_t end = str.find('}', start);
    if(end == std::string::npos)
    {
        // treat as normal text
        return std::string();
    }
    return str.substr(start, end - start + 1);
}

/**
 * @brief Helper macro to return color on string equality
 * @param[in] colname The color name
 */
#define COLRET(colname) \
    if(str == #colname) return Color::colname;
 /**
  * @brief Converts a color name to its value
  * @param[in] str The color name (like "Aqua", "Black", etc.)
  * @return The color value
  */
Color stringToColor(const std::string& str)
{
    if(str.empty())
    {
        return Color::None;
    }
    COLRET(Black) COLRET(Blue) COLRET(Green) COLRET(Aqua) COLRET(Red)
        COLRET(Purple) COLRET(Brown) COLRET(White) COLRET(Gray)
        COLRET(LightBlue) COLRET(LightGreen) COLRET(LightAqua) COLRET(LightRed)
        COLRET(LightPurple) COLRET(Yellow) COLRET(BrightWhite) COLRET(None)
        BOOST_THROW_EXCEPTION(std::invalid_argument("Invalid color string: " + str));
}
#undef COLRET

/**
 * @brief Extracts the foreground color value from a color string
 * @param[in] str The string to convert
 * @return E.g.: Color::Black if str=="{Black;Blue}"
 */
Color extractFgColor(const std::string& str)
{
    if(str.size() < 3 || str.front() != '{' || str.back() != '}')
    {
        return Color::None;
    }
    size_t pos = str.find(';');
    if(pos == std::string::npos)
    {
        return Color::None;
    }
    return stringToColor(str.substr(1, pos - 1));
}

/**
 * @brief Extracts the background color value from a color string
 * @param[in] str The string to convert
 * @return E.g.: Color::Blue if str=="{Black;Blue}"
 */
Color extractBgColor(const std::string& str)
{
    if(str.size() < 3 || str.front() != '{' || str.back() != '}')
    {
        return Color::None;
    }
    size_t pos = str.find(';');
    if(pos == std::string::npos)
    {
        return Color::None;
    }
    return stringToColor(str.substr(pos + 1, str.length() - pos - 2));
}
} // anonymous namespace

void Label::setEscapedText(const std::string& txt)
{
    LockGuard guard(this);
    Color currentFg = Color::None;
    Color currentBg = Color::None;
    std::string stripped;
    std::vector<Color> fgc, bgc;
    for(size_t i = 0; i < txt.length(); i++)
    {
        if(txt[i] == '{')
        {
            std::string colStr = getColorString(txt, i);
            if(!colStr.empty())
            {
                currentFg = extractFgColor(colStr);
                currentBg = extractBgColor(colStr);
                i += colStr.length() - 1;
                continue;
            }
        }
        stripped.push_back(txt[i]);
        fgc.push_back(currentFg);
        bgc.push_back(currentBg);
    }
    m_text = stripped;
    m_fgColors = fgc;
    m_bgColors = bgc;
    sizeColorsToMax();
}

int Label::setHeight(int /*h*/)
{
    return 1;
}

int Label::setWidth(int w)
{
    LockGuard guard(this);
    Widget::setWidth(w);
    sizeColorsToMax();
    return area().width();
}

size_t Label::length() const
{
    LockGuard guard(this);
    return m_text.length();
}

void Label::setFgColorRange(size_t pos, Color color, size_t len)
{
    LockGuard guard(this);
    if(pos >= m_bgColors.size())
        return;
    if((len == 0) || (len + pos > m_bgColors.size()))
        len = m_bgColors.size() - pos;
    std::fill_n(&m_fgColors.at(pos), len, color);
}

void Label::setBgColorRange(size_t pos, Color color, size_t len)
{
    LockGuard guard(this);
    if(pos >= m_bgColors.size())
        return;
    if((len == 0) || (len + pos > m_bgColors.size()))
        len = m_bgColors.size() - pos;
    std::fill_n(&m_bgColors.at(pos), len, color);
}

void Label::drawThis()
{
    LockGuard guard(this);
    if(length() == 0)
    {
        return;
    }
    int offset;
    switch(alignment)
    {
        case Alignment::Left:
            offset = 0;
            break;
        case Alignment::Right:
            offset = area().width() - length();
            break;
        case Alignment::Center:
            offset = (area().width() - length()) / 2;
            break;
        default:
            BOOST_THROW_EXCEPTION(std::invalid_argument("Invalid alignment"));
    }
    int w = area().width();
    for(int localX = offset; localX < w; localX++)
    {
        size_t textPos = localX - offset;
        if(textPos >= length())
        {
            break;
        }
        drawChar(localX, 0, m_text[textPos]);
        if(m_fgColors[textPos] != Color::None)
        {
            setFgColorAt(localX, 0, m_fgColors[textPos]);
        }
        if(m_bgColors[textPos] != Color::None)
        {
            setBgColorAt(localX, 0, m_bgColors[textPos]);
        }
    }
}

char& Label::charAt(size_t pos)
{
    LockGuard guard(this);
    return m_text.at(pos);
}

char Label::charAt(size_t pos) const
{
    LockGuard guard(this);
    return m_text.at(pos);
}

std::string Label::text() const
{
    LockGuard guard(this);
    return m_text;
}
} // namespace ppg