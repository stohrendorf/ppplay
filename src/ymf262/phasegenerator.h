#ifndef PPP_OPL_PHASEGENERATOR_H
#define PPP_OPL_PHASEGENERATOR_H

#include <cmath>
#include <cstdint>

#include <boost/assert.hpp>

namespace opl
{

class Opl3;

/**
 * @class PhaseGenerator
 * @brief OPL 3 Phase generator
 */
class PhaseGenerator
{
	//! @brief Owning chip
	Opl3* m_opl;
	//! @brief Current phase, 21 bits
	uint32_t m_phase;
	uint16_t m_fNum;
	uint8_t m_block;
	uint8_t m_mult;
public:
	PhaseGenerator(Opl3* opl) : m_opl(opl), m_phase( 0 ), m_fNum(0), m_block(0), m_mult(0) {
		BOOST_ASSERT( opl != nullptr );
	}

	void setFrequency( uint16_t f_number, uint8_t block, uint8_t mult );

	// Result: 10 bits
	int getPhase( bool vib );

	void keyOn();

//     @Override
//     public String toString() {
//          return String.format("Operator frequency: %f Hz.\n", OPL3Data.sampleRate*phaseIncrement);
//     }
};
}

#endif
