#ifndef PPP_OPL_PHASEGENERATOR_H
#define PPP_OPL_PHASEGENERATOR_H

#include <cmath>

namespace opl
{

class Opl3;
class PhaseGenerator
{
	double m_phase;
	double m_phaseIncrement;
	Opl3* m_opl;
public:
	constexpr PhaseGenerator(Opl3* opl) : m_opl(opl), m_phase( 0 ), m_phaseIncrement( 0 ) {
	}

	void setFrequency( int f_number, int block, int mult ) ;

	double getPhase( int vib );

	void keyOn() ;

//     @Override
//     public String toString() {
//          return String.format("Operator frequency: %f Hz.\n", OPL3Data.sampleRate*phaseIncrement);
//     }
};
}

#endif
