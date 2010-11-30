/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
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

#include "genpattern.h"

/**
* @file
* @ingroup GenMod
* @brief General Pattern and Track definitions
*/

using namespace ppp;
GenCell::GenCell() throw() : m_active(false) {
}

GenCell::~GenCell() {
}

void GenCell::reset() throw() {
	m_active = false;
}

bool GenCell::isActive() const throw() {
	return m_active;
}

std::string GenCell::trackerString() const throw() {
	return "";
}

BinStream &GenCell::serialize(BinStream &str) const {
	return str.write(&m_active);
}

BinStream &GenCell::unserialize(BinStream &str) {
	return str.read(&m_active);
}
