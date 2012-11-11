#include "topcymbaloperator.h"
#include "opl3.h"

namespace opl
{
int16_t TopCymbalOperator::nextSample( Phase /*modulator*/ )
{
	// The Top Cymbal operator uses his own phase together with the High Hat phase.
	uint16_t highHatPhase = opl()->highHatOperator()->phase().pre();
	uint16_t thisPhase = phase().pre();
	uint16_t phaseBit = (((thisPhase & 0x88) ^ ((thisPhase<<5) & 0x80)) | ((highHatPhase ^ (highHatPhase<<2)) & 0x20)) ? 0x02 : 0x00;
	
	envelopeGenerator()->advance( egt(), am() );

	setPhase((1+phaseBit)<<8);

	uint8_t waveform = ws();
	if( opl()->isNew() ) {
		waveform &= 0x07;
	}
	else {
		waveform &= 0x03;
	}
	
	return getOutput(phase(), waveform);
}
}
