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

#ifndef XMPATTERN_H
#define XMPATTERN_H

#include "xmcell.h"

namespace ppp {
namespace xm {

class XmPattern {
		DISABLE_COPY(XmPattern)
	public:
		typedef std::shared_ptr<XmPattern> Ptr;
		typedef std::vector<Ptr> Vector;
	private:
		std::vector<XmCell::Vector> m_tracks;
		XmCell::Ptr createCell(uint16_t trackIndex, uint16_t row);
	public:
		XmPattern() = delete;
		XmPattern(int16_t chans);
		~XmPattern();
		bool load(BinStream& str);
		XmCell::Ptr cellAt(uint16_t trackIndex, uint16_t row);
		std::size_t numRows() const;
		std::size_t numChannels() const;
};

} // namespace xm
} // namespace ppp

#endif // XMPATTERN_H
