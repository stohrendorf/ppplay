#ifndef PPP_OPL_BASSDRUMCHANNEL_H
#define PPP_OPL_BASSDRUMCHANNEL_H
#include "channel2op.h"
#include "operator.h"

namespace opl
{

class Opl3;
class BassDrumChannel : public Channel2Op
{
public:
	static constexpr int bassDrumChannelBaseAddress = 6;
	static constexpr int op1BaseAddress = 0x10;
	static constexpr int op2BaseAddress = 0x13;

	BassDrumChannel(Opl3* opl) : Channel2Op( opl, bassDrumChannelBaseAddress, new Operator( opl, op1BaseAddress ), new Operator( opl, op2BaseAddress ) ) {
	}
	
	~BassDrumChannel() ;

	std::vector<double> nextSample() ;

protected:
	// Key ON and OFF are unused in rhythm channels.
	void keyOn() {    }
	void keyOff() {    }
};
}

#endif
