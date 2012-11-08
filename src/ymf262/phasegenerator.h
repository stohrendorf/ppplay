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
	//! @brief Current phase, 10.10 fractional bits
	uint32_t m_phase;
	uint16_t m_fNum;
	uint8_t m_block;
	uint8_t m_mult;
public:
	PhaseGenerator(Opl3* opl) : m_opl(opl), m_phase( 0 ), m_fNum(0), m_block(0), m_mult(0) {
		BOOST_ASSERT( opl != nullptr );
	}

	/**
	 * @post m_fNum<1024 && m_block<8 && m_mult<16
	 */
	void setFrequency( uint16_t f_number, uint8_t block, uint8_t mult );

	// Result: 10 bits
	uint16_t advance( bool vib );
	uint16_t phase() const
	{
		return m_phase>>10;
	}

	/**
	 * @post m_phase==0
	 */
	void keyOn();
};
}

#endif
