#include "topcymbaloperator.h"
#include "opl3.h"

namespace opl
{
double TopCymbalOperator::getOperatorOutput( int modulator )
{
	// The Top Cymbal operator uses his own phase together with the High Hat phase.
	uint16_t highHatPhase = opl()->highHatOperator()->phase();
	uint16_t phaseBit = (((phase() & 0x88) ^ ((phase()<<5) & 0x80)) | ((highHatPhase ^ (highHatPhase<<2)) & 0x20)) ? 0x02 : 0x00;
	
	setEnvelope( envelopeGenerator()->getEnvelope( egt(), am() ) );

	setPhase((1+phaseBit)<<8);

	uint8_t waveform = ws();
	if( opl()->isNew() ) {
		waveform &= 0x07;
	}
	else {
		waveform &= 0x03;
	}
	
	return getOutput(0, phase(), waveform);
}
}
