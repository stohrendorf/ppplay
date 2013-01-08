#ifndef PPP_OPL_HIGHHATSNAREDRUMCHANNEL_H
#define PPP_OPL_HIGHHATSNAREDRUMCHANNEL_H
#include "abstractrhythmchannel.h"

namespace opl
{

class Opl3;
class HighHatSnareDrumChannel : public AbstractRhythmChannel
{
public:
	static constexpr int highHatSnareDrumChannelBaseAddress = 7;

	HighHatSnareDrumChannel(Opl3* opl) ;
};
}

#endif
