#include "phasegenerator.h"
#include "opl3.h"

namespace opl
{
void PhaseGenerator::setFrequency( int f_number, int block, int mult )
{
	// This frequency formula is derived from the following equation:
	// f_number = baseFrequency * pow(2,19) / sampleRate / pow(2,block-1);
	double baseFrequency =
		f_number * std::pow( 2, block - 1 ) * Opl3::sampleRate / std::pow( 2, 19 );
	double operatorFrequency = baseFrequency * Operator::multTable[mult];

	// phase goes from 0 to 1 at
	// period = (1/frequency) seconds ->
	// Samples in each period is (1/frequency)*sampleRate =
	// = sampleRate/frequency ->
	// So the increment in each sample, to go from 0 to 1, is:
	// increment = (1-0) / samples in the period ->
	// increment = 1 / (OPL3Data.sampleRate/operatorFrequency) ->
	m_phaseIncrement = operatorFrequency / Opl3::sampleRate;
}
double PhaseGenerator::getPhase( int vib )
{
	if( vib == 1 )
		// phaseIncrement = (operatorFrequency * vibrato) / sampleRate
		m_phase += m_phaseIncrement * Opl3::vibratoTable[m_opl->dvb()][m_opl->vibratoIndex()];
	else
		// phaseIncrement = operatorFrequency / sampleRate
		m_phase += m_phaseIncrement;
	m_phase = std::fmod( m_phase, 1 );
	return m_phase;
}
void PhaseGenerator::keyOn()
{
	m_phase = 0;
}
}

