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
	void nextSample(std::array<int16_t,4>* dest) {
		getInFourChannels( dest, 0 );
	}

	void keyOn() { }
	void keyOff() { }
	void updateOperators() { }
};
}

#endif
