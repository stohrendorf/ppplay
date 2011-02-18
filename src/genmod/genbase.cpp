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

#include "genbase.h"

/**
* @file
* @brief Module base declarations (Implementation)
* @ingroup GenMod
*/

namespace ppp {
	const std::array<const char[3], 12> NoteNames = {{
		"C-", "C#", "D-", "D#",
		"E-", "F-", "F#", "G-",
		"G#", "A-", "A#", "B-"
	}};

// 	const std::array<const int16_t, 64> ProtrackerLookup = {{
// 		      0,  24,  49,  74,  97, 120, 141, 161,
// 		    180, 197, 212, 224, 235, 244, 250, 253,
// 		    255, 253, 250, 244, 235, 224, 212, 197,
// 		    180, 161, 141, 120,  97,  74,  49,  24,
// 		   -  0, -24,- 49,- 74,- 97,-120,-141,-161,
// 		   -180,-197,-212,-224,-235,-244,-250,-253,
// 		   -255,-253,-250,-244,-235,-224,-212,-197,
// 		   -180,-161,-141,-120,- 97,- 74,- 49,- 24
// 	}};

	GenOrder::GenOrder(uint8_t idx) throw() : m_index(idx)
	{ }
	uint8_t GenOrder::getIndex() const throw() {
		return m_index;
	}
	void GenOrder::setIndex(const uint8_t n) throw() {
		m_index = n;
	}
	IArchive& GenOrder::serialize(IArchive* data) {
		return *data & m_index;
	}
}
