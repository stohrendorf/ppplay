#ifndef PPP_OPL_DISABLEDCHANNEL_H
#define PPP_OPL_DISABLEDCHANNEL_H
#include "abstractchannel.h"

namespace opl
{

class Opl3;
class DisabledChannel : public AbstractChannel
{
public:
	DisabledChannel(Opl3* opl) : AbstractChannel( opl, 0 ) {
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
