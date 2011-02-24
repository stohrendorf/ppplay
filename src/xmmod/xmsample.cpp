#include "xmsample.h"

using namespace ppp;
using namespace ppp::xm;

XmSample::XmSample() : m_finetune( 0 ), m_panning( 128 ), m_relativeNote( 0 ), m_16bit( false )
{ }

bool XmSample::load( BinStream& str, const std::size_t ) throw( PppException ) {
	LOG_DEBUG( "Loading xm sample @ 0x%.8x", str.pos() );
	int32_t sampleLength;
	str.read( &sampleLength );
	setLength( sampleLength );
	int32_t loopStart, loopLen;
	str.read( &loopStart );
	setLoopStart( loopStart );
	str.read( &loopLen );
	setLoopEnd( loopStart + loopLen );
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
		case 2:
			setLoopType( LoopType::Pingpong );
	}
	m_16bit = type & 4;
	if( loopLen == 0 )
		setLoopType( LoopType::None );
	str.read( &m_panning );
	str.read( &m_relativeNote );
	str.seekrel( 1 );
	{
		char title[22];
		LOG_DEBUG( "Sample title @ 0x%.8x", str.pos() );
		str.read( title, 22 );
		setTitle( stringncpy( title, 22 ) );
	}
	setDataMono( new int16_t[getLength()] );
	std::fill_n( getNonConstDataMono(), getLength(), 0 );
	return str.good();
}

bool XmSample::loadData( BinStream& str ) {
	LOG_DEBUG( "Loading xm sample data @ 0x%.8x (%d samples)", str.pos(), getLength() );
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
