#include "snaredrumoperator.h"
#include "opl3.h"
namespace opl
{
double SnareDrumOperator::getOperatorOutput( double modulator )
{
	if( envelopeGenerator()->isOff() ) return 0;

	double envelopeInDB = envelopeGenerator()->getEnvelope( egt(), am() );
	setEnvelope( std::pow( 10, envelopeInDB / 10.0 ) );

	// If it is in OPL2 mode, use first four waveforms only:
	int waveIndex = ws() & ( ( opl()->isNew() << 2 ) + 3 );
	const double* waveform = Operator::waveforms[waveIndex];

	setPhase( opl()->highHatOperator()->phase() * 2 );

	double operatorOutput = getOutput( modulator, phase(), waveform );

	double noise = rand() * envelope() / RAND_MAX;

	if( operatorOutput / envelope() != 1 && operatorOutput / envelope() != -1 ) {
		if( operatorOutput > 0 )  operatorOutput = noise;
		else if( operatorOutput < 0 ) operatorOutput = -noise;
		else operatorOutput = 0;
	}

	return operatorOutput * 2;
}
}

