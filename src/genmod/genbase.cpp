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

	const int16_t ProtrackerLookup[256] = {
		      0,   0,   0,   0,  24,  24,  24,  24,  49,  49,  49,  49,  74,  74,  74,  74,  97,  97,  97,  97, 120, 120, 120, 120, 141, 141, 141, 141, 161, 161, 161, 161,
		    180, 180, 180, 180, 197, 197, 197, 197, 212, 212, 212, 212, 224, 224, 224, 224, 235, 235, 235, 235, 244, 244, 244, 244, 250, 250, 250, 250, 253, 253, 253, 253,
		    255, 255, 255, 255, 253, 253, 253, 253, 250, 250, 250, 250, 244, 244, 244, 244, 235, 235, 235, 235, 224, 224, 224, 224, 212, 212, 212, 212, 197, 197, 197, 197,
		    180, 180, 180, 180, 161, 161, 161, 161, 141, 141, 141, 141, 120, 120, 120, 120,  97,  97,  97,  97,  74,  74,  74,  74,  49,  49,  49,  49,  24,  24,  24,  24,
		   -  0,-  0,-  0,-  0,- 24,- 24,- 24,- 24,- 49,- 49,- 49,- 49,- 74,- 74,- 74,- 74,- 97,- 97,- 97,- 97,-120,-120,-120,-120,-141,-141,-141,-141,-161,-161,-161,-161,
		   -180,-180,-180,-180,-197,-197,-197,-197,-212,-212,-212,-212,-224,-224,-224,-224,-235,-235,-235,-235,-244,-244,-244,-244,-250,-250,-250,-250,-253,-253,-253,-253,
		   -255,-255,-255,-255,-253,-253,-253,-253,-250,-250,-250,-250,-244,-244,-244,-244,-235,-235,-235,-235,-224,-224,-224,-224,-212,-212,-212,-212,-197,-197,-197,-197,
		   -180,-180,-180,-180,-161,-161,-161,-161,-141,-141,-141,-141,-120,-120,-120,-120,- 97,- 97,- 97,- 97,- 74,- 74,- 74,- 74,- 49,- 49,- 49,- 49,- 24,- 24,- 24,- 24,
	};

	GenOrder::GenOrder(uint8_t idx) throw() : m_index{idx}, m_count{0}, m_states{} {
		m_states.push_back(BinStream::SpBinStream(new SBinStream()));
	}
	BinStream::SpBinStream GenOrder::getState(uint16_t idx) throw(PppException) {
		PPP_TEST(idx>=m_states.size());
		return m_states[idx];
	}
	BinStream::SpBinStream GenOrder::getCurrentState() throw(PppException) {
		return getState(m_count);
	}
	uint8_t GenOrder::getIndex() const throw() {
		return m_index;
	}
	void GenOrder::setIndex(const uint8_t n) throw() {
		m_index = n;
	}
	uint8_t GenOrder::getCount() const throw() {
		return m_count;
	}
	void GenOrder::setCount(const uint8_t n) throw() {
		m_count = n;
	}
	uint8_t GenOrder::incCount() throw(PppException) {
		PPP_TEST(m_count==0xff);
		m_count++;
		while(m_count>=m_states.size())
			m_states.push_back( BinStream::SpBinStream(new SBinStream()) );
		return m_count;
	}
	void GenOrder::resetCount() throw() {
		m_count = 0;
	}
}
