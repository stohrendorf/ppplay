#include "tomtomtopcymbalchannel.h"
#include "opl3.h"

namespace opl
{
TomTomTopCymbalChannel::TomTomTopCymbalChannel( Opl3* opl ) : AbstractRhythmChannel( opl, tomTomTopCymbalChannelBaseAddress, opl->tomTomOperator(), opl->topCymbalOperator() )
{
}
}

