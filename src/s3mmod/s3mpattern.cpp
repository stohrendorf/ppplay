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

/**
 * @ingroup S3mMod
 * @{
 */

#include <boost/exception/all.hpp>

#include "s3mpattern.h"

#include "logger/logger.h"

namespace ppp {
namespace s3m {

S3mPattern::S3mPattern() : m_channels() {
	for(uint8_t i = 0; i < 32; i++) {
		m_channels.push_back(S3mCell::Vector(64));
	}
}

S3mCell::Ptr S3mPattern::createCell(uint16_t trackIndex, int16_t row) {
	BOOST_ASSERT((row >= 0) && (row <= 63));
	BOOST_ASSERT(trackIndex < m_channels.size());
	S3mCell::Vector& track = m_channels.at(trackIndex);
	S3mCell::Ptr& cell = track.at(row);
	if(cell) {
		return cell;
	}
	cell.reset(new S3mCell());
	return cell;
}

S3mCell::Ptr S3mPattern::cellAt(uint16_t chanIdx, int16_t row) {
	if(row < 0) {
		return S3mCell::Ptr();
	}
	if(chanIdx >= m_channels.size()) {
		return S3mCell::Ptr();
	}
	return m_channels.at(chanIdx).at(row);
}

bool S3mPattern::load(BinStream& str, size_t pos) {
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
	catch( boost::exception& e) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
	catch(...) {
		BOOST_THROW_EXCEPTION( std::runtime_error("Unknown exception") );
	}
}

}
}

/**
 * @}
 */
