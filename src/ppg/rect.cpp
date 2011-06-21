/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "rect.h"

namespace ppg {

Rect::Rect(int x, int y, int width, int height) : m_topLeft(x, y), m_bottomRight(x + width - 1, y + height - 1) {
}

int Rect::top() const {
	return m_topLeft.y();
}

void Rect::setTop(int top) {
	m_topLeft.setY(top);
}

int Rect::left() const {
	return m_topLeft.x();
}

void Rect::setLeft(int left) {
	m_topLeft.setX(left);
}

int Rect::bottom() const {
	return m_bottomRight.y();
}

void Rect::setBottom(int bottom) {
	m_bottomRight.setY(bottom);
}

int Rect::right() const {
	return m_bottomRight.x();
}

void Rect::setRight(int right) {
	m_bottomRight.setX(right);
}

int Rect::width() const {
	return m_bottomRight.x() - m_topLeft.x() + 1;
}

void Rect::setWidth(int width) {
	m_bottomRight.setX(m_topLeft.x() + width - 1);
}

int Rect::height() const {
	return m_bottomRight.y() - m_topLeft.y() + 1;
}

void Rect::setHeight(int height) {
	m_bottomRight.setY(m_topLeft.y() + height - 1);
}

Point Rect::topLeft() const {
	return m_topLeft;
}

Point& Rect::topLeft() {
	return m_topLeft;
}

Point Rect::bottomRight() const {
	return m_bottomRight;
}

Point& Rect::bottomRight() {
	return m_bottomRight;
}

Point Rect::size() const {
	return Point(width(), height());
}

}
