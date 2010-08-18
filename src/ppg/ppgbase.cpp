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

PpgWidget::PpgWidget(const std::string &name) throw() :
		m_isContainer(true),
		m_name(name), m_type("Pure Virtual"),
		m_visible(false),
		m_parent(),
		m_left(0), m_top(0), m_bottom(0), m_right(0),
		m_children() {
	return;
}

PpgWidget::PpgWidget(const PpgWidget& src) throw() :
		m_isContainer(src.m_isContainer),
		m_name(src.m_name), m_type(src.m_type), m_visible(false), m_parent(src.m_parent),
		m_left(src.m_left), m_top(src.m_top), m_bottom(src.m_bottom), m_right(src.m_right),
		m_children(src.m_children) {
}

PpgWidget::~PpgWidget() throw() {
	if(isContainer()) {
		std::for_each(
			m_children.begin(),
			m_children.end(),
			[](PpgWidget*w){delete w;}
		);
	}
}

PpgWidget &PpgWidget::operator=(const PpgWidget & /*src*/) throw() {
	// here are no values that should be copied...
	return *this;
}

int PpgWidget::setLeft(const int x, const bool absolute) throw() {
	int tmp = getWidth() - 1;
	m_left = x;
	m_right = m_left + tmp;
	if (absolute) {
		mapToAbsolute(&m_left, NULL);
		mapToAbsolute(&m_right, NULL);
	}
	return m_left;
}

int PpgWidget::setTop(const int y, const bool absolute) throw() {
	int tmp = getHeight() - 1;
	m_top = y;
	m_bottom = m_top + tmp;
	if (absolute) {
		mapToAbsolute(NULL, &m_top);
		mapToAbsolute(NULL, &m_bottom);
	}
	return m_top;
}

bool PpgWidget::setPosition(const int x, const int y, const bool absolute) throw() {
	setLeft(x, absolute);
	setTop(y, absolute);
	return (getLeft() != x) || (getTop() != y);
}

int PpgWidget::setRight(const int x, const bool absolute) throw() {
	int tmp = getWidth() - 1;
	m_right = x;
	m_left = x - tmp;
	if (absolute)
		mapToAbsolute(&m_right, &tmp);
	return m_right;
}

int PpgWidget::setBottom(const int y, const bool absolute) throw() {
	int tmp = getHeight() - 1;
	m_bottom = y;
	m_top = y - tmp;
	if (absolute)
		mapToAbsolute(&tmp, &m_top);
	return m_bottom;
}

bool PpgWidget::setBottomRight(const int x, const int y, const bool absolute) throw() {
	setRight(x, absolute);
	setBottom(y, absolute);
	return (getBottom() != y) || (getRight() != x);
}

int PpgWidget::setWidth(const int w) throw(PpgException) {
	PPG_TEST(w <= 0);
	m_right = m_left + w - 1;
	return getWidth();
}

int PpgWidget::setHeight(const int h) throw(PpgException) {
	PPG_TEST(h <= 0);
	m_bottom = m_top + h - 1;
	return 1;
}

bool PpgWidget::setDimensions(int w, int h) throw(PpgException) {
	setWidth(w);
	setHeight(h);
	return (getHeight() != h) || (getWidth() != w);
}

void PpgWidget::draw() throw(PpgException) {
	if (!isVisible())
		return;
	// draw from bottom to top so that top elements are drawn over bottom ones
	if(isContainer()) {
		for (int i = m_children.size() - 1; i >= 0; i--) {
			if (!m_children[i]->isVisible())
				continue;
			m_children[i]->draw();
		}
	}
	drawThis();
}

int PpgWidget::getLeft() const throw() {
	return m_left;
}

int PpgWidget::getTop() const throw() {
	return m_top;
}

int PpgWidget::getRight() const throw() {
	return m_right;
}

int PpgWidget::getBottom() const throw() {
	return m_bottom;
}

int PpgWidget::getWidth() const throw() {
	return m_right - m_left + 1;
}

