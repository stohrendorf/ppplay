#include "phasegenerator.h"
#include "opl3.h"

namespace
{
// [mult:0..15][oct:0..7][fnum>>7:0..7]
int vibDepLookup[16][8][4];
int initVibDepLookup()
{
	for( int mult = 0; mult < 16; mult++ ) {
		for( int oct = 0; oct < 8; oct++ ) {
			for( int fnum = 0; fnum < 8; fnum++ ) {
				vibDepLookup[mult][oct][fnum] = std::pow( 2, oct + fnum + 0.14 / 12 ) * opl::Operator::multTable[mult];
			}
		}
	}
	return 1;
}
}

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

	m_phase += (inc << m_block) * Operator::multTable[m_mult];
	m_phase &= 0x1fffff; // 21 bits
	return m_phase >> 11;
}
void PhaseGenerator::keyOn()
{
	m_phase = 0;
}
}
