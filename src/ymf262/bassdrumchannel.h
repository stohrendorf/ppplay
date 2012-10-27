#ifndef PPP_OPL_BASSDRUMCHANNEL_H
#define PPP_OPL_BASSDRUMCHANNEL_H
#include "channel2op.h"
#include "operator.h"

namespace opl
{
class BassDrumChannel : public Channel2Op
{
	static constexpr int bassDrumChannelBaseAddress = 6;
	static constexpr int op1BaseAddress = 0x10;
	static constexpr int op2BaseAddress = 0x13;

	BassDrumChannel() : Channel2Op( bassDrumChannelBaseAddress, new Operator( op1BaseAddress ), new Operator( op2BaseAddress ) ) {
	}
	
	~BassDrumChannel() {
		delete m_op1;
		delete m_op2;
	}

	std::vector<double> getChannelOutput() {
		// Bass Drum ignores first operator, when it is in series.
		if( m_cnt == 1 ) m_op1->m_ar = 0;
		return Channel2Op::getChannelOutput();
	}

protected:
	// Key ON and OFF are unused in rhythm channels.
	void keyOn() {    }
	void keyOff() {    }
};
}

#endif
