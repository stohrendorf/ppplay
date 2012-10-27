#ifndef PPP_OPL_TOPCYMBALOPERATOR_H
#define PPP_OPL_TOPCYMBALOPERATOR_H
#include "operator.h"

namespace opl
{
class TopCymbalOperator : public Operator
{
    static constexpr int topCymbalOperatorBaseAddress = 0x15;
    
    TopCymbalOperator(int baseAddress = topCymbalOperatorBaseAddress) : Operator(baseAddress) {
    }
    
    double getOperatorOutput(double modulator) {
        double highHatOperatorPhase = 
            OPL3.highHatOperator.phase * OperatorData.multTable[OPL3.highHatOperator.mult];
        // The Top Cymbal operator uses his own phase together with the High Hat phase.
        return getOperatorOutput(modulator, highHatOperatorPhase);
    }

protected:
    // This method is used here with the HighHatOperator phase
    // as the externalPhase. 
    // Conversely, this method is also used through inheritance by the HighHatOperator, 
    // now with the TopCymbalOperator phase as the externalPhase.
    double getOperatorOutput(double modulator, double externalPhase) {
        double envelopeInDB = m_envelopeGenerator.getEnvelope(m_egt, m_am);
        m_envelope = std::pow(10, envelopeInDB/10.0);
        
        m_phase = m_phaseGenerator.getPhase(m_vib);
        
        int waveIndex = m_ws & ((OPL3._new<<2) + 3); 
        const double* waveform = OperatorData.waveforms[m_waveIndex];
        
        // Empirically tested multiplied phase for the Top Cymbal:
        double carrierPhase = std::fmod(8 * m_phase, 1);
        double modulatorPhase = externalPhase;
        double modulatorOutput = getOutput(Operator.noModulator,modulatorPhase, waveform);
        double carrierOutput = getOutput(modulatorOutput,carrierPhase, waveform);
        
        int cycles = 4; 
        if( (carrierPhase*cycles)%cycles > 0.1) carrierOutput = 0;
        
        return carrierOutput*2;  
    }    
};
}

#endif
