#include "bassdrumchannel.h"

namespace opl
{
std::vector< double > BassDrumChannel::nextSample()
{
	// Bass Drum ignores first operator, when it is in series.
	if( cnt() == 1 ) op1()->setAr(0);
	return Channel2Op::nextSample();
}
BassDrumChannel::~BassDrumChannel()
{
	delete op1();
	delete op2();
}
}
