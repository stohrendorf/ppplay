#ifndef PPP_OPL_CHANNEL4OP_H
#define PPP_OPL_CHANNEL4OP_H
#include "abstractchannel.h"

namespace opl
{

class Opl3;

class Operator;
class Channel4Op : public AbstractChannel
{
	Operator* m_op1;
	Operator* m_op2;
	Operator* m_op3;
	Operator* m_op4;

public:
	Channel4Op( Opl3* opl, int baseAddress, Operator* o1, Operator* o2, Operator* o3, Operator* o4 )
		: AbstractChannel( opl, baseAddress ), m_op1( o1 ), m_op2( o2 ), m_op3( o3 ), m_op4( o4 ) {
	}

	std::vector<double> getChannelOutput() ;

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
//         str.append(String.format("op3:\n%s", m_op3->toString()));
//         str.append(String.format("op4:\n%s", m_op4->toString()));
//
//         return str.toString();
//     }
};
}

#endif
