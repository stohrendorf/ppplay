#include "abstractrhythmchannel.h"
#include "operator.h"

namespace opl
{
std::vector< int16_t > AbstractRhythmChannel::nextSample()
{
	// Note that, different from the common channel,
	// we do not check to see if the Operator's envelopes are Off.
	// Instead, we always do the calculations,
	// to update the publicly available phase.
	int16_t op1Output = op1()->nextSample( Operator::noModulator );
	int16_t op2Output = op2()->nextSample( Operator::noModulator );
	int16_t channelOutput = ( op1Output + op2Output ) / 2;

	return getInFourChannels( channelOutput );
}
}
