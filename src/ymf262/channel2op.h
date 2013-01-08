#ifndef PPP_OPL_CHANNEL2OP_H
#define PPP_OPL_CHANNEL2OP_H

#include <stuff/utils.h>

#include "abstractchannel.h"
#include <memory>

namespace opl
{

class Opl3;

class Operator;
class Channel2Op : public AbstractChannel
{
	DISABLE_COPY(Channel2Op)
private:
	Operator* m_op1;
	Operator* m_op2;

	static light4cxx::Logger* logger();
public:
	typedef std::shared_ptr<Channel2Op> Ptr;
	
	Channel2Op( Opl3* opl, int baseAddress, Operator* o1, Operator* o2 )
		: AbstractChannel( opl, baseAddress ), m_op1( o1 ), m_op2( o2 ) {
	}

	std::vector<int16_t> nextSample();
	Operator* op1() const { return m_op1; }
	Operator* op2() const { return m_op2; }

protected:
	void keyOn();
	void keyOff();
	void updateOperators();
};
}

#endif
