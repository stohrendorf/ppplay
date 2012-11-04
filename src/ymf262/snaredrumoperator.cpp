#include "snaredrumoperator.h"
#include "opl3.h"
namespace opl
{
int16_t SnareDrumOperator::nextSample( uint16_t modulator )
{
	if( envelopeGenerator()->isOff() )
		return 0;

	setEnvelope( envelopeGenerator()->getEnvelope( egt(), am() ) );

	setPhase( opl()->highHatOperator()->phase() * 2 );
	uint8_t phaseBit = ( phase() >> 8 ) & 1;
	uint8_t noiseBit = rand() & 1;
	setPhase( ( ( 1 + phaseBit ) ^ noiseBit ) << 8 );
	return getOutput( modulator + phase(), ws() );
}
}
