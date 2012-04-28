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

#include "fbinstream.h"

#include <fstream>

FBinStream::FBinStream( const std::string& filename ) :
	BinStream( SpIoStream( new std::fstream( filename.c_str(), std::ios::in | std::ios::binary ) ) ),
	m_filename( filename ), m_size( 0 )
{
	stream()->seekg( 0, std::ios::end );
	m_size = stream()->tellg();
	stream()->seekg( 0 );
}

FBinStream::~FBinStream()
{
	if( stream().unique() ) {
		std::static_pointer_cast<std::fstream>( stream() )->close();
	}
}

bool FBinStream::isOpen() const
{
	return std::static_pointer_cast<std::fstream>( stream() )->is_open();
}

std::string FBinStream::filename() const
{
	return m_filename;
}

size_t FBinStream::size() const
{
	return m_size;
}
