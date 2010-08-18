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

PpgLabel::PpgLabel(const std::string &name, const std::string &text) throw() :
		PpgWidget(name),
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

PpgLabel::~PpgLabel() throw() {
}

void PpgLabel::sizeColorsToMax() {
	m_fgColors.resize(std::max<std::size_t>(m_text.length(), getWidth()), ESC_NOCHANGE);
	m_bgColors.resize(std::max<std::size_t>(m_text.length(), getWidth()), ESC_NOCHANGE);
}


PpgLabel &PpgLabel::operator=(const PpgLabel & src) throw() {
	PpgWidget::operator=(src);
	m_text = src.m_text;
	alignment = src.alignment;
	m_fgColors = src.m_fgColors;
	m_bgColors = src.m_bgColors;
	setWidth(src.getWidth());
	return *this;
}

PpgLabel &PpgLabel::operator=(const std::string & ass) throw() {
	m_text = ass;
	sizeColorsToMax();
	return *this;
}

PpgLabel &PpgLabel::operator+=(const std::string & src) throw() {
	m_text += src;
	sizeColorsToMax();
	return *this;
}

PpgLabel::PpgLabel(const PpgLabel &src) throw() :
		PpgWidget(src.m_name), m_text(src.m_text), m_fgColors(src.m_fgColors), m_bgColors(src.m_bgColors), alignment(src.alignment) {
	isFinalNode();
	m_bottom = src.m_bottom;
	m_top = src.m_top;
	m_left = src.m_left;
	m_right = src.m_right;
}

int PpgLabel::setHeight(const int /*h*/) throw(PpgException) {
	return 1;
}

int PpgLabel::setWidth(const int w) throw(PpgException) {
	PpgWidget::setWidth(w);
	sizeColorsToMax();
	return getWidth();
}

unsigned int PpgLabel::length() const throw() {
	return m_text.length();
}

char &PpgLabel::operator[](const unsigned int index) throw(PpgException) {
	PPG_TEST(index>=m_text.length());
	return m_text[index];
}

void PpgLabel::setFgColor(unsigned int pos, const unsigned char color, unsigned int len) throw() {
	if (pos >= m_bgColors.size())
		return;
	if ((len == 0) || (len + pos > m_bgColors.size()))
		len = m_bgColors.size() - pos;
	std::fill_n(&m_fgColors[pos], len, color);
}

void PpgLabel::setBgColor(unsigned int pos, const unsigned char color, unsigned int len) throw() {
	if (pos >= m_bgColors.size())
		return;
	if ((len == 0) || (len + pos > m_bgColors.size()))
		len = m_bgColors.size() - pos;
	std::fill_n(&m_bgColors[pos], len, color);
}

void PpgLabel::drawThis() throw(PpgException) {
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

template PpgLabel *PpgWidget::getByPath(const std::string &path) throw();
