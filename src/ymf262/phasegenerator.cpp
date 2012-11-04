#include "phasegenerator.h"
#include "opl3.h"

namespace opl
{
// block: 0..7, f_number: 1..1023, mult: 0..15
void PhaseGenerator::setFrequency( uint16_t f_number, uint8_t block, uint8_t mult )
{
	m_fNum = f_number&0x3ff;
	m_block = block&0x07;
	m_mult = mult&0x0f;
}
int PhaseGenerator::getPhase( bool vib )
{
	uint16_t inc = m_fNum;
	if (vib)
	{
		uint16_t delta = m_fNum>>7;
		if( ((m_opl->vibratoIndex()>>10) & 3) == 3 ) {
			delta >>= 1;
		}
		if( !m_opl->dvb() ) {
			delta >>= 1;
		}
		if( (m_opl->vibratoIndex()>>12)&1 ) {
			inc -= delta;
		}
		else {
			inc += delta;
		}
	}

	static constexpr int multTable[16] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};
	m_phase += ((inc << m_block) * multTable[m_mult]) >> 1;
	m_phase &= 0x1fffff; // 21 bits
	return m_phase >> 11;
}
void PhaseGenerator::keyOn()
{
	m_phase = 0;
}
}
