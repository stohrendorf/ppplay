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

#include "modsample.h"

#include "stream/binstream.h"

#include <boost/format.hpp>

#include <cstring>

/**
 * @ingroup ModMod
 * @{
 */

namespace ppp
{
namespace mod
{

#pragma pack(push,1)
/**
 * @struct Header
 * @brief Mod Sample Header
 * @note Big-endian values
 */
struct Header {
	char name[22];
	uint16_t length;
	uint8_t finetune;
	uint8_t volume;
	uint16_t loopStart;
	uint16_t loopLength;
};
#pragma pack(pop)

ModSample::ModSample() : m_finetune( 0 )
{
}

bool ModSample::loadHeader( BinStream& stream )
{
	Header hdr;
	stream.read( reinterpret_cast<char*>( &hdr ), sizeof( hdr ) );
	if( !stream.good() ) {
		return false;
	}
	swapEndian( &hdr.length );
	swapEndian( &hdr.loopStart );
	swapEndian( &hdr.loopLength );
	if( hdr.length > 1 ) {
		resizeData( hdr.length << 1 );
	}
	else {
		resizeData( 0 );
	}
	if( hdr.loopLength > 1 && ( hdr.loopLength + hdr.loopStart <= hdr.length ) ) {
		setLoopStart( hdr.loopStart << 1 );
		setLoopEnd( ( hdr.loopStart + hdr.loopLength ) << 1 );
		setLoopType( LoopType::Forward );
	}
	setTitle( stringncpy( hdr.name, 22 ) );
	logger()->debug( L4CXX_LOCATION, "Length=%u, loop=%u+%u=%u, name='%s'", length(), hdr.loopStart<<1, hdr.loopLength<<1, ( hdr.loopStart + hdr.loopLength )<<1, title() );
	setVolume( std::min<uint8_t>( hdr.volume, 0x40 ) );
	m_finetune = hdr.finetune & 0x0f;
	return stream.good();
}

bool ModSample::loadData( BinStream& stream )
{
	if( length() == 0 ) {
		return true;
	}
	{
		// check for the funny adpcm data
		char tmp[5];
		stream.read(tmp, 5);
		if(strncasecmp(tmp, "ADPCM", 5) == 0) {
			logger()->debug(L4CXX_LOCATION, "Detected ADPCM compressed sample data");
			return loadAdpcmData(stream);
		}
		stream.seekrel(-5);
	}
	logger()->debug(L4CXX_LOCATION, "Loading %d bytes sample data", length());
	if( stream.pos() + length() > stream.size() ) {
		logger()->warn( L4CXX_LOCATION, "File truncated: %u bytes requested while only %u bytes left. Truncating sample.", length(), stream.size() - stream.pos() );
		resizeData( stream.size() - stream.pos() );
	}
	for( auto it = beginIterator(); it != endIterator(); it++ ) {
		int8_t tmp;
		stream.read( &tmp );
		it->left = it->right = tmp << 8;
	}
	return stream.good();
}

bool ModSample::loadAdpcmData( BinStream& stream )
{
	int8_t compressionTable[16];
	stream.read(compressionTable, 16);
	// signed char GetDeltaValue(signed char prev, UINT n) const { return (signed char)(prev + CompressionTable[n & 0x0F]); }
	int8_t delta = 0;
	for(auto it = beginIterator(); it < endIterator(); ) {
		uint8_t tmpByte;
		stream.read( &tmpByte );
		delta += compressionTable[ lowNibble(tmpByte) ];
		it->left = it->right = delta<<8;
		it++;
		delta += compressionTable[ highNibble(tmpByte) ];
		it->left = it->right = delta<<8;
		it++;
	}
	return stream.good();
}

uint8_t ModSample::finetune() const
{
	return m_finetune;
}

light4cxx::Logger* ModSample::logger()
{
	return light4cxx::Logger::get( GenSample::logger()->name() + ".mod" );
}


}
}
