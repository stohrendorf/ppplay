#ifndef PPP_OPL_DISABLEDCHANNEL_H
#define PPP_OPL_DISABLEDCHANNEL_H
#include "abstractchannel.h"

namespace opl
{
class DisabledChannel : public AbstractChannel
{
	DisabledChannel() : AbstractChannel( 0 ) {
	}
	std::vector<double> getChannelOutput() {
		return getInFourChannels( 0 );
	}
protected:
	void keyOn() { }
	void keyOff() { }
	void updateOperators() { }
};
}

#endif
