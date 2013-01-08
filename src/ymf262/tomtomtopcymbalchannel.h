#ifndef PPP_OPL_TOMTOMTOPCYMBALCHANNEL_H
#define PPP_OPL_TOMTOMTOPCYMBALCHANNEL_H
#include "abstractrhythmchannel.h"

namespace opl
{

class Opl3;
class TomTomTopCymbalChannel : public AbstractRhythmChannel
{
public:
	static constexpr int tomTomTopCymbalChannelBaseAddress = 8;

	TomTomTopCymbalChannel(Opl3* opl) ;
};
}

#endif
