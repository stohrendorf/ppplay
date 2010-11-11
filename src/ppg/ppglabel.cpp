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

#include "ppglabel.h"
#include <algorithm>

namespace ppg {

Label::Label(const std::string &name, const std::string &text) throw() :
		Widget(name),
		m_text(text),
		m_fgColors(),
		m_bgColors(),
		alignment(Alignment::alLeft)
{
	isFinalNode();
	if (length() != 0)
		setWidth(length());
	else
		setWidth(1);
}

Label::~Label() throw() {
}

void Label::sizeColorsToMax() {
	m_fgColors.resize(std::max<std::size_t>(m_text.length(), getWidth()), ESC_NOCHANGE);
	m_bgColors.resize(std::max<std::size_t>(m_text.length(), getWidth()), ESC_NOCHANGE);
}


Label &Label::operator=(const Label & src) throw() {
	Widget::operator=(src);
	m_text = src.m_text;
	alignment = src.alignment;
	m_fgColors = src.m_fgColors;
	m_bgColors = src.m_bgColors;
	setWidth(src.getWidth());
	return *this;
}

Label &Label::operator=(const std::string & ass) throw() {
	m_text = ass;
	sizeColorsToMax();
	return *this;
}

Label &Label::operator+=(const std::string & src) throw() {
	m_text += src;
	sizeColorsToMax();
	return *this;
}

Label::Label(const Label &src) throw() :
		Widget(src.m_name), m_text(src.m_text), m_fgColors(src.m_fgColors), m_bgColors(src.m_bgColors), alignment(src.alignment) {
	isFinalNode();
	m_bottom = src.m_bottom;
	m_top = src.m_top;
	m_left = src.m_left;
	m_right = src.m_right;
}

int Label::setHeight(const int /*h*/) throw(Exception) {
	return 1;
}

int Label::setWidth(const int w) throw(Exception) {
	Widget::setWidth(w);
	sizeColorsToMax();
	return getWidth();
}

unsigned int Label::length() const throw() {
	return m_text.length();
}

char &Label::operator[](const unsigned int index) throw(Exception) {
	PPG_TEST(index>=m_text.length());
	return m_text[index];
}

void Label::setFgColor(unsigned int pos, const unsigned char color, unsigned int len) throw() {
	if (pos >= m_bgColors.size())
		return;
	if ((len == 0) || (len + pos > m_bgColors.size()))
		len = m_bgColors.size() - pos;
	std::fill_n(&m_fgColors[pos], len, color);
}

void Label::setBgColor(unsigned int pos, const unsigned char color, unsigned int len) throw() {
	if (pos >= m_bgColors.size())
		return;
	if ((len == 0) || (len + pos > m_bgColors.size()))
		len = m_bgColors.size() - pos;
	std::fill_n(&m_bgColors[pos], len, color);
}

void Label::drawThis() throw(Exception) {
	if (!m_parent) {
		std::cerr << "PpgLabel: no parent" << std::endl;
		return;
	}
	if(length()==0)
		return;
	int offset;
	int w = getWidth();
	switch (alignment) {
		case Alignment::alLeft:
			offset = 0;
			break;
		case Alignment::alRight:
			offset = w - length();
			break;
		case Alignment::alCenter:
			offset = (w - length()) / 2;
			break;
		default:
			PPG_THROW("Invalid alignment");
	}
	for (int i = 0; i < w; i++) {
		if (i - offset < 0)
			continue;
		else if (i - offset >= static_cast<long>(length()))
			continue;
		else {
			drawChar(i, 0, m_text[i-offset]);
			if (m_fgColors[i-offset] != ESC_NOCHANGE)
				drawFgColor(i, 0, m_fgColors[i-offset]);
			if (m_bgColors[i-offset] != ESC_NOCHANGE)
				drawBgColor(i, 0, m_bgColors[i-offset]);
		}
	}
}

template Label *Widget::getByPath(const std::string &path) throw();

} // namespace ppg
