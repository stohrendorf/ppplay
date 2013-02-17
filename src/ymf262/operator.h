#ifndef PPP_OPL_OPERATOR_H
#define PPP_OPL_OPERATOR_H

#include <memory>

#include <stuff/utils.h>

#include "phasegenerator.h"
#include "envelopegenerator.h"

#include <light4cxx/logger.h>

namespace opl
{

class Opl3;
class Operator
{
	DISABLE_COPY( Operator )
public:
	typedef std::shared_ptr<Operator> Ptr;
	
	static constexpr int
	AM1_VIB1_EGT1_KSR1_MULT4_Offset = 0x20,
	KSL2_TL6_Offset = 0x40,
	AR4_DR4_Offset = 0x60,
	SL4_RR4_Offset = 0x80,
	_5_WS3_Offset = 0xE0;

private:
	Opl3* m_opl;
	int m_operatorBaseAddress;

	PhaseGenerator m_phaseGenerator;
	EnvelopeGenerator m_envelopeGenerator;

	uint16_t m_phase;

	//! @brief Amplitude Modulation. This register is used in EnvelopeGenerator::getEnvelope().
	bool m_am;
	//! @brief Vibrato. This register is used in PhaseGenerator::getPhase().
	bool m_vib;
	//! @brief Key Scale Rate. Sets the actual envelope rate together with rate and keyScaleNumber. This register is used in EnvelopeGenerator::setActualAttackRate().
	bool m_ksr;
	//! @brief Envelope Generator Type. This register is used in EnvelopeGenerator::getEnvelope().
	bool m_egt;
	//! @brief Multiple. Multiplies the Channel.baseFrequency to get the Operator.operatorFrequency. This register is used in PhaseGenerator::setFrequency().
	uint8_t m_mult;
	//! @brief Key Scale Level. Sets the attenuation in accordance with the octave.
	uint8_t m_ksl;
	//! @brief Total Level. Sets the overall damping for the envelope.
	uint8_t m_tl;
	//! @brief Attack Rate.
	uint8_t m_ar;
	//! @brief Decay Rate.
	uint8_t m_dr;
	//! @brief Sustain Level.
	uint8_t m_sl;
	//! @brief Release Rate.
	uint8_t m_rr;
	uint8_t m_ws;
	// 0..1023
	uint16_t m_f_number;
	// 0..7
	uint8_t m_block;
	
	static light4cxx::Logger* logger();

	int16_t handleTopCymbal();
	int16_t handleHighHat( uint16_t modulator );
	int16_t handleSnareDrum( uint16_t modulator );

public:
	static constexpr uint noModulator = 0;

	/**
	 * @post m_ar<16
	 */
	void setAr( uint8_t val ) {
		m_ar = val&0x0f;
	}
	const EnvelopeGenerator* envelopeGenerator() const {
		return &m_envelopeGenerator;
	}
	EnvelopeGenerator* envelopeGenerator() {
		return &m_envelopeGenerator;
	}
	const PhaseGenerator* phaseGenerator() const {
		return &m_phaseGenerator;
	}
	PhaseGenerator* phaseGenerator() {
		return &m_phaseGenerator;
	}

	Operator( Opl3* opl, int baseAddress );
	virtual ~Operator() {}

	/**
	 * @post m_mult<16
	 */
	void update_AM1_VIB1_EGT1_KSR1_MULT4();
	
	/**
	 * @post m_ksl<4 && m_tl<64
	 */
	void update_KSL2_TL6();

	/**
	 * @post m_ar<16 && m_dr<16
	 */
	void update_AR4_DR4();
	
	/**
	 * @post m_sl<16 && m_rr<16
	 */
	void update_SL4_RR4();
	
	/**
	 * @post m_ws<8
	 */
	void update_5_WS3();

	/**
	 * @param[in] modulator Fractional phase modulation, max. 10.9 bits used
	 * @return 13.9 bit sample value
	 * @see AbstractChannel::avgFeedback()
	 */
	int16_t nextSample( uint16_t modulator );

	/**
	 * @brief Calculate operator output
	 * @param[in] outputPhase Waveform phase, max. 10.9 bit used
	 * @param[in] ws Waveform selector
	 * @return Waveform sample, amplitude is -4085..4084
	 */
	int16_t getOutput( uint16_t outputPhase, uint8_t ws );

	void keyOn();
	void keyOff();
	void updateOperator( uint16_t f_num, uint8_t blk );
};
}

#endif
