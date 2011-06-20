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

#include "genbase.h"
#include "stream/iarchive.h"

/**
* @file
* @brief Module base declarations (Implementation)
* @ingroup GenMod
*/

namespace ppp {

const std::array<const char[3], 12> NoteNames = {
	{
		"C-", "C#", "D-", "D#",
		"E-", "F-", "F#", "G-",
		"G#", "A-", "A#", "B-"
	}
};

GenOrder::GenOrder(uint8_t idx) : m_index(idx)
{ }

uint8_t GenOrder::index() const {
	return m_index;
}

void GenOrder::setIndex(const uint8_t n) {
	m_index = n;
}

IArchive& GenOrder::serialize(IArchive* data) {
	return *data & m_index;
}

}
