#include "highhatoperator.h"
#include "opl3.h"

namespace opl
{
int16_t HighHatOperator::nextSample( int16_t modulator )
{
	uint16_t cymbalPhase = opl()->topCymbalOperator()->phase();
	uint16_t phasebit = ( ( ( phase() & 0x88 ) ^( ( phase() << 5 ) & 0x80 ) ) | ( ( cymbalPhase ^( cymbalPhase << 2 ) ) & 0x20 ) ) ? 0x02 : 0x00;
	uint16_t noisebit = rand() & 1;
	setPhase( ( phasebit << 8 ) | ( 0x34 << ( phasebit ^( noisebit << 1 ) ) ) );
	envelopeGenerator()->advance( egt(), am() );
	return getOutput( modulator + phase(), ws() );
}
}
