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

#include "xmsample.h"

using namespace ppp;
using namespace ppp::xm;

XmSample::XmSample() : m_finetune( 0 ), m_panning( 0x80 ), m_relativeNote( 0 ), m_16bit( false )
{ }

bool XmSample::load( BinStream& str, std::size_t ) throw( PppException ) {
	int32_t dataSize;
	str.read( &dataSize );
	int32_t loopStart, loopLen;
	str.read( &loopStart );
	str.read( &loopLen );
	uint8_t volume;
	str.read( &volume );
	setVolume( volume );
	str.read( &m_finetune );
	uint8_t type;
	str.read( &type );
	switch( type & 3 ) {
		case 0:
		case 3:
			setLoopType( LoopType::None );
			break;
		case 1:
			setLoopType( LoopType::Forward );
			break;
		case 2:
			setLoopType( LoopType::Pingpong );
			break;
	}
	m_16bit = (type & 0x10)!=0;
	if(m_16bit) {
		setLength( dataSize/2 );
		setLoopStart( loopStart/2 );
		setLoopEnd( (loopStart + loopLen)/2 );
	}
	else {
		setLength( dataSize );
		setLoopStart( loopStart );
		setLoopEnd( loopStart + loopLen );
	}
	if( loopLen == 0 )
		setLoopType( LoopType::None );
	str.read( &m_panning );
	str.read( &m_relativeNote );
	str.seekrel( 1 );
	{
		char title[22];
		str.read( title, 22 );
		setTitle( stringncpy( title, 22 ) );
	}
	setDataMono( new BasicSample[getLength()] );
	std::fill_n( getNonConstDataMono(), getLength(), 0 );
	return str.good();
}

bool XmSample::loadData( BinStream& str ) {
	if( getLength() == 0 )
		return true;
	if( m_16bit ) { // 16 bit
		int16_t smp16 = 0;
		BasicSample* smpPtr = getNonConstDataMono();
		for( std::size_t i = 0; i < getLength(); i++ ) {
			int16_t delta;
			str.read( &delta );
			smp16 += delta;
			*( smpPtr++ ) = smp16;
		}
	}
	else { // 8 bit
		int8_t smp8 = 0;
		BasicSample* smpPtr = getNonConstDataMono();
		for( std::size_t i = 0; i < getLength(); i++ ) {
			int8_t delta;
			str.read( &delta );
			smp8 += delta;
			*( smpPtr++ ) = smp8 << 8;
		}
	}
	return str.good();
}
