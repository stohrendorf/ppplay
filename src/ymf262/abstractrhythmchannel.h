#ifndef PPP_OPL_ABSTRACTRHYTHMCHANNEL_H
#define PPP_OPL_ABSTRACTRHYTHMCHANNEL_H

#include "channel2op.h"

namespace opl
{
class AbstractRhythmChannel : public Channel2Op
{
public:
	AbstractRhythmChannel( int baseAddress, Operator* o1, Operator* o2 )
		: Channel2Op( baseAddress, o1, o2 ) {
	}

	std::vector<double> getChannelOutput() {
		double channelOutput = 0, op1Output = 0, op2Output = 0;
		std::vector<double> output;

		// Note that, different from the common channel,
		// we do not check to see if the Operator's envelopes are Off.
		// Instead, we always do the calculations,
		// to update the publicly available phase.
		op1Output = m_op1->getOperatorOutput( Operator.noModulator );
		op2Output = m_op2->getOperatorOutput( Operator.noModulator );
		channelOutput = ( op1Output + op2Output ) / 2;

		output = getInFourChannels( channelOutput );
		return output;
	};

protected:
	void keyOn() { }
	void keyOff() { }
};
}

#endif
