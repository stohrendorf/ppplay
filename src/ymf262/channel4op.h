#ifndef PPP_OPL_CHANNEL4OP_H
#define PPP_OPL_CHANNEL4OP_H

#include <stuff/utils.h>

#include "abstractchannel.h"
#include <memory>

namespace opl
{

class Opl3;

class Operator;
class Channel4Op : public AbstractChannel
{
	DISABLE_COPY(Channel4Op)
private:
	Operator* m_op1;
	Operator* m_op2;
	Operator* m_op3;
	Operator* m_op4;
	static light4cxx::Logger* logger();

public:
	typedef std::shared_ptr<Channel4Op> Ptr;
	
	Channel4Op( Opl3* opl, int baseAddress, Operator* o1, Operator* o2, Operator* o3, Operator* o4 )
		: AbstractChannel( opl, baseAddress ), m_op1( o1 ), m_op2( o2 ), m_op3( o3 ), m_op4( o4 ) {
	}

	std::vector<int16_t> nextSample() ;

protected:
	void keyOn();
	void keyOff();
	void updateOperators();
};
}

#endif
