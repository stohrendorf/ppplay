#ifndef PPP_OPL_ABSTRACTCHANNEL_H
#define PPP_OPL_ABSTRACTCHANNEL_H

#include "stuff/utils.h"
#include <vector>
#include <cstdint>

namespace opl
{

class Opl3;

class AbstractChannel
{
	DISABLE_COPY( AbstractChannel )
public:
	static constexpr int _2_KON1_BLOCK3_FNUMH2_Offset = 0xB0;
	static constexpr int  FNUML8_Offset = 0xA0;
	static constexpr int  CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset = 0xC0;

	// Feedback rate in fractions of 2*Pi, normalized to (0,1):
	// 0, Pi/16, Pi/8, Pi/4, Pi/2, Pi, 2*Pi, 4*Pi turns to be:
	static constexpr double feedback[] = {0, 1 / 32.0, 1 / 16.0, 1 / 8.0, 1 / 4.0, 1 / 2.0, 1, 2};
private:
	Opl3* m_opl;
	int m_channelBaseAddress;

	double m_feedback[2];

	//! @brief Frequency Number, low register.
	int m_fnuml;
	int m_fnumh;
	//! @brief Key On. If changed, calls Channel.keyOn() / keyOff().
	bool m_kon;
	// 0..7
	uint8_t m_block;
	bool m_cha;
	bool m_chb;
	bool m_chc;
	bool m_chd;
	uint8_t m_fb;
	bool m_cnt;

public:
	// Factor to convert between normalized amplitude to normalized
	// radians. The amplitude maximum is equivalent to 8*Pi radians.
	static constexpr double toPhase = 4;

	bool cnt() const {
		return m_cnt;
	}
	Opl3* opl() const {
		return m_opl;
	}
	int fnuml() const {
		return m_fnuml;
	}
	int fnumh() const {
		return m_fnumh;
	}
	uint8_t block() const {
		return m_block;
	}
	uint8_t fb() const {
		return m_fb;
	}
	double avgFeedback() const {
		return ( m_feedback[0] + m_feedback[1] ) / 2;
	}
	void clearFeedback() {
		m_feedback[0] = m_feedback[1] = 0;
	}
	void pushFeedback( double fb ) {
		m_feedback[0] = m_feedback[1];
		m_feedback[1] = fb;
	}

	AbstractChannel( Opl3* opl, int baseAddress );
	virtual ~AbstractChannel() {}

	void update_2_KON1_BLOCK3_FNUMH2() ;

	void update_FNUML8() ;

	void update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1() ;

	void updateChannel() ;

	std::vector<double> getInFourChannels( double channelOutput ) ;

	virtual std::vector<double> nextSample() = 0;
	virtual void keyOn() = 0;
	virtual void keyOff() = 0;
	virtual void updateOperators() = 0;

	int baseAddress() const {
		return m_channelBaseAddress;
	}
};
}

#endif
