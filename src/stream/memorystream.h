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

#ifndef PPPLAY_MEMORYSTREAM_H
#define PPPLAY_MEMORYSTREAM_H

#include "stream.h"

/**
 * @class MemoryStream
 * @ingroup Common
 * @brief Class derived from Stream for a std::stringstream
 */
class PPPLAY_STREAM_EXPORT MemoryStream : public Stream
{
	DISABLE_COPY( MemoryStream )
public:
	explicit MemoryStream( const std::string& name ="SBinStream" );
	virtual std::streamsize size() const;
};

#endif
