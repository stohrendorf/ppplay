#include "snaredrumoperator.h"
#include "opl3.h"
namespace opl
{
Fractional9 SnareDrumOperator::nextSample( Fractional9 modulator )
{
	if( envelopeGenerator()->isOff() )
		return 0;

	envelopeGenerator()->advance( egt(), am() );

	uint8_t phaseBit = ( phase().trunc() >> 9 ) & 1;
	uint8_t noiseBit = rand() & 1;
	setPhase( ( ( 1 + phaseBit ) ^ noiseBit ) << 1 );
	return getOutput( modulator + (phase()<<1), ws() );
}
}
