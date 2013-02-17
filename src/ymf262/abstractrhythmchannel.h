#ifndef PPP_OPL_ABSTRACTRHYTHMCHANNEL_H
#define PPP_OPL_ABSTRACTRHYTHMCHANNEL_H

#include "channel2op.h"

namespace opl
{

class Opl3;
class AbstractRhythmChannel : public Channel2Op
{
public:
	AbstractRhythmChannel( Opl3* opl, int baseAddress, Operator* o1, Operator* o2 )
		: Channel2Op( opl, baseAddress, o1, o2 ) {
	}

	void nextSample(std::array<int16_t,4>* dest);

protected:
	void keyOn() { }
	void keyOff() { }
};
}

#endif
