#include "channelstate.h"
#include <stream/abstractarchive.h>

namespace ppp
{
AbstractArchive& ChannelState::serialize(AbstractArchive* archive)
{
    *archive
        % active
        % noteTriggered
        % instrument
        % instrumentName
        % note
        % fx
        % fxParam
        % fxDesc
        % panning
        % volume
        % cell;
    return *archive;
}
}