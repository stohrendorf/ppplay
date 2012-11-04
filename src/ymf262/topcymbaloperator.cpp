#include "topcymbaloperator.h"
#include "opl3.h"

namespace opl
{
double TopCymbalOperator::getOperatorOutput( int modulator )
{
	int highHatOperatorPhase =
		opl()->highHatOperator()->phase() * Operator::multTable[opl()->highHatOperator()->mult()];
	// The Top Cymbal operator uses his own phase together with the High Hat phase.
	return getOperatorOutput( modulator, highHatOperatorPhase );
}
double TopCymbalOperator::getOperatorOutput( int /*modulator*/, int externalPhase )
{
	setEnvelope( envelopeGenerator()->getEnvelope( egt(), am() ) );

	setPhase( phaseGenerator()->getPhase( vib() ) );

	// Empirically tested multiplied phase for the Top Cymbal:
	int carrierPhase = ( 8 * phase() ) & 0x3ff;
	int modulatorPhase = externalPhase&0x3ff;
	
	uint8_t waveform = ws();
	if( opl()->isNew() ) {
		waveform &= 0x07;
	}
	else {
		waveform &= 0x03;
	}
	
	double modulatorOutput = getOutput( Operator::noModulator, modulatorPhase, waveform );
	double carrierOutput = getOutput( modulatorOutput*1024, carrierPhase, waveform );

	int cycles = 4;
	if( std::fmod( carrierPhase * cycles, cycles ) > 0.1 ) carrierOutput = 0;

	return carrierOutput * 2;
}
}
