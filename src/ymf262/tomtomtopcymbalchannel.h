#ifndef PPP_OPL_TOMTOMTOPCYMBALCHANNEL_H
#define PPP_OPL_TOMTOMTOPCYMBALCHANNEL_H
#include "abstractrhythmchannel.h"

namespace opl
{
class TomTomTopCymbalChannel : public AbstractRhythmChannel
{
	static constexpr int tomTomTopCymbalChannelBaseAddress = 8;

	TomTomTopCymbalChannel() : AbstractRhythmChannel( tomTomTopCymbalChannelBaseAddress, OPL3.tomTomOperator, OPL3.topCymbalOperator ) {
	}
};
}

#endif
