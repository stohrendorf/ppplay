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

#include "gencell.h"

/**
* @file
* @ingroup GenMod
* @brief General Pattern and Track definitions
*/

using namespace ppp;

GenCell::GenCell() throw() : m_active( false ) {
}

GenCell::~GenCell() {
}

void GenCell::reset() throw() {
	m_active = false;
}

bool GenCell::isActive() const throw() {
	return m_active;
}

IArchive& GenCell::serialize( IArchive* data ) {
	*data& m_active;
	return *data;
}

void GenCell::setActive(bool a) throw()
{
    m_active = a;
}
