#include "snaredrumoperator.h"
#include "opl3.h"
namespace opl
{
double SnareDrumOperator::getOperatorOutput( int modulator )
{
	if( envelopeGenerator()->isOff() )
		return 0;

	setEnvelope( envelopeGenerator()->getEnvelope( egt(), am() ) );

	setPhase( opl()->highHatOperator()->phase() * 2 );

	double operatorOutput = getOutput( modulator, phase(), ws() );

	if( std::fabs(operatorOutput / envelope()) != 1 ) {
		double noise = rand() * (511.0-envelope()) / 511 / RAND_MAX;
		operatorOutput = std::fabs(noise);
	}

	return operatorOutput * 2;
}
}
