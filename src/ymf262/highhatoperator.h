#ifndef PPP_OPL_HIGHHATOPERATOR_H
#define PPP_OPL_HIGHHATOPERATOR_H

#include <cstdlib>

#include "topcymbaloperator.h"

namespace opl
{
class HighHatOperator : public TopCymbalOperator
{
    static constexpr int highHatOperatorBaseAddress = 0x11;     
    
    HighHatOperator() : TopCymbalOperator(highHatOperatorBaseAddress) {
    }
    
    double getOperatorOutput(double modulator) {
        double topCymbalOperatorPhase = 
            OPL3.topCymbalOperator.phase * OperatorData.multTable[OPL3.topCymbalOperator.mult];
        // The sound output from the High Hat resembles the one from
        // Top Cymbal, so we use the parent method and modifies his output
        // accordingly afterwards.
        double operatorOutput = TopCymbalOperator::getOperatorOutput(modulator, topCymbalOperatorPhase);
        if(operatorOutput == 0) operatorOutput = rand()*m_envelope/RAND_MAX;
        return operatorOutput;
    }
};
}

#endif
