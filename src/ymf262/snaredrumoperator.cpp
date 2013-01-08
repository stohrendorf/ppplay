#include "snaredrumoperator.h"
#include "opl3.h"
namespace opl
{
int16_t SnareDrumOperator::nextSample( uint16_t modulator )
{
	if( envelopeGenerator()->isOff() ) {
		return 0;
	}

	envelopeGenerator()->advance( egt(), am() );

	uint8_t phaseBit = ( phase() >> 9 ) & 1;
	uint8_t noiseBit = opl()->randBit();
	setPhase( ( ( 1 + phaseBit ) ^ noiseBit ) << 1 );
	return getOutput( modulator + (phase()<<1), ws() );
}
}
