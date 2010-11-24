/***************************************************************************
 *   Copyright (C) 2009 by Syron                                         *
 *   mr.syron@gmail.com                                                    *
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

#ifndef ppglabelH
#define ppglabelH

#include "ppgbase.h"
#include <vector>

namespace ppg {

/**
 * @class PpgLabel
 * @ingroup Ppg
 * @brief A colored text label
 */
class Label : public Widget {
	private:
		/**
		 * @brief Resizes #aFgColors and #aBgColors if the new text is longer than the arrays' sizes
		 */
		void sizeColorsToMax();
	//protected:
		std::string m_text; //!< @brief The text in this label
		std::vector<uint8_t> m_fgColors; //!< @brief Text chars' foreground colors @see PpgElement::ESC_NOCHANGE #setFgColor
		std::vector<uint8_t> m_bgColors; //!< @brief Text chars' background colors @see PpgElement::ESC_NOCHANGE #setBgColor
		virtual void drawThis() throw(Exception);
	public:
		/**
		 * @brief Label alignment enumeration class
		 */
		enum class Alignment {
			alLeft, alCenter, alRight
		};
		Alignment alignment; //!< @brief Label's alignment
		/**
		 * @brief Constructor
		 * @param[in] name Label's unique name
		 * @param[in] text Initial text
		 */
		Label(Widget*parent, const std::string &text = std::string());
		//! @copydoc PpgWidget::~PpgWidget
		virtual ~Label() throw();
		/**
		 * @brief Get the label's text length
		 * @return Text length
		 */
		virtual unsigned int length() const throw();
		/**
		 * @brief Assignment operator for std::string's
		 * @param[in] src String to assign to this label
		 * @return Reference to *this
		 */
		Label &operator=(const std::string &src) throw();
		void setText(const std::string &txt);
		char &charAt(std::size_t pos);
		char charAt(std::size_t pos) const;
		/**
		 * @brief Append-Assignment operator for std::string's
		 * @param[in] src String to append to this label
		 * @return Reference to *this
		 */
		Label &operator+=(const std::string &src) throw();
		/**
		 * @brief Char access
		 * @param[in] index Index of the char of the text to access
		 * @return Reference to the char
		 * @exception PpgException if @a index is out of range
		 */
		virtual char &operator[](const unsigned int index) throw(Exception);
		/**
		 * @brief Set's the foreground color of @a len chars from position @a pos to @a color
		 * @param[in] pos Starting position
		 * @param[in] color New foreground color
		 * @param[in] len Number of chars. Set to @c 0 to set all colors from @a pos to the end of the string
		 */
		virtual void setFgColor(unsigned int pos, unsigned char color = ESC_NOCHANGE, unsigned int len = 1) throw();
		/**
		 * @brief Set's the background color of @a len chars from position @a pos to @a color
		 * @param[in] pos Starting position
		 * @param[in] color New background color
		 * @param[in] len Number of chars. Set to @c 0 to set all colors from @a pos to the end of the string
		 */
		virtual void setBgColor(unsigned int pos, unsigned char color = ESC_NOCHANGE, unsigned int len = 1) throw();
		virtual int setHeight(const int h) throw(Exception);
		virtual int setWidth(const int w) throw(Exception);
};

} // namespace ppg

#endif