int PpgWidget::getHeight() const throw() {
	return m_bottom - m_top + 1;
}

std::string PpgWidget::getName() const throw() {
	return m_name;
}

std::string PpgWidget::getType() const throw() {
	return m_type;
}

void PpgWidget::show() throw() {
	m_visible = true;
}

void PpgWidget::hide() throw() {
	m_visible = false;
}

bool PpgWidget::isVisible() const throw() {
	return m_visible;
}

void PpgWidget::drawChar(int x, int y, const char c) throw() {
	if ((x < 0) || (x >= getWidth()) || (y < 0) || (y >= getHeight()))
		return;
	if (m_parent) {
		mapToParent(&x, &y);
		m_parent->drawChar(x, y, c);
	}
}

void PpgWidget::drawFgColor(int x, int y, const unsigned char c) throw() {
	if ((x < 0) || (x >= getWidth()) || (y < 0) || (y >= getHeight()))
		return;
	if (m_parent) {
		mapToParent(&x, &y);
		m_parent->drawFgColor(x, y, c);
	}
}

void PpgWidget::drawBgColor(int x, int y, const unsigned char c) throw() {
	if ((x < 0) || (x >= getWidth()) || (y < 0) || (y >= getHeight()))
		return;
	if (m_parent) {
		mapToParent(&x, &y);
		m_parent->drawBgColor(x, y, c);
	}
}

void PpgWidget::mapToParent(int *x, int *y) throw() {
	if (!m_parent)
		return;
	if (x != NULL)
		*x += getLeft();
	if (y != NULL)
		*y += getTop();
}

void PpgWidget::mapToAbsolute(int *x, int *y) throw() {
	if (!m_parent)
		return;
	mapToParent(x, y);
	m_parent->mapToAbsolute(x, y);
}

void PpgWidget::setParent(PpgWidget *newParent, PpgWidget *caller) throw() {
	PPG_TEST(newParent==NULL);
	if (m_parent)
		m_parent->removeChild(newParent);
	m_parent = newParent;
	if (m_parent != caller)
		m_parent->addChild(*newParent);
}

void PpgWidget::drawThis() throw(PpgException) {
}

PpgWidget *PpgWidget::getTopParent() throw() {
	if (!m_parent)
		return NULL;
	else
		return m_parent->getTopParent();
}

void PpgWidget::addChild(PpgWidget &child) throw(PpgException) {
	PPG_TEST(!isContainer());
	m_children.push_back(&child);
	child.setParent(this, this);
}


bool PpgWidget::removeChild(PpgWidget *child) throw() {
	PPG_TEST(!isContainer());
	std::vector<PpgWidget*>::iterator it = std::find(m_children.begin(), m_children.end(), child);
	if(it!=m_children.end()) {
		delete *it;
		m_children.erase(it);
		return true;
	}
	return false;
}

PpgWidget *PpgWidget::firstChild() throw() {
	PPG_TEST(!isContainer())
	if (m_children.empty())
		return NULL;
	return m_children[0];
}

void PpgWidget::toTop(const std::string &name) throw() {
	PPG_TEST(!isContainer());
	for (unsigned int i = 0; i < m_children.size(); i++) {
		if (m_children[i]->getName() == name) {
			toTop(i);
			break;
		}
	}
}

void PpgWidget::toTop(PpgWidget &vp) throw() {
	PPG_TEST(!isContainer());
	for (unsigned int i = 0; i < m_children.size(); i++) {
		if (m_children[i] == &vp) {
			toTop(i);
			break;
		}
	}
}

void PpgWidget::toTop(unsigned int zOrder) throw() {
	PPG_TEST(!isContainer());
	if ((zOrder < 1) || (zOrder >= m_children.size()))
		return;
	while (zOrder > 1) {
		PpgWidget *tmp = m_children[zOrder-1];
		m_children[zOrder-1] = m_children[zOrder];
		m_children[zOrder] = tmp;
		zOrder--;
	}
}

template class std::vector<PpgWidget*>;
