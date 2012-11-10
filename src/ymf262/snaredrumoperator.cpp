#include "snaredrumoperator.h"
#include "opl3.h"
namespace opl
{
int16_t SnareDrumOperator::nextSample( uint32_t modulator )
{
	if( envelopeGenerator()->isOff() )
		return 0;

	envelopeGenerator()->advance( egt(), am() );

	uint8_t phaseBit = ( phase() >> 19 ) & 1;
	uint8_t noiseBit = rand() & 1;
	setPhase( ( ( 1 + phaseBit ) ^ noiseBit ) << 18 );
	return getOutput( modulator + (phase()<<1), ws() );
}
}
