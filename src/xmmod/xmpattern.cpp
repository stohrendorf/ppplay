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
 * @ingroup XmModule
 * @{
 */

#include "xmpattern.h"

#include <boost/assert.hpp>
#include <boost/format.hpp>

namespace ppp {
namespace xm {

XmCell::Ptr XmPattern::createCell(uint16_t trackIndex, uint16_t row) {
	BOOST_ASSERT(row < numRows());
	BOOST_ASSERT(trackIndex < numChannels());
	XmCell::Vector* track = &m_columns.at(trackIndex);
	XmCell::Ptr& cell = track->at(row);
	if(cell) {
		return cell;
	}
	cell.reset(new XmCell());
	return cell;
}

XmPattern::XmPattern(int16_t chans) : m_columns(chans) {
}

bool XmPattern::load(BinStream& str) {
	logger()->trace(L4CXX_LOCATION, boost::format("Start: %#x")%str.pos());
	uint32_t hdrLen;
	str.read(&hdrLen);
	logger()->trace(L4CXX_LOCATION, boost::format("hdrLen=%d")%hdrLen);
	uint8_t packType;
	str.read(&packType);
	if(packType != 0) {
		logger()->error(L4CXX_LOCATION, boost::format("Unsupported Pattern pack type: %d")%(packType+0));
		return false;
	}
	uint16_t rows;
	str.read(&rows);
	if(rows==0) {
		// create a 64-row default pattern
		logger()->debug(L4CXX_LOCATION, "Number of rows = 0, creating 64-rows default pattern.");
		for(XmCell::Vector& chan : m_columns) {
			for(int r = 0; r < 64; r++) {
				chan.push_back(XmCell::Ptr(new XmCell()));
			}
		}
		return true;
	}
	else if(rows < 1 || rows > 256) {
		logger()->warn(L4CXX_LOCATION, boost::format("Number of rows out of range: %d")%rows);
		return false;
	}
	for(XmCell::Vector& chan : m_columns) {
		chan.resize(rows, XmCell::Ptr());
	}
	uint16_t packedSize;
	str.read(&packedSize);
	logger()->trace(L4CXX_LOCATION, boost::format("Header end: %#x")%str.pos());
	str.seekrel(hdrLen - 9);   // copied from schismtracker
	if(packedSize == 0) {
/*		for(size_t i = 0; i < m_columns.size(); i++) {
			m_columns.at(i).clear();
			for(int r = 0; r < 64; r++) {
				m_columns.at(i).push_back(XmCell::Ptr(new XmCell()));
			}
		}*/
		return true;
	}
	for(uint16_t row = 0; row < rows; row++) {
		for(XmCell::Vector& chan : m_columns) {
			XmCell* cell = new XmCell();
			if(!cell->load(str)) {
				return false;
			}
			chan.at(row).reset(cell);
		}
	}
	return !str.fail();
}

XmCell::Ptr XmPattern::cellAt(uint16_t column, uint16_t row) {
	if(column >= numChannels() || row >= numRows()) {
		return XmCell::Ptr();
	}
	const XmCell::Vector& track = m_columns.at(column);
	return track.at(row);
}

size_t XmPattern::numRows() const {
	if(numChannels() == 0) {
		return 0;
	}
	return m_columns.at(0).size();
}

size_t XmPattern::numChannels() const {
	return m_columns.size();
}

XmPattern::Ptr XmPattern::createDefaultPattern(int16_t chans) {
	XmPattern::Ptr result(new XmPattern(chans));
	for(int i = 0; i < chans; i++) {
		for(int r = 0; r < 64; r++) {
			result->m_columns.at(i).push_back(XmCell::Ptr(new XmCell()));
		}
	}
	return result;
}

light4cxx::Logger::Ptr XmPattern::logger()
{
	return light4cxx::Logger::get( "pattern.xm" );
}

}
}

/**
 * @}
 */
