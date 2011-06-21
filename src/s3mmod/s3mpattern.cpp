/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "s3mpattern.h"
#include "genmod/gencell.h"

/**
* @file
* @brief S3M Pattern class
* @ingroup S3mMod
*/

namespace ppp {
namespace s3m {

S3mPattern::S3mPattern() : m_tracks() {
	try {
		for(uint8_t i = 0; i < 32; i++) {
			m_tracks.push_back(S3mCell::Vector(64));
		}
	}
	catch(...) {
		PPP_THROW("Unknown Exception");
	}
}

S3mPattern::~S3mPattern() {
}

S3mCell::Ptr S3mPattern::createCell(uint16_t trackIndex, int16_t row) {
	PPP_ASSERT((row >= 0) && (row <= 63));
	PPP_ASSERT(trackIndex < m_tracks.size());
	S3mCell::Vector* track = &m_tracks[trackIndex];
	S3mCell::Ptr& cell = track->at(row);
	if(cell)
		return cell;
	cell.reset(new S3mCell());
	return cell;
}

S3mCell::Ptr S3mPattern::cellAt(uint16_t trackIndex, int16_t row) {
	if(row < 0)
		return S3mCell::Ptr();
	if(trackIndex >= m_tracks.size())
		return S3mCell::Ptr();
	return m_tracks[trackIndex][row];
}

bool S3mPattern::load(BinStream& str, std::size_t pos) {
	try {
		uint16_t patSize;
		str.seek(pos);
		str.read(&patSize);
		uint16_t currRow = 0, currTrack = 0;
		while(currRow < 64) {
			uint8_t master;
			str.read(&master);
			if(master == 0) {
				currRow++;
				continue;
			}
			currTrack = master & 31;
			str.seekrel(-1);
			if(str.fail()) {
				LOG_ERROR("str.fail()...");
				return false;
			}
			S3mCell::Ptr cell = createCell(currTrack, currRow);
			if(!cell->load(str)) {
				LOG_ERROR("Cell loading: ERROR");
				return false;
			}
		}
		return true;
	}
	PPP_RETHROW()
	catch(...) {
		PPP_THROW("Unknown Exception");
	}
}

}
}
