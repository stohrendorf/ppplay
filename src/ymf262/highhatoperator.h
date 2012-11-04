#ifndef PPP_OPL_HIGHHATOPERATOR_H
#define PPP_OPL_HIGHHATOPERATOR_H

#include <cstdlib>

#include "topcymbaloperator.h"

namespace opl
{

class Opl3;
class HighHatOperator : public TopCymbalOperator
{
public:
    static constexpr int highHatOperatorBaseAddress = 0x11;     
    
    HighHatOperator(Opl3* opl) : TopCymbalOperator(opl, highHatOperatorBaseAddress) {
    }
    
    double getOperatorOutput(int modulator) ;
};
}

#endif
