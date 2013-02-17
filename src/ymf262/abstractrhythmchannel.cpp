#include "abstractrhythmchannel.h"
#include "operator.h"

namespace opl
{
void AbstractRhythmChannel::nextSample( std::array< int16_t, 4 >* dest )
{
	// Note that, different from the common channel,
	// we do not check to see if the Operator's envelopes are Off.
	// Instead, we always do the calculations,
	// to update the publicly available phase.
	getInFourChannels( dest, op1()->nextSample( Operator::noModulator ) + op2()->nextSample( Operator::noModulator ) );
}
}
