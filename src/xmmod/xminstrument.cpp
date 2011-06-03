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

#include "xminstrument.h"

#include <cstdint>

#pragma pack(push,1)
struct InstrumentHeader {
	uint32_t size;
	char name[22];
	uint8_t type;
	uint16_t numSamples;
};
struct InstrumentHeader2 {
	uint32_t size;
	uint8_t indices[96]; //!< @brief Maps note indices to their samples
	struct { int16_t x,y; } volEnvelope[12];
	struct { int16_t x,y; } panEnvelope[12];
	uint8_t numVolPoints;
	uint8_t numPanPoints;
	uint8_t volSustainPoint;
	uint8_t volLoopStart, volLoopEnd;
	uint8_t panSustainPoint;
	uint8_t panLoopStart, panLoopEnd;
	uint8_t volType, panType;
	uint8_t vibType, vibSweep, vibDepth, vibRate;
	uint16_t volFadeout;
	uint16_t reserved[11];
};
#pragma pack(pop)

using namespace ppp::xm;

XmInstrument::XmInstrument() : m_samples(), m_map() {
	std::fill_n( m_map, 96, 0 );
}

bool XmInstrument::load( BinStream& str ) {
	std::size_t startPos = str.pos();
	InstrumentHeader hdr;
	str.read( reinterpret_cast<char*>( &hdr ), sizeof( hdr ) );
	/*	if(hdr.type!=0) {
			LOG_WARNING("Instrument header type error @ 0x%.8x", str.pos()-sizeof(hdr));
			return false;
		}*/
	if( hdr.numSamples == 0 ) {
		str.seek( startPos + hdr.size );
		return true;
	}
	PPP_TEST( hdr.numSamples > 255 );
	m_samples.resize( hdr.numSamples );
	InstrumentHeader2 hdr2;
	str.read( reinterpret_cast<char*>( &hdr2 ), sizeof( hdr2 ) );
	std::copy( hdr2.indices, hdr2.indices + 96, m_map );
	str.seek( startPos + hdr.size );
	for( uint16_t i = 0; i < hdr.numSamples; i++ ) {
		XmSample::Ptr smp( new XmSample() );
		smp->load( str, 0 );
		m_samples[i] = smp;
	}
	for( uint16_t i = 0; i < hdr.numSamples; i++ ) {
		m_samples[i]->loadData( str );
	}
	m_title = ppp::stringncpy( hdr.name, 22 );
	return true;
}

uint8_t XmInstrument::mapNoteIndex( uint8_t note ) const {
	if( note >= 96 )
		return 0;
	return m_map[note]&15;
}

XmSample::Ptr XmInstrument::mapNoteSample( uint8_t note ) const {
	if( note >= 96 )
		return XmSample::Ptr();
	return m_samples[m_map[note]&15];
}

std::string XmInstrument::title() const
{
	return m_title;
}
