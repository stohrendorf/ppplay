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
};
struct InstrumentHeader2 {
};
#pragma pack(pop)

using namespace ppp::xm;

XmInstrument::XmInstrument() : m_samples(), m_map() {
	std::fill_n( m_map, 96, 0 );
}

bool XmInstrument::load( BinStream& str ) {
	std::size_t startPos = str.pos();
	InstrumentHeader hdr;
	LOG_DEBUG( "Loading Instrument header @ 0x%.8x", str.pos() );
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
	LOG_DEBUG( "Loading Instrument header part 2 @ 0x%.8x", str.pos() );
	str.read( reinterpret_cast<char*>( &hdr2 ), sizeof( hdr2 ) );
	std::copy( hdr2.indices, hdr2.indices + 96, m_map );
	LOG_DEBUG( "Loading %d samples", hdr.numSamples );
	str.seek( startPos + hdr.size );
	for( uint16_t i = 0; i < hdr.numSamples; i++ ) {
		XmSample::Ptr smp( new XmSample() );
		smp->load( str, 0 );
		m_samples[i] = smp;
	}
	for( uint16_t i = 0; i < hdr.numSamples; i++ ) {
		m_samples[i]->loadData( str );
	}
	return true;
}

uint8_t XmInstrument::mapNoteIndex( uint8_t note ) const {
	if( note >= 96 )
		return 0;
	return m_map[note];
}

XmSample::Ptr XmInstrument::mapNoteSample( uint8_t note ) const {
	if( note >= 96 )
		return XmSample::Ptr();
	return m_samples[m_map[note]];
}
