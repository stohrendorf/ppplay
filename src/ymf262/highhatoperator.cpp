#include "highhatoperator.h"
#include "opl3.h"

namespace opl
{
Fractional9 HighHatOperator::nextSample( Fractional9 modulator )
{
	uint16_t cymbalPhase = opl()->topCymbalOperator()->phase().trunc();
	uint16_t thisPhase = phase().trunc();
	uint16_t phasebit = ( ( ( thisPhase & 0x88 ) ^( ( thisPhase << 5 ) & 0x80 ) ) | ( ( cymbalPhase ^( cymbalPhase << 2 ) ) & 0x20 ) ) ? 0x02 : 0x00;
	uint16_t noisebit = rand() & 1;
	setPhase( (( phasebit << 8 ) | ( 0x34 << ( phasebit ^( noisebit << 1 ) ) )) );
	envelopeGenerator()->advance( egt(), am() );
	return getOutput( modulator + phase(), ws() );
}
}
