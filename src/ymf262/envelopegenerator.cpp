#include "envelopegenerator.h"
#include "operator.h"
#include "opl3.h"

#if 0
namespace
{
// These attack periods in miliseconds were taken from the YMF278B manual.
// The attack actual rates range from 0 to 63, with different data for
// 0%-100% and for 10%-90%:

/*
 * !! WRONG !!
 * Real values: [x][0] = 12445.9 * 2 ^ -x/4
 * Real values: [x][1] = 7430.38 * 2 ^ -x/4
 */
constexpr double attackTimeValuesTable[64][2] = {
	{INFINITY, INFINITY},    {INFINITY, INFINITY},    {INFINITY, INFINITY},    {INFINITY, INFINITY},
	{2826.24, 1482.75}, {2252.80, 1155.07}, {1884.16, 991.23}, {1597.44, 868.35},
	{1413.12, 741.38}, {1126.40, 577.54}, {942.08, 495.62}, {798.72, 434.18},
	{706.56, 370.69}, {563.20, 288.77}, {471.04, 247.81}, {399.36, 217.09},

	{353.28, 185.34}, {281.60, 144.38}, {235.52, 123.90}, {199.68, 108.54},
	{176.76, 92.67}, {140.80, 72.19}, {117.76, 61.95}, {99.84, 54.27},
	{88.32, 46.34}, {70.40, 36.10}, {58.88, 30.98}, {49.92, 27.14},
	{44.16, 23.17}, {35.20, 18.05}, {29.44, 15.49}, {24.96, 13.57},

	{22.08, 11.58}, {17.60, 9.02}, {14.72, 7.74}, {12.48, 6.78},
	{11.04, 5.79}, {8.80, 4.51}, {7.36, 3.87}, {6.24, 3.39},
	{5.52, 2.90}, {4.40, 2.26}, {3.68, 1.94}, {3.12, 1.70},
	{2.76, 1.45}, {2.20, 1.13}, {1.84, 0.97}, {1.56, 0.85},

	{1.40, 0.73}, {1.12, 0.61}, {0.92, 0.49}, {0.80, 0.43},
	{0.70, 0.37}, {0.56, 0.31}, {0.46, 0.26}, {0.42, 0.22},
	{0.38, 0.19}, {0.30, 0.14}, {0.24, 0.11}, {0.20, 0.11},
	{0.00, 0.00}, {0.00, 0.00}, {0.00, 0.00}, {0.00, 0.00}
};

// These decay and release periods in miliseconds were taken from the YMF278B manual.
// The rate index range from 0 to 63, with different data for
// 0%-100% and for 10%-90%: [attackRate][0to100,10to90]

/*
 * [x][0] = 78561.28 * 2 ^ (-x/4) = 2^(ld(78561.28) - x/4)
 * [x][1] = 16424.96 * 2 ^ (-x/4) = 2^(ld(16424.96) - x/4)
 * 2^(ld(78.56128*1024) - x/4) ~= 2^(16.3 - x/4)
 *                             ~= 2^(16 - x/4 + 1/4)
 */
constexpr double decayAndReleaseTimeValuesTable[64][2] = {
	{INFINITY, INFINITY},    {INFINITY, INFINITY},    {INFINITY, INFINITY},    {INFINITY, INFINITY},
	{39280.64, 8212.48}, {31416.32, 6574.08}, {26173.44, 5509.12}, {22446.08, 4730.88},
	{19640.32, 4106.24}, {15708.16, 3287.04}, {13086.72, 2754.56}, {11223.04, 2365.44},
	{9820.16, 2053.12}, {7854.08, 1643.52}, {6543.36, 1377.28}, {5611.52, 1182.72},

	{4910.08, 1026.56}, {3927.04, 821.76}, {3271.68, 688.64}, {2805.76, 591.36},
	{2455.04, 513.28}, {1936.52, 410.88}, {1635.84, 344.34}, {1402.88, 295.68},
	{1227.52, 256.64}, {981.76, 205.44}, {817.92, 172.16}, {701.44, 147.84},
	{613.76, 128.32}, {490.88, 102.72}, {488.96, 86.08}, {350.72, 73.92},

	{306.88, 64.16}, {245.44, 51.36}, {204.48, 43.04}, {175.36, 36.96},
	{153.44, 32.08}, {122.72, 25.68}, {102.24, 21.52}, {87.68, 18.48},
	{76.72, 16.04}, {61.36, 12.84}, {51.12, 10.76}, {43.84, 9.24},
	{38.36, 8.02}, {30.68, 6.42}, {25.56, 5.38}, {21.92, 4.62},

	{19.20, 4.02}, {15.36, 3.22}, {12.80, 2.68}, {10.96, 2.32},
	{9.60, 2.02}, {7.68, 1.62}, {6.40, 1.35}, {5.48, 1.15},
	{4.80, 1.01}, {3.84, 0.81}, {3.20, 0.69}, {2.74, 0.58},
	{2.40, 0.51}, {2.40, 0.51}, {2.40, 0.51}, {2.40, 0.51}
};

inline uint8_t calculateActualRate( uint8_t rate, bool ksr, uint8_t keyScaleNumber )
{
	// If, as an example at the maximum, rate is 15 and the rate offset is 15,
	// the value would be 75, but the maximum allowed is 63:
	uint8_t rof = keyScaleNumber;
	if( !ksr ) {
		rof >>= 2;
	}
	return std::max( 63, rate * 4 + rof );
}

}
#endif

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
	m_tl = tl;
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
	int tmp = ddArray[m_fnum >> 6] - 16*m_block;
	if( tmp<= 0 ) {
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

uint16_t EnvelopeGenerator::getEnvelope( bool egt, bool am )
{
	// http://forums.submarine.org.uk/phpBB/viewtopic.php?f=9&t=16&start=20
	static constexpr uint8_t stepTable[16] = {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1};
	switch( m_stage ) {
		case Stage::ATTACK:
			if( m_env != 0 ) {
				int rate1 = ( ( ( m_block << 1 ) + ( ( m_fnum >> ( m_opl->nts() ? 8 : 9 ) ) & 0x1 ) ) >> ( m_ksr ? 0 : 2 ) ) + ( m_ar << 2 );
				if( rate1 > 60 ) {
					rate1 = 60;
				}
				int rate2 = rate1 & 0x03;
				rate1 >>= 2;

				if( m_ar != 0 ) {
					if( !( m_clock & ( 0x0FFF >> rate1 ) ) ) {
						if( rate1 == 15 ) {
							m_env = 0;
						}
						else if( rate1 > 12 ) {
							int stepState = m_clock & 0x07;
							m_env -= ( m_env >> ( 16 - rate1 - stepTable[( rate2 << 2 ) + ( stepState >> 1 )] ) ) + 1;
						}
						else {
							int stepState = ( m_clock >> ( 12 - rate1 ) ) & 0x07;
							if( ( stepState & 0x01 ) || stepTable[( rate2 << 2 ) + ( stepState >> 1 )] )
								m_env -= ( m_env >> 3 ) + 1;
						}
					}
				}
			}
			else {
				m_stage = Stage::DECAY;
			}
			break;

		case Stage::DECAY:
			if( m_env < ( 1 << ( m_sl + 3 ) ) ) {
				if( m_dr ) {
					int rate1 = ( ( ( m_block << 1 ) + ( ( m_fnum >> ( m_opl->nts() ? 8 : 9 ) ) & 0x1 ) ) >> ( m_ksr ? 0 : 2 ) ) + ( m_dr << 2 );
					if( rate1 > 60 )
						rate1 = 60;
					int rate2 = rate1 & 0x03;
					rate1 >>= 2;

					if( !( m_clock & ( 0x0FFF >> rate1 ) ) ) {
						if( rate1 > 12 ) {
							int stepState = m_clock & 0x07;
							m_env += ( 1 << ( rate1 + stepTable[( rate2 << 2 ) + ( stepState >> 1 )] - 13 ) );
						}
						else {
							int stepState = ( m_clock >> ( 12 - rate1 ) ) & 0x07;
							if( ( stepState & 0x01 ) || stepTable[( rate2 << 2 ) + ( stepState >> 1 )] )
								m_env++;
						}
					}
				}
			}
			else
				m_stage = egt ? Stage::SUSTAIN : Stage::RELEASE;
			break;
			
		case Stage::SUSTAIN:
			break;
			
		case Stage::RELEASE:
			if( m_rr ) {
				int rate1 = ( ( ( m_block << 1 ) + ( ( m_fnum >> ( m_opl->nts() ? 8 : 9 ) ) & 0x1 ) ) >> ( m_ksr ? 0 : 2 ) ) + ( m_rr << 2 );
				if( rate1 > 60 )
					rate1 = 60;
				int rate2 = rate1 & 0x03;
				rate1 >>= 2;

				if( !( m_clock & ( 0x0FFF >> rate1 ) ) ) {
					if( rate1 > 12 ) {
						int stepState = m_clock & 0x07;
						m_env += ( 1 << ( rate1 - stepTable[( rate2 << 2 ) + ( stepState >> 1 )] - 13 ) );
					}
					else {
						int stepState = ( m_clock >> ( 12 - rate1 ) ) & 0x07;
						if( ( stepState & 0x01 ) || stepTable[( rate2 << 2 ) + ( stepState >> 1 )] )
							m_env++;
					}
				}
				if(m_env >= 511) {
					m_env = 511;
					m_stage = Stage::OFF;
				}
			}
			break;
	}
	if( m_stage != Stage::SUSTAIN && m_stage != Stage::OFF ) {
		m_clock++;
	}
	
	int total = m_env + (m_tl<<2) + m_kslAdd;
	
	if(am) {
		int amVal = (26*m_opl->tremoloIndex())>>8;
		if( amVal > 26 ) {
			amVal = (2*26) - amVal;
		}
		if( m_opl->dam() ) {
			amVal >>= 2;
		}
		total += amVal;
	}
	
	if(total>=511) {
		return 511;
	}
	return total;
}

void EnvelopeGenerator::keyOn()
{
	m_stage = Stage::ATTACK;
	m_clock = 0;
}

void EnvelopeGenerator::keyOff()
{
	if( m_stage != Stage::OFF ) {
		m_stage = Stage::RELEASE;
	}
}
}
