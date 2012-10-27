#ifndef PPP_OPL_PHASEGENERATOR_H
#define PPP_OPL_PHASEGENERATOR_H
#include <cmath>

namespace opl
{
class PhaseGenerator
{
	double m_phase;
	double m_phaseIncrement;

	PhaseGenerator() : m_phase( 0 ), m_phaseIncrement( 0 ) {
	}

	void setFrequency( int f_number, int block, int mult ) {
		// This frequency formula is derived from the following equation:
		// f_number = baseFrequency * pow(2,19) / sampleRate / pow(2,block-1);
		double baseFrequency =
			f_number * std::pow( 2, block - 1 ) * OPL3Data.sampleRate / std::pow( 2, 19 );
		double operatorFrequency = baseFrequency * OperatorData.multTable[mult];

		// phase goes from 0 to 1 at
		// period = (1/frequency) seconds ->
		// Samples in each period is (1/frequency)*sampleRate =
		// = sampleRate/frequency ->
		// So the increment in each sample, to go from 0 to 1, is:
		// increment = (1-0) / samples in the period ->
		// increment = 1 / (OPL3Data.sampleRate/operatorFrequency) ->
		m_phaseIncrement = operatorFrequency / OPL3Data.sampleRate;
	}

	double getPhase( int vib ) {
		if( vib == 1 )
			// phaseIncrement = (operatorFrequency * vibrato) / sampleRate
			m_phase += m_phaseIncrement * OPL3Data.vibratoTable[OPL3.dvb][OPL3.vibratoIndex];
		else
			// phaseIncrement = operatorFrequency / sampleRate
			m_phase += m_phaseIncrement;
		m_phase = std::fmod( m_phase, 1 );
		return m_phase;
	}

	void keyOn() {
		m_phase = 0;
	}

//     @Override
//     public String toString() {
//          return String.format("Operator frequency: %f Hz.\n", OPL3Data.sampleRate*phaseIncrement);
//     }
};
}

#endif
