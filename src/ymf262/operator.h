#ifndef PPP_OPL_OPERATOR_H
#define PPP_OPL_OPERATOR_H

#include <cmath>
#include <memory>

#include <stuff/utils.h>

#include "phasegenerator.h"
#include "envelopegenerator.h"

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

	enum class Type
	{
		NO_MODULATION, CARRIER, FEEDBACK
	};

	static constexpr int waveLength = 1024;

private:
	Opl3* m_opl;
	int m_operatorBaseAddress;

	PhaseGenerator m_phaseGenerator;
	EnvelopeGenerator m_envelopeGenerator;

	//! @brief Envelope, 0..511 for 0..96 dB
	uint8_t m_envelope;
	// 0..1023
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

public:
	static constexpr int noModulator = 0;

	void setAr( uint8_t val ) {
		m_ar = val;
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
	Opl3* opl() const {
		return m_opl;
	}
	void setEnvelope( uint8_t e ) {
		m_envelope = e;
	}
	uint8_t mult() const {
		return m_mult;
	}
	uint16_t phase() const {
		return m_phase;
	}
	void setPhase( uint16_t p ) {
		m_phase = p & 0x3ff;
	}
	bool egt() const {
		return m_egt;
	}
	bool am() const {
		return m_am;
	}
	uint8_t ws() const {
		return m_ws;
	}
	bool vib() const {
		return m_vib;
	}

	Operator( Opl3* opl, int baseAddress );
	virtual ~Operator() {}

	void update_AM1_VIB1_EGT1_KSR1_MULT4();
	void update_KSL2_TL6();
	void update_AR4_DR4();
	void update_SL4_RR4();
	void update_5_WS3();

	virtual int16_t nextSample( uint16_t modulator );

	/**
	 * @brief Calculate operator output
	 * @param[in] outputPhase Phase, 0..1023
	 * @param[in] ws Waveform selector
	 * @return Waveform sample, amplitude is -4085..4084
	 * @note @a modulator and @a outputPhase will simply be added together
	 */
	int16_t getOutput( uint16_t outputPhase, uint8_t ws );

	void keyOn();
	void keyOff();
	void updateOperator( uint16_t f_num, uint8_t blk );
};
}

#endif
