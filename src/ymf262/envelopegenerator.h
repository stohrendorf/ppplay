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
 * 
 * @details
 * This is a 9-bit envelope generator. It has a range of 0 to 96 dB,
 * which means that 16 units are 3 dB which halves the output.
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
	//! @brief Envelope, 9 bits
	uint16_t m_env;
	//! @brief Key scale rate
	bool m_ksr;
	//! @brief Total level, 6 bits, att. is 0.75dB * m_tl
	uint8_t m_tl;
	//! @brief Key scale level (2 bits)
	uint8_t m_ksl;
	//! @brief Key scale level in Base-2-dB
	uint8_t m_kslAdd;
	//! @brief Total envelope level, 0..511
	uint16_t m_total;
	//! @brief Clock counter, 15 bits
	uint32_t m_counter;

	static const uint16_t Silence = 511;
	
	uint8_t calculateRate(uint8_t delta) const;
	/**
	 * @brief Advances the counter and returns the overflow
	 * @param[in] rate Decay/release rate
	 * @return Counter overflow
	 */
	uint8_t advanceCounter(uint8_t rate);
	/**
	 * @brief Handles decay/release phases
	 * @param[in] rate Decay/release rate register value
	 */
	void attenuate(uint8_t rate);
	/**
	 * @brief Handles attack phase
	 */
	void attack();
	
public:
	constexpr EnvelopeGenerator( Opl3* opl )
		: m_opl( opl ), m_stage( Stage::OFF ), /*m_attenuation( 0 ),*/
		  m_ar( 0 ), m_dr( 0 ), m_sl( 0 ), m_rr( 0 ), m_fnum( 0 ), m_block( 0 ),
		  m_env( Silence ), m_ksr( false ), m_tl( 0 ), m_ksl( 0 ),
		  m_kslAdd(0), m_total(Silence), m_counter(0)
	{
	}

	constexpr bool isOff() {
		return m_stage == Stage::OFF;
	}
	constexpr bool isSilent() {
		return m_total == Silence;
	}
	/**
	 * @post m_stage==Stage::OFF
	 */
	void setOff() {
		m_stage = Stage::OFF;
	}

	/**
	 * @post m_sl<15 || m_sl==31
	 */
	void setActualSustainLevel( uint8_t sl );

	/**
	 * @param[in] tl Total level, 6 bits
	 * @post m_tl < 64
	 */
	void setTotalLevel( uint8_t tl );

	/**
	 * @post m_ksl<4 && m_block<8 && m_fnum<1024
	 */
	void setAttennuation( uint16_t f_number, uint8_t block, uint8_t ksl );
	
	/**
	 * @post m_ar<=15
	 */
	void setAttackRate( uint8_t attackRate );
	
	/**
	 * @post m_dr<=15
	 */
	void setDecayRate( uint8_t decayRate );
	
	/**
	 * @post m_rr<=15
	 */
	void setReleaseRate( uint8_t releaseRate );
	void setKsr( bool ksr ) {
		m_ksr = ksr;
	}

	/**
	 * @return Envelope, 0..511 for 0..96dB
	 * @post m_env<=ExactSilence
	 */
	uint16_t advance( bool egt, bool am );
	
	constexpr uint16_t envelope() const
	{
		return m_total;
	}
	
	/**
	 * @post m_stage==Stage::ATTACK && m_clock==0
	 */
	void keyOn();
	
	/**
	 * @post m_stage==Stage::OFF || m_stage==Stage::RELEASE
	 */
	void keyOff();
};
}

#endif
