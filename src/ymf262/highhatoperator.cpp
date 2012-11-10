#include "highhatoperator.h"
#include "opl3.h"

namespace opl
{
int16_t HighHatOperator::nextSample( uint32_t modulator )
{
	uint16_t cymbalPhase = opl()->topCymbalOperator()->phase() >> 10;
	uint16_t thisPhase = phase()>>10;
	uint16_t phasebit = ( ( ( thisPhase & 0x88 ) ^( ( thisPhase << 5 ) & 0x80 ) ) | ( ( cymbalPhase ^( cymbalPhase << 2 ) ) & 0x20 ) ) ? 0x02 : 0x00;
	uint16_t noisebit = rand() & 1;
	setPhase( (( phasebit << 8 ) | ( 0x34 << ( phasebit ^( noisebit << 1 ) ) ))<<10 );
	envelopeGenerator()->advance( egt(), am() );
	return getOutput( modulator + phase(), ws() );
}
}
