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

#include "filestream.h"

#include <boost/assert.hpp>

#include <fstream>

FileStream::FileStream( const std::string& filename, Mode mode ) :
	Stream( new std::fstream( filename.c_str(), (mode==Mode::Read ? std::ios::in : std::ios::out) | std::ios::binary ), filename ),
	m_size( 0 )
{
	if( mode == Mode::Read ) {
		stream()->seekg( 0, std::ios::end );
		m_size = stream()->tellg();
		stream()->seekg( 0 );
	}
}

bool FileStream::isOpen() const
{
	const std::fstream* fs = dynamic_cast<const std::fstream*>(stream());
	BOOST_ASSERT_MSG( fs!=nullptr, "Stream is not a file stream" );
	return fs->is_open();
}

std::streamsize FileStream::size() const
{
	return m_size;
}
