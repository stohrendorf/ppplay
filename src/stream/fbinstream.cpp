/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Syron <mr.syron@googlemail.com>

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

#include "fbinstream.h"

FBinStream::FBinStream( const std::string& filename ) :
	BinStream( SpIoStream( new std::fstream( filename.c_str(), std::ios::in | std::ios::binary ) ) ),
	m_filename( filename ) {
}

FBinStream::~FBinStream() {
	std::static_pointer_cast<std::fstream>( stream() )->close();
}

bool FBinStream::isOpen() const {
	return std::static_pointer_cast<std::fstream>( stream() )->is_open();
}