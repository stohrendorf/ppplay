#include "highhatsnaredrumchannel.h"
#include "opl3.h"

namespace opl
{
HighHatSnareDrumChannel::HighHatSnareDrumChannel( Opl3* opl ) : AbstractRhythmChannel( opl, highHatSnareDrumChannelBaseAddress, opl->highHatOperator(), opl->snareDrumOperator() )
{
}
}

