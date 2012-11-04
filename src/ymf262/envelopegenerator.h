#ifndef PPP_OPL_ENVELOPEGENERATOR_H
#define PPP_OPL_ENVELOPEGENERATOR_H

#include <cmath>
#include <cstdint>

namespace opl
{

class Opl3;

/**
 * @class EnvelopeGenerator
 * @brief Envelope generator
 */
class EnvelopeGenerator
{
public:
	enum class Stage
	{
		ATTACK, DECAY, SUSTAIN, RELEASE, OFF
	};
private:
	Opl3* m_opl;
	Stage m_stage;
	//! @brief Attack rate (4 bits)
	uint8_t m_ar;
	//! @brief Decay rate (4 bits)
	uint8_t m_dr;
	/**
	 * @brief Sustain level (4 bits)
	 * @note Effective size is 5 bits, to handle the case SL=0x0f
	 */
	uint8_t m_sl;
	//! @brief Release rate (4 bits)
	uint8_t m_rr;
	//! @brief F-Number (10 bits)
	uint16_t m_fnum;
	//! @brief Block (3 bits)
	uint8_t m_block;
	//! @brief Envelope, 511 = max. att.
	uint16_t m_env;
	//! @brief Key scale rate
	bool m_ksr;
	//! @brief Internal envelope clock counter
	uint32_t m_clock;
	//! @brief Total level, 6 bits, att. is 0.75dB * m_tl
	uint8_t m_tl;
	//! @brief Key scale level (2 bits)
	uint8_t m_ksl;
	//! @brief Key scale level in Base-2-dB
	uint8_t m_kslAdd;

	static constexpr uint16_t Silence = 511;
	
public:
	EnvelopeGenerator( Opl3* opl )
		: m_opl( opl ), m_stage( Stage::OFF ), /*m_attenuation( 0 ),*/
		  m_ar( 0 ), m_dr( 0 ), m_sl( 0 ), m_rr( 0 ), m_fnum( 0 ), m_block( 0 ),
		  m_env( Silence ), m_ksr( false ), m_clock( 0 ), m_tl( 0 ), m_ksl( 0 ),
		  m_kslAdd(0)
	{
	}

	bool isOff() const {
		return m_stage == Stage::OFF;
	}
	void setOff() {
		m_stage = Stage::OFF;
	}

	void setActualSustainLevel( uint8_t sl );

	/**
	 * @param[in] tl Total level, 6 bits
	 */
	void setTotalLevel( uint8_t tl );

	void setAttennuation( uint16_t f_number, uint8_t block, uint8_t ksl );
	void setAttackRate( uint8_t attackRate );
	void setDecayRate( uint8_t decayRate );
	void setReleaseRate( uint8_t releaseRate );
	void setKsr( bool ksr ) {
		m_ksr = ksr;
	}

public:
	// output is 0..63 for 0..96dB
	uint8_t getEnvelope( bool egt, bool am );
	void keyOn();
	void keyOff();

private:
};
}

#endif
