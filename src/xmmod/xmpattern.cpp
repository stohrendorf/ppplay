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
#include "xmcell.h"
#include "stuff/utils.h"
#include "stream/stream.h"

#include <boost/assert.hpp>
#include <boost/format.hpp>

namespace ppp
{
namespace xm
{

XmCell* XmPattern::createCell( uint16_t trackIndex, uint16_t row )
{
	BOOST_ASSERT( row < numRows() );
	BOOST_ASSERT( trackIndex < numChannels() );
	std::vector<XmCell*>& track = m_columns[ trackIndex ];
	XmCell*& cell = track[row];
	if( cell ) {
		return cell;
	}
	cell = new XmCell();
	return cell;
}

XmPattern::XmPattern( int16_t chans ) : m_columns( chans )
{
}

XmPattern::~XmPattern()
{
	for( auto& col : m_columns ) {
		deleteAll(col);
	}
	m_columns.clear();
}


bool XmPattern::load( Stream* str )
{
	logger()->trace( L4CXX_LOCATION, "Start: %#x", str->pos() );
	uint32_t hdrLen;
	*str >> hdrLen;
	logger()->trace( L4CXX_LOCATION, "hdrLen=%d", hdrLen );
	uint8_t packType;
	*str >> packType;
	if( packType != 0 ) {
		logger()->error( L4CXX_LOCATION, "Unsupported Pattern pack type: %d", int(packType) );
		return false;
	}
	uint16_t rows;
	*str >> rows;
	if( rows == 0 ) {
		// create a 64-row default pattern
		logger()->debug( L4CXX_LOCATION, "Number of rows = 0, creating 64-rows default pattern." );
	for( auto& chan : m_columns ) {
			for( int r = 0; r < 64; r++ ) {
				chan.push_back( new XmCell() );
			}
		}
		return true;
	}
	else if( rows < 1 || rows > 256 ) {
		logger()->warn( L4CXX_LOCATION, "Number of rows out of range: %d", rows );
		return false;
	}
	for( auto& chan : m_columns ) {
		chan.resize( rows, nullptr );
	}
	uint16_t packedSize;
	*str >> packedSize;
	logger()->trace( L4CXX_LOCATION, "Header end: %#x", str->pos() );
	str->seekrel( hdrLen - 9 ); // copied from schismtracker
	if( packedSize == 0 ) {
		return true;
	}
	for( uint16_t row = 0; row < rows; row++ ) {
		for( auto& chan : m_columns ) {
			XmCell* cell = new XmCell();
			chan.at( row ) = cell;
			if( !cell->load( str ) ) {
				return false;
			}
		}
	}
	return str->good();
}

XmCell* XmPattern::cellAt( uint16_t column, uint16_t row )
{
	if( column >= numChannels() || row >= numRows() ) {
		return nullptr;
	}
	return m_columns[column][row];
}

size_t XmPattern::numRows() const
{
	if( numChannels() == 0 ) {
		return 0;
	}
	return m_columns.front().size();
}

size_t XmPattern::numChannels() const
{
	return m_columns.size();
}

XmPattern* XmPattern::createDefaultPattern( int16_t chans )
{
	XmPattern* result = new XmPattern( chans );
	for( int i = 0; i < chans; i++ ) {
		for( int r = 0; r < 64; r++ ) {
			result->m_columns[i].push_back( new XmCell() );
		}
	}
	return result;
}

light4cxx::Logger* XmPattern::logger()
{
	return light4cxx::Logger::get( "pattern.xm" );
}

}
}

/**
 * @}
 */
