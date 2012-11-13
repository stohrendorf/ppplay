#include "phasegenerator.h"
#include "opl3.h"

namespace opl
{
constexpr uint8_t multTable[16] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};

void PhaseGenerator::setFrequency( uint16_t f_number, uint8_t block, uint8_t mult )
{
	m_fNum = f_number&0x3ff;
	m_block = block&0x07;
	m_mult = mult&0x0f;
}

Phase PhaseGenerator::advance( bool vib )
{
	/*
	 * According to the YMF262 manual:
	 * FNUM = (frq<<(20-BLOCK)) / Opl3::SampleRate
	 * -> frq = (FNUM*Opl3::SampleRate) >> (20-BLOCK)
	 * 
	 * The sine wave length is 1<<10, so if sinFrq is the number of sine samples per second:
	 * sinFrq = (FNUM*Opl3::SampleRate) >> (10-BLOCK)
	 * Thus, the number of sine samples per output sample is:
	 * sinOutFrq = FNUM >> (10-BLOCK)
	 *           = (FNUM<<BLOCK)>>10
	 */
	uint32_t inc = m_fNum;
	if (vib)
	{
		uint16_t delta = m_fNum>>7;
		if( ((m_opl->vibratoIndex()>>10) & 3) == 3 ) {
			delta >>= 1;
		}
		if( !m_opl->dvb() ) {
			// 14 -> 7 percent
			delta >>= 1;
		}
		if( (m_opl->vibratoIndex()>>12)&1 ) {
			inc -= delta;
		}
		else {
			inc += delta;
		}
	}
	inc <<= m_block;
	inc >>= 1;

	static constexpr int multTable[16] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};
	m_phase.fullAdd((inc * multTable[m_mult])>>1);
	return m_phase;
}

void PhaseGenerator::keyOn()
{
	m_phase = 0;
}
}
