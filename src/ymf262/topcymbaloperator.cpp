#include "topcymbaloperator.h"
#include "opl3.h"

namespace opl
{
double TopCymbalOperator::getOperatorOutput( double modulator )
{
	double highHatOperatorPhase =
		opl()->highHatOperator()->phase() * Operator::multTable[opl()->highHatOperator()->mult()];
	// The Top Cymbal operator uses his own phase together with the High Hat phase.
	return getOperatorOutput( modulator, highHatOperatorPhase );
}
double TopCymbalOperator::getOperatorOutput( double modulator, double externalPhase )
{
	double envelopeInDB = envelopeGenerator()->getEnvelope( egt(), am() );
	setEnvelope( std::pow( 10, envelopeInDB / 10.0 ) );

	setPhase( phaseGenerator()->getPhase( vib() ) );

	int waveIndex = ws() & ( ( opl()->isNew() << 2 ) + 3 );
	const double* waveform = Operator::waveforms[waveIndex];

	// Empirically tested multiplied phase for the Top Cymbal:
	double carrierPhase = std::fmod( 8 * phase(), 1 );
	double modulatorPhase = externalPhase;
	double modulatorOutput = getOutput( Operator::noModulator, modulatorPhase, waveform );
	double carrierOutput = getOutput( modulatorOutput, carrierPhase, waveform );

	int cycles = 4;
	if( std::fmod( carrierPhase * cycles, cycles ) > 0.1 ) carrierOutput = 0;

	return carrierOutput * 2;
}
}

