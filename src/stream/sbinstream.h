/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef SBINSTREAM_H
#define SBINSTREAM_H

#include "binstream.h"

/**
 * @class SBinStream
 * @ingroup Common
 * @brief Class derived from BinStream for a std::stringstream
 */
class SBinStream : public BinStream {
		DISABLE_COPY( SBinStream )
	public:
		explicit SBinStream();
		virtual ~SBinStream();
		virtual size_t size() const;
};

#endif
