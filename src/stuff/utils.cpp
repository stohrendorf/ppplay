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

#include "utils.h"
#include <cstdarg>

namespace ppp {
	std::string stringf( const char fmt[], ... ) {
		va_list args;
		va_start( args, fmt );
		char* tmp = new char[1024];
		vsnprintf( tmp, 1024, fmt, args );
		va_end( args );
		std::string res( tmp );
		delete[] tmp;
		return res;
	}

	std::string stringncpy( const char src[], std::size_t maxlen ) {
		std::string res;
		for( std::size_t i = 0; i < maxlen; i++ ) {
			if( src[i] == 0x00 )
				break;
			res += src[i];
		}
		return res;
	}

	void swapEndian( char* data, std::size_t size ) {
		for( std::size_t i = 0; i < size / 2; i++ )
			std::swap( data[i], data[size - i - 1] );
	}
}
