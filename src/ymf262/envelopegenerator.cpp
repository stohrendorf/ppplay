#include "envelopegenerator.h"
#include "operator.h"
#include "opl3.h"

namespace opl
{
void EnvelopeGenerator::setActualSustainLevel( uint8_t sl )
{
	sl &= 0x0f;
	if( sl == 0x0F ) {
		m_sl = 0x1f;
	}
	else {
		m_sl = sl;
	}
}

void EnvelopeGenerator::setTotalLevel( uint8_t tl )
{
	// The datasheet states that the TL formula is
	// TL = (24*d5 + 12*d4 + 6*d3 + 3*d2 + 1.5*d1 + 0.75*d0),
	// 10^(0.075*tl) ~= 2^(tl/4)
	m_tl = tl&0x3f;
}

void EnvelopeGenerator::setAttennuation( uint16_t f_number, uint8_t block, uint8_t ksl )
{
	m_fnum = f_number & 0x3ff;
	m_block = block & 0x07;
	m_ksl = ksl & 0x03;

	if( m_ksl == 0 ) {
		m_kslAdd = 0;
		return;
	}

	// 1.5 dB att. for base 2 of oct. 7
	// created by: round(8*log2( 10^(dbMax[msb]/10) ));
	static constexpr int ddArray[16] = {
		0, 24, 32, 37, 40, 43, 45, 47, 48, 50, 51, 52, 53, 54, 55, 56
	};
	int tmp = ddArray[m_fnum >> 6] - 8*(7-m_block);
	if( tmp <= 0 ) {
		m_kslAdd = 0;
		return;
	}
	m_kslAdd = tmp;
	switch( m_ksl ) {
		case 0:
			// done
			break;
		case 1:
			// 3 db
			m_kslAdd <<= 1;
			break;
		case 2:
			// no change, 1.5 dB
			break;
		case 3:
			// 6 dB
			m_kslAdd <<= 2;
			break;
	}
}

void EnvelopeGenerator::setAttackRate( uint8_t attackRate )
{
	m_ar = attackRate & 0x0f;
}

void EnvelopeGenerator::setDecayRate( uint8_t decayRate )
{
	m_dr = decayRate & 0x0f;
}

void EnvelopeGenerator::setReleaseRate( uint8_t releaseRate )
{
	m_rr = releaseRate & 0x0f;
}

uint8_t EnvelopeGenerator::calculateRate( uint8_t rate ) const
{
	if( rate == 0 ) {
		return 0;
	}
	// calculate key scale number
	uint8_t res = ( m_fnum >> ( m_opl->nts() ? 8 : 9 ) ) & 0x1;
	res |= m_block<<1;
	if( !m_ksr ) {
		res >>= 2;
	}
	res += rate << 2;
	// res: max. 6 bits
	return std::min<uint8_t>(63, res);
}

uint16_t EnvelopeGenerator::advance( bool egt, bool am )
{
	uint32_t delta = 0;
	switch(m_stage) {
		case Stage::OFF:
			m_total = Silence;
			m_env = ExactSilence;
			return Silence;
		case Stage::ATTACK:
			delta = calculateRate(m_ar);
			break;
		case Stage::DECAY:
			delta = calculateRate(m_dr);
			break;
		case Stage::SUSTAIN:
			break;
		case Stage::RELEASE:
			delta = calculateRate(m_rr);
			break;
	}
	
	const uint16_t oldEnv = m_env>>EnvelopeShift;
	
	const uint8_t rateHi = delta >>2;
	delta &= 3;
	delta = uint32_t(4|delta)<<rateHi;
	switch( m_stage ) {
		case Stage::OFF:
			// already handled above, but to silence the
			// compiler about a not handled enum value...
			return Silence;
		case Stage::ATTACK:
			if( m_ar == 0 ) {
				break;
			}
			else if( rateHi == 15 ) {
				m_env = 0;
			}
			else {
				uint32_t counter = m_env & ((1<<15)-1);
				counter += delta;
				const uint8_t overflow = (counter>>15);
				uint16_t env = m_env>>15;
				if( overflow!=0 ) {
					env -= (env>>3)*overflow + 1;
					counter &= (1<<15)-1;
				}
				m_env = (env<<15) | counter;
			}
			if( m_env==0 || m_env>ExactSilence ) {
				m_stage = Stage::DECAY;
				// in case of an overflow
				m_env = 0;
				break;
			}
			break;

		case Stage::DECAY:
			if( (m_env>>EnvelopeShift) >= uint32_t(m_sl)<<4 ) {
				m_stage = Stage::SUSTAIN;
				break;
			}
			if(m_dr==0) {
				break;
			}
			
			m_env += delta;
			break;

		case Stage::SUSTAIN:
			if(!egt) {
				m_stage = Stage::RELEASE;
			}
			break;

		case Stage::RELEASE:
			if( m_rr == 0 ) {
				break;
			}
			m_env += delta;
			break;
	}
	
	if( m_env >= ExactSilence ) {
		// too low
		m_env = ExactSilence;
	}
	
	int total = oldEnv + (m_tl<<2) + m_kslAdd;

	if( am ) {
		int amVal = m_opl->tremoloIndex() >> 8;
		if( amVal > 26 ) {
			amVal = ( 2 * 26 ) - amVal;
		}
		BOOST_ASSERT( amVal>=0 && amVal<=26 );
		if( !m_opl->dam() ) {
			amVal >>= 2;
		}
		total += amVal;
	}

	if( total >= Silence ) {
		m_total = Silence;
	}
	else {
		m_total = total;
	}
	return m_total;
}

void EnvelopeGenerator::keyOn()
{
	if( m_stage != Stage::SUSTAIN ) {
		m_stage = Stage::ATTACK;
	}
}

void EnvelopeGenerator::keyOff()
{
	if( m_stage != Stage::OFF ) {
		m_stage = Stage::RELEASE;
	}
}

}

