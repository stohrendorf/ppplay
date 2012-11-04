#include "abstractrhythmchannel.h"
#include "operator.h"

namespace opl
{
std::vector< double > AbstractRhythmChannel::nextSample()
{
	double channelOutput = 0, op1Output = 0, op2Output = 0;
	std::vector<double> output;

	// Note that, different from the common channel,
	// we do not check to see if the Operator's envelopes are Off.
	// Instead, we always do the calculations,
	// to update the publicly available phase.
	op1Output = op1()->nextSample( Operator::noModulator );
	op2Output = op2()->nextSample( Operator::noModulator );
	channelOutput = ( op1Output + op2Output ) / 2;

	output = getInFourChannels( channelOutput );
	return output;
}
}

