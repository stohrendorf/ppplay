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
#include "s3mcell.h"
#include "stream/stream.h"

namespace ppp
{
namespace s3m
{

S3mPattern::S3mPattern() : m_channels(32, std::vector<S3mCell*>( 64, nullptr ))
{
}

S3mPattern::~S3mPattern()
{
	for( std::vector<S3mCell*>& chan : m_channels ) {
		deleteAll(chan);
	}
	m_channels.clear();
}

S3mCell* S3mPattern::createCell( uint16_t chanIdx, int16_t row )
{
	if( row < 0 || row > 63 ) {
		throw std::out_of_range("Invalid row index");
	}
	if( chanIdx >= m_channels.size() ) {
		throw std::out_of_range("Invalid channel index");
	}
	auto& chan = m_channels[ chanIdx ];
	auto& cell = chan.at( row );
	if( cell != nullptr ) {
		return cell;
	}
	cell = new S3mCell();
	return cell;
}

S3mCell* S3mPattern::cellAt( uint16_t chanIdx, int16_t row )
{
	if( row < 0 || chanIdx >= m_channels.size() ) {
		return nullptr;
	}
	return m_channels[ chanIdx ].at( row );
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
			S3mCell* cell = createCell( currTrack, currRow );
			if( !cell->load( str ) ) {
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
