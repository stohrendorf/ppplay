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

/**
 * @ingroup ModMod
 * @{
 */

namespace ppp {
namespace mod {

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

ModSample::ModSample() : m_finetune(0)
{
}

bool ModSample::loadHeader(BinStream& stream)
{
	Header hdr;
	stream.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
	if(!stream.good()) {
		return false;
	}
	swapEndian(&hdr.length);
	swapEndian(&hdr.loopStart);
	swapEndian(&hdr.loopLength);
	if(hdr.length>1) {
		setLength(hdr.length<<1);
	}
	if(hdr.loopLength>1 && (hdr.loopLength+hdr.loopStart<=hdr.length)) {
		setLoopStart(hdr.loopStart<<1);
		setLoopEnd((hdr.loopStart+hdr.loopLength)<<1);
		setLoopType(LoopType::Forward);
	}
	setTitle( stringncpy(hdr.name, 22) );
	LOG4CXX_DEBUG(logger(), boost::format("Length=%u, loop=%u+%u=%u, name='%s'")%length()%hdr.loopStart%hdr.loopLength%(hdr.loopStart+hdr.loopLength)%title());
// 	LOG_DEBUG("Loading sample (length=%u, loop=%u+%u=%u, name='%s', vol=%u)", length(), hdr.loopStart, hdr.loopLength, hdr.loopStart+hdr.loopLength, title().c_str(), hdr.volume);
	setVolume( std::min<uint8_t>( hdr.volume, 0x40 ) );
	m_finetune = hdr.finetune&0x0f;
	return stream.good();
}

bool ModSample::loadData(BinStream& stream)
{
	if(length()==0) {
		return true;
	}
	if(stream.pos()+length()>stream.size()) {
		LOG4CXX_WARN(logger(), boost::format("File truncated: %u bytes requested while only %u bytes left. Truncating sample.")%length()%(stream.size()-stream.pos()));
		setLength( stream.size()-stream.pos() );
	}
	setDataMono( new BasicSample[length()] );
	BasicSample* s = nonConstDataMono();
	for(size_t i=0; i<length(); i++) {
		int8_t tmp;
		stream.read(&tmp);
		*(s++) = tmp<<8;
	}
	return stream.good();
}

uint8_t ModSample::finetune() const
{
	return m_finetune;
}

log4cxx::LoggerPtr ModSample::logger()
{
	return log4cxx::Logger::getLogger( GenSample::logger()->getName() + ".mod" );
}


}
}
