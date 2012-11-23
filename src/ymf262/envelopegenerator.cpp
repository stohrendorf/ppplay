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
	// res: max. 4 bits
	if( !m_ksr ) {
		res >>= 2;
	}
	// res: max. 7 bits (15+60=75)
	res += rate << 2;
	// res: max. 6 bits
	return std::min<uint8_t>(63, res);
}

uint16_t EnvelopeGenerator::advance( bool egt, bool am )
{
	// http://forums.submarine.org.uk/phpBB/viewtopic.php?f=9&t=16&start=20
	// sub-index is (clock&7)>>1
	static constexpr uint8_t stepTable[16] = {
		0, 0, 0, 0, // rateLo = 0
		0, 0, 1, 0, // rateLo = 1
		0, 1, 0, 1, // rateLo = 2
		0, 1, 1, 1  // rateLo = 3
	};
	/*
	 * uint8_t row = (rate<<1) | (rate>>1);
	 * uint8_t subIndex = (clock>>1) & 3;
	 * uint8_t stepBit = row>>subIndex;
	 */
	uint8_t rateHi = 0;
	switch(m_stage) {
		case Stage::OFF:
			m_total = m_env = Silence;
			return Silence;
		case Stage::ATTACK:
			rateHi = calculateRate(m_ar);
			break;
		case Stage::DECAY:
			rateHi = calculateRate(m_dr);
			break;
		case Stage::SUSTAIN:
			break;
		case Stage::RELEASE:
			rateHi = calculateRate(m_rr);
			break;
	}
	// rateHi: 0..60, rateLo: 0..12
	const uint8_t rateLo = (rateHi & 0x03) << 2;
	// rateHi: 0..15
	rateHi >>= 2;

	const uint16_t clock = m_opl->counter();
	
	switch( m_stage ) {
		case Stage::OFF:
			// already handled above, but to silence the
			// compiler about a not handled enum value...
			return Silence;
		case Stage::ATTACK:
			if( rateHi == 15 ) {
				m_env = 0;
			}
			if( m_env==0 || m_env>Silence ) {
				m_stage = Stage::DECAY;
				// in case of an overflow
				m_env = 0;
				break;
			}
			if( m_ar == 0 ) {
				break;
			}
			if( rateHi > 12 ) {
				int stepState = (clock & 0x07) >> 1;
				m_env -= ( m_env >> ( 16 - rateHi - stepTable[rateLo + stepState ] ) ) + 1;
			}
			else if( !( clock & ( 0x0FFF >> rateHi ) ) ) {
				int stepState = ( clock >> ( 12 - rateHi ) ) & 0x07;
				if( ( stepState & 0x01 ) || stepTable[ rateLo + ( stepState >> 1 )] ) {
					m_env -= ( m_env >> 3 ) + 1;
				}
			}
			break;

		case Stage::DECAY:
			if( m_env >= (m_sl<<4) ) {
				m_stage = Stage::SUSTAIN;
				break;
			}
			if(m_dr==0) {
				break;
			}

			if( rateHi > 12 ) {
				uint8_t stepState = (clock & 0x07) >> 1;
				// 1101->00 1110->01 1111->10 <==> (rateHi & 3) ^ 1
				m_env += ( 1 << ( rateHi + stepTable[ rateLo + stepState ] - 13 ) );
			}
			else if( !( clock & ( 0x0FFF >> rateHi ) ) ) {
				uint8_t stepState = ( clock >> ( 12 - rateHi ) ) & 0x07;
				if( ( stepState & 0x01 ) || stepTable[ rateLo + ( stepState >> 1 )] ) {
					m_env++;
				}
			}
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
			if( rateHi > 12 ) {
				uint8_t stepState = (clock & 0x07) >> 1;
				m_env += ( 1 << ( rateHi - stepTable[rateLo + stepState ] - 13 ) );
			}
			else if( !( clock & ( 0x0FFF >> rateHi ) ) ) {
				uint8_t stepState = ( clock >> ( 12 - rateHi ) ) & 0x07;
				if( ( stepState & 0x01 ) || stepTable[rateLo + ( stepState >> 1 )] ) {
					m_env++;
				}
			}
			break;
	}
	
	if( m_env>=Silence ) {
		// too low
		m_env = Silence;
		m_total = Silence;
		return Silence;
	}
	
	int total = m_env + (m_tl<<2) + m_kslAdd;

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
}

void EnvelopeGenerator::keyOff()
{
	if( m_stage != Stage::OFF ) {
		m_stage = Stage::RELEASE;
	}
}

light4cxx::Logger* EnvelopeGenerator::logger()
{
	return light4cxx::Logger::get("opl.envelope");
}
}
