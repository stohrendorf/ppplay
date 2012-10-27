#ifndef PPP_OPL_HIGHHATSNAREDRUMCHANNEL_H
#define PPP_OPL_HIGHHATSNAREDRUMCHANNEL_H
#include "abstractrhythmchannel.h"

namespace opl
{
class HighHatSnareDrumChannel : public AbstractRhythmChannel
{
	static constexpr int highHatSnareDrumChannelBaseAddress = 7;

	HighHatSnareDrumChannel() : AbstractRhythmChannel( highHatSnareDrumChannelBaseAddress, OPL3.highHatOperator, OPL3.snareDrumOperator ) {
	}
};
}

#endif
