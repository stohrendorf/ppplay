#include "snaredrumoperator.h"
#include "opl3.h"
namespace opl
{
int16_t SnareDrumOperator::nextSample( opl::Phase modulator )
{
	if( envelopeGenerator()->isOff() )
		return 0;

	envelopeGenerator()->advance( egt(), am() );

	uint8_t phaseBit = ( phase().pre() >> 9 ) & 1;
	uint8_t noiseBit = rand() & 1;
	setPhase( ( ( 1 + phaseBit ) ^ noiseBit ) << 1 );
	return getOutput( modulator + (phase().pre()<<1), ws() );
}
}
