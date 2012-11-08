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
	static constexpr int ddArray[] = {
		0, 24, 32, 37, 40, 43, 45, 47, 48, 50, 51, 52, 53, 54, 55, 56
	};
	int tmp = ddArray[m_fnum >> 6] - 16 * m_block;
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

uint16_t EnvelopeGenerator::advance( bool egt, bool am )
{
	// http://forums.submarine.org.uk/phpBB/viewtopic.php?f=9&t=16&start=20
	static constexpr uint8_t stepTable[16] = {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1};
	uint8_t rateHi = ( m_fnum >> ( m_opl->nts() ? 8 : 9 ) ) & 0x1;
	rateHi |= m_block<<1;
	if( m_ksr ) {
		rateHi >>= 2;
	}
	if( m_stage == Stage::ATTACK ) {
		rateHi += m_ar<<2;
	}
	else if( m_stage == Stage::DECAY ) {
		rateHi += m_dr<<2;
	}
	else if( m_stage == Stage::RELEASE ) {
		rateHi += m_rr<<2;
	}
	if( rateHi > 60 ) {
		rateHi = 60;
	}
	const uint8_t rateLo = (rateHi & 0x03) << 2;
	rateHi >>= 2;

	switch( m_stage ) {
		case Stage::OFF:
			return Silence;
		case Stage::ATTACK:
			if( m_env==0 ) {
				m_stage = Stage::DECAY;
				break;
			}
			if( m_ar == 0 ) {
				break;
			}
			if( !( m_clock & ( 0x0FFF >> rateHi ) ) ) {
				if( rateHi == 15 ) {
					m_env = 0;
				}
				else if( rateHi > 12 ) {
					int stepState = (m_clock & 0x07) >> 1;
					m_env -= ( m_env >> ( 16 - rateHi - stepTable[rateLo + stepState ] ) ) + 1;
				}
				else {
					int stepState = ( m_clock >> ( 12 - rateHi ) ) & 0x07;
					if( ( stepState & 0x01 ) || stepTable[ rateLo + ( stepState >> 1 )] )
						m_env -= ( m_env >> 3 ) + 1;
				}
			}
			break;

		case Stage::DECAY:
			if( m_env >= ( 1 << ( m_sl + 3 ) ) ) {
				m_stage = egt ? Stage::SUSTAIN : Stage::RELEASE;
				break;
			}
			if(m_dr==0) {
				break;
			}

			if( !( m_clock & ( 0x0FFF >> rateHi ) ) ) {
				if( rateHi > 12 ) {
					uint8_t stepState = (m_clock & 0x07) >> 1;
					m_env += ( 1 << ( rateHi + stepTable[ rateLo + stepState ] - 13 ) );
				}
				else {
					int stepState = ( m_clock >> ( 12 - rateHi ) ) & 0x07;
					if( ( stepState & 0x01 ) || stepTable[ rateLo + ( stepState >> 1 )] )
						m_env++;
				}
			}
			break;

		case Stage::SUSTAIN:
			break;

		case Stage::RELEASE:
			if( m_rr == 0 ) {
				break;
			}
			if( !( m_clock & ( 0x0FFF >> rateHi ) ) ) {
				if( rateHi > 12 ) {
					int stepState = (m_clock & 0x07) >> 1;
					m_env += ( 1 << ( rateHi - stepTable[rateLo + stepState ] - 13 ) );
				}
				else {
					int stepState = ( m_clock >> ( 12 - rateHi ) ) & 0x07;
					if( ( stepState & 0x01 ) || stepTable[rateLo + ( stepState >> 1 )] )
						m_env++;
				}
			}
			break;
	}
	m_clock++;
	
	if( m_env>Silence ) {
		m_env = Silence;
		if( m_stage == Stage::RELEASE ) {
			m_total = Silence;
			m_stage = Stage::OFF;
			return Silence;
		}
	}
	
	int total = m_env + ( m_tl << 2 ) + m_kslAdd;

	if( am ) {
		int amVal = m_opl->tremoloIndex() >> 8;
		if( amVal > 26 ) {
			amVal = ( 2 * 26 ) - amVal;
		}
		if( m_opl->dam() ) {
			amVal >>= 2;
		}
		total += amVal;
	}

	if( total > Silence ) {
		m_total = Silence;
	}
	else {
		m_total = total;
	}
	return m_total;
}

void EnvelopeGenerator::keyOn()
{
	m_stage = Stage::ATTACK;
	m_clock = 0;
	m_env = Silence;
}

void EnvelopeGenerator::keyOff()
{
	if( m_stage != Stage::OFF && m_stage != Stage::RELEASE ) {
		m_stage = Stage::RELEASE;
		m_clock = 0;
	}
}

light4cxx::Logger* EnvelopeGenerator::logger()
{
	return light4cxx::Logger::get("opl.envelope");
}
}
