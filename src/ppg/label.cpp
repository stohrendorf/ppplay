/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#include "label.h"
#include <algorithm>

namespace ppg {

Label::Label(Widget* parent, const std::string& text) :
	Widget(parent),
	m_text(text),
	m_fgColors(),
	m_bgColors(),
	alignment(Alignment::Left) {
	if(length() != 0) {
		setWidth(length());
	}
	else {
		setWidth(1);
	}
	Widget::setHeight(1);
}

Label::~Label() = default;

void Label::sizeColorsToMax() {
	int w = area().width();
	m_fgColors.resize(std::max<std::size_t>(m_text.length(), w), ESC_NOCHANGE);
	m_bgColors.resize(std::max<std::size_t>(m_text.length(), w), ESC_NOCHANGE);
}

void Label::setText(const std::string& txt) {
	m_text = txt;
	sizeColorsToMax();
}

int Label::setHeight(int /*h*/) {
	return 1;
}

int Label::setWidth(int w) {
	Widget::setWidth(w);
	sizeColorsToMax();
	return area().width();
}

std::size_t Label::length() const {
	return m_text.length();
}

void Label::setFgColorRange(std::size_t pos, uint8_t color, std::size_t len) {
	if(pos >= m_bgColors.size())
		return;
	if((len == 0) || (len + pos > m_bgColors.size()))
		len = m_bgColors.size() - pos;
	std::fill_n(&m_fgColors[pos], len, color);
}

void Label::setBgColorRange(std::size_t pos, uint8_t color, std::size_t len) {
	if(pos >= m_bgColors.size())
		return;
	if((len == 0) || (len + pos > m_bgColors.size()))
		len = m_bgColors.size() - pos;
	std::fill_n(&m_bgColors[pos], len, color);
}

void Label::drawThis() {
	if(length() == 0)
		return;
	int offset;
	switch(alignment) {
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
			PPG_THROW("Invalid alignment");
	}
	int w = area().width();
	for(int localX = offset; localX < w; localX++) {
		std::size_t textPos = localX - offset;
		if(textPos >= length())
			break;
		drawChar(localX, 0, m_text[textPos]);
		if(m_fgColors[textPos] != ESC_NOCHANGE)
			setFgColorAt(localX, 0, m_fgColors[textPos]);
		if(m_bgColors[textPos] != ESC_NOCHANGE)
			setBgColorAt(localX, 0, m_bgColors[textPos]);
	}
}

char& Label::charAt(std::size_t pos) {
	return m_text[pos];
}
char Label::charAt(std::size_t pos) const {
	return m_text[pos];
}

std::string Label::text() const {
	return m_text;
}

} // namespace ppg
