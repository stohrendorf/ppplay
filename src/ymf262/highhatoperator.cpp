#include "highhatoperator.h"
#include "opl3.h"

namespace opl
{
double HighHatOperator::getOperatorOutput( double modulator )
{
	double topCymbalOperatorPhase =
		opl()->topCymbalOperator()->phase() * Operator::multTable[opl()->topCymbalOperator()->mult()];
	// The sound output from the High Hat resembles the one from
	// Top Cymbal, so we use the parent method and modifies his output
	// accordingly afterwards.
	double operatorOutput = TopCymbalOperator::getOperatorOutput( modulator, topCymbalOperatorPhase );
	if( operatorOutput == 0 ) operatorOutput = rand() * envelope() / RAND_MAX;
	return operatorOutput;
}
}
