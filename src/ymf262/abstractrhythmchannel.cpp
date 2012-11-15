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
	return getInFourChannels( op1()->nextSample( Operator::noModulator ) + op2()->nextSample( Operator::noModulator ) );
}
}
