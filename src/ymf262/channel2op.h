#ifndef PPP_OPL_CHANNEL2OP_H
#define PPP_OPL_CHANNEL2OP_H

#include "abstractchannel.h"

namespace opl
{

class Opl3;

class Operator;
class Channel2Op : public AbstractChannel
{
private:
	Operator* m_op1;
	Operator* m_op2;

public:
	Channel2Op( Opl3* opl, int baseAddress, Operator* o1, Operator* o2 )
		: AbstractChannel( opl, baseAddress ), m_op1( o1 ), m_op2( o2 ) {
	}

	std::vector<double> getChannelOutput();
	Operator* op1() const { return m_op1; }
	Operator* op2() const { return m_op2; }

protected:
	void keyOn() ;

	void keyOff() ;

	void updateOperators() ;

//     @Override
//     public String toString() {
//         StringBuffer str = new StringBuffer();
//
//         int f_number = (fnumh<<8)+fnuml;
//
//         str.append(String.format("channelBaseAddress: %d\n", channelBaseAddress));
//         str.append(String.format("f_number: %d, block: %d\n", f_number, block));
//         str.append(String.format("cnt: %d, feedback: %d\n", cnt, fb));
//         str.append(String.format("op1:\n%s", m_op1->toString()));
//         str.append(String.format("op2:\n%s", m_op2->toString()));
//
//         return str.toString();
//     }
};
}

#endif
