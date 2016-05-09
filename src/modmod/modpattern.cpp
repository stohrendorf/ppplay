#include "modpattern.h"
#include "modcell.h"

#include "stuff/utils.h"

#include <boost/assert.hpp>

namespace ppp
{
namespace mod
{
ModPattern::ModPattern() : Field<ModCell>()
{
}

bool ModPattern::load(Stream* str, uint8_t numChans)
{
    reset(numChans, 64);
    for(int i = 0; i < 64; i++)
    {
        for(size_t j = 0; j < numChans; j++)
        {
            if(!at(j, i).load(str))
            {
                return false;
            }
        }
    }
    return *str;
}
}
}