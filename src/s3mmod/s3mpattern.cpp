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
#include "stream/stream.h"

namespace ppp
{
namespace s3m
{

S3mPattern::S3mPattern() : Field<S3mCell>( 32, 64 )
{
}

bool S3mPattern::load( Stream* str, size_t pos )
{
    try {
        uint16_t patSize;
        str->seek( pos );
        *str >> patSize;
        uint16_t currRow = 0, currTrack = 0;
        while( currRow < 64 ) {
            uint8_t master;
            *str >> master;
            if( master == 0 ) {
                currRow++;
                continue;
            }
            currTrack = master & 31;
            str->seekrel( -1 );
            if( !str->good() ) {
                logger()->error( L4CXX_LOCATION, "str->fail()..." );
                return false;
            }
            if( !at( currTrack, currRow ).load( str ) ) {
                logger()->error( L4CXX_LOCATION, "Cell loading: ERROR" );
                return false;
            }
        }
        return str->good();
    }
    catch( boost::exception& e ) {
        BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
    }
    catch( ... ) {
        BOOST_THROW_EXCEPTION( std::runtime_error( "Unknown exception" ) );
    }
}

light4cxx::Logger* S3mPattern::logger()
{
    return light4cxx::Logger::get( "pattern.s3m" );
}

}
}

/**
 * @}
 */
