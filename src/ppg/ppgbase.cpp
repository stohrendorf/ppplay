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

#include "ppgbase.h"
#include <algorithm>

namespace ppg {

Widget::Widget(Widget* parent) :
		m_visible(false),
		m_parent(parent),
		m_left(0), m_top(0), m_bottom(0), m_right(0),
		m_children()
{
	if(parent)
		parent->m_children.push_back(this);
}

Widget::~Widget() throw() {
	if(m_parent)
		m_parent->m_children.remove(this);
	std::list<Widget*> backup = m_children;
	std::for_each(
		backup.begin(),
		backup.end(),
		[](Widget*w){delete w;}
	);
}

int Widget::setLeft(const int x, const bool absolute) throw() {
	int tmp = getWidth() - 1;
	m_left = x;
	m_right = m_left + tmp;
	if (absolute) {
		mapToAbsolute(&m_left, NULL);
		mapToAbsolute(&m_right, NULL);
	}
	return m_left;
}

int Widget::setTop(const int y, const bool absolute) throw() {
	int tmp = getHeight() - 1;
	m_top = y;
	m_bottom = m_top + tmp;
	if (absolute) {
		mapToAbsolute(NULL, &m_top);
		mapToAbsolute(NULL, &m_bottom);
	}
	return m_top;
}

bool Widget::setPosition(const int x, const int y, const bool absolute) throw() {
	setLeft(x, absolute);
	setTop(y, absolute);
	return (getLeft() != x) || (getTop() != y);
}

int Widget::setRight(const int x, const bool absolute) throw() {
	int tmp = getWidth() - 1;
	m_right = x;
	m_left = x - tmp;
	if (absolute)
		mapToAbsolute(&m_right, &tmp);
	return m_right;
}

int Widget::setBottom(const int y, const bool absolute) throw() {
	int tmp = getHeight() - 1;
	m_bottom = y;
	m_top = y - tmp;
	if (absolute)
		mapToAbsolute(&tmp, &m_top);
	return m_bottom;
}

bool Widget::setBottomRight(const int x, const int y, const bool absolute) throw() {
	setRight(x, absolute);
	setBottom(y, absolute);
	return (getBottom() != y) || (getRight() != x);
}

int Widget::setWidth(const int w) throw(Exception) {
	PPG_TEST(w <= 0);
	m_right = m_left + w - 1;
	return getWidth();
}

int Widget::setHeight(const int h) throw(Exception) {
	PPG_TEST(h <= 0);
	m_bottom = m_top + h - 1;
	return 1;
}

bool Widget::setDimensions(int w, int h) throw(Exception) {
	setWidth(w);
	setHeight(h);
	return (getHeight() != h) || (getWidth() != w);
}

void Widget::draw() throw(Exception) {
	if (!isVisible())
		return;
	// draw from bottom to top so that top elements are drawn over bottom ones
	for(std::list<Widget*>::reverse_iterator revIt = m_children.rbegin(); revIt!=m_children.rend(); revIt++) {
		Widget* w = *revIt;
		if( !w->isVisible() )
			continue;
		w->draw();
	}
	drawThis();
}

int Widget::getLeft() const throw() {
	return m_left;
}

int Widget::getTop() const throw() {
	return m_top;
}

int Widget::getRight() const throw() {
	return m_right;
}

int Widget::getBottom() const throw() {
	return m_bottom;
}

int Widget::getWidth() const throw() {
	return m_right - m_left + 1;
}

int Widget::getHeight() const throw() {
	return m_bottom - m_top + 1;
}

void Widget::show() throw() {
	m_visible = true;
}

void Widget::hide() throw() {
	m_visible = false;
}

bool Widget::isVisible() const throw() {
	return m_visible;
}

void Widget::drawChar(int x, int y, const char c) throw() {
	if ((x < 0) || (x >= getWidth()) || (y < 0) || (y >= getHeight()))
		return;
	if (m_parent) {
		mapToParent(&x, &y);
		m_parent->drawChar(x, y, c);
	}
}

void Widget::drawFgColor(int x, int y, const unsigned char c) throw() {
	if ((x < 0) || (x >= getWidth()) || (y < 0) || (y >= getHeight()))
		return;
	if (m_parent) {
		mapToParent(&x, &y);
		m_parent->drawFgColor(x, y, c);
	}
}

void Widget::drawBgColor(int x, int y, const unsigned char c) throw() {
	if ((x < 0) || (x >= getWidth()) || (y < 0) || (y >= getHeight()))
		return;
	if (m_parent) {
		mapToParent(&x, &y);
		m_parent->drawBgColor(x, y, c);
	}
}

void Widget::mapToParent(int *x, int *y) throw() {
	if (!m_parent)
		return;
	if (x != NULL)
		*x += getLeft();
	if (y != NULL)
		*y += getTop();
}

void Widget::mapToAbsolute(int *x, int *y) throw() {
	if (!m_parent)
		return;
	mapToParent(x, y);
	m_parent->mapToAbsolute(x, y);
}

Widget *Widget::getTopParent() throw() {
	if (!m_parent)
		return NULL;
	else
		return m_parent->getTopParent();
}

void Widget::toTop(ppg::Widget* vp) throw() {
	if(std::find(m_children.begin(), m_children.end(), vp)==m_children.end())
		return;
	m_children.remove(vp);
	m_children.push_front(vp);
}
} // namespace ppg
