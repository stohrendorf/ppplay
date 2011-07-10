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

#ifndef LABEL_H
#define LABEL_H

#include "widget.h"

#include <vector>

namespace ppg {

/**
 * @class Label
 * @ingroup Ppg
 * @brief A colored text label
 */
class Label : public Widget {
		DISABLE_COPY(Label)
	private:
		/**
		 * @brief Resizes m_fgColors and m_bgColors if the new text length or the widget's width are greater than the arrays' sizes
		 */
		void sizeColorsToMax();
		std::string m_text; //!< @brief The text in this label
		std::vector<Color> m_fgColors; //!< @brief Text chars' foreground colors
		std::vector<Color> m_bgColors; //!< @brief Text chars' background colors
		virtual void drawThis();
	public:
		/**
		 * @brief Constructor
		 * @param[in] parent Parent widget
		 * @param[in] text Initial text
		 */
		Label(Widget* parent, const std::string& text = std::string());
		/**
		 * @brief Label alignment enumeration class
		 */
		enum class Alignment {
		    Left, Center, Right
		};
		Alignment alignment; //!< @brief Label's alignment
		//! @copydoc ppg::Widget::~Widget
		virtual ~Label();
		/**
		 * @brief Get the label's text length
		 * @return Text length
		 */
		std::size_t length() const;
		/**
		 * @brief Sets the label's text
		 * @param[in] txt Text to assign to this label
		 */
		void setText(const std::string& txt);
		/**
		 * @brief Sets the label's text from an escaped string
		 * @param[in] txt Text to assign to this label
		 * @details
		 * Sets this label's string and colors from the escaped
		 * string @a txt. Colors may be inserted by using the pattern
		 * @code "{[foreground];[background]}" @endcode
		 * Examples:
		 * @li "{None;Blue}" which is equivalent to "{;Blue}"
		 * @li "{White;}" which is equivalent to "{White;None}"
		 * 
		 * For valid names, see ppg::Color
		 */
		void setEscapedText(const std::string& txt);
		/**
		 * @brief Get the label's text
		 * @return The label's text
		 */
		std::string text() const;
		/**
		 * @brief Get the character at position @a pos
		 * @param[in] pos Position of the requested character
		 * @return Reference to the character at position @a pos
		 */
		char& charAt(std::size_t pos);
		/**
		 * @overload
		 * @brief Get the character at position @a pos
		 * @param[in] pos Position of the requested character
		 * @return The character at position @a pos
		 */
		char charAt(std::size_t pos) const;
		/**
		 * @brief Set's the foreground color of @a len chars from position @a pos to @a color
		 * @param[in] pos Starting position
		 * @param[in] color New foreground color
		 * @param[in] len Number of chars. Set to @c 0 to set all colors from @a pos to the end of the string
		 */
		virtual void setFgColorRange(std::size_t pos, Color color = Color::None, std::size_t len = 1);
		/**
		 * @brief Set's the background color of @a len chars from position @a pos to @a color
		 * @param[in] pos Starting position
		 * @param[in] color New background color
		 * @param[in] len Number of chars. Set to @c 0 to set all colors from @a pos to the end of the string
		 */
		virtual void setBgColorRange(std::size_t pos, Color color = Color::None, std::size_t len = 1);
		virtual int setHeight(int h);
		virtual int setWidth(int w);
};

} // namespace ppg

#endif
