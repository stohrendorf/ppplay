#include "bassdrumchannel.h"

namespace opl
{
void BassDrumChannel::nextSample( std::array< int16_t, 4 >* dest )
{
	// Bass Drum ignores first operator, when it is in series.
	if( cnt() ) {
		op1()->setAr(0);
	}
	Channel2Op::nextSample(dest);
}
BassDrumChannel::~BassDrumChannel()
{
	delete op1();
	delete op2();
}
}
