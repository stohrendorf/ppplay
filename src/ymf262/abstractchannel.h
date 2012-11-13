#ifndef PPP_OPL_ABSTRACTCHANNEL_H
#define PPP_OPL_ABSTRACTCHANNEL_H

#include "stuff/utils.h"
#include "fractional9.h"

#include <light4cxx/logger.h>
#include <vector>
#include <cstdint>
#include <memory>

namespace opl
{

class Opl3;

class AbstractChannel
{
	DISABLE_COPY( AbstractChannel )
public:
	typedef std::shared_ptr<AbstractChannel> Ptr;
	
	static constexpr int _2_KON1_BLOCK3_FNUMH2_Offset = 0xB0;
	static constexpr int  FNUML8_Offset = 0xA0;
	static constexpr int  CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset = 0xC0;
private:
	Opl3* m_opl;
	int m_channelBaseAddress;

	//! @brief Frequency Number, 0..1023
	uint16_t m_fnum;
	//! @brief Key On. If changed, calls Channel.keyOn() / keyOff().
	bool m_kon;
	//! @brief Block/octave (0..7)
	uint8_t m_block;
	bool m_cha;
	bool m_chb;
	bool m_chc;
	bool m_chd;
	uint8_t m_fb;
	Fractional9 m_feedback[2];
	bool m_cnt;
	
	static light4cxx::Logger* logger();

public:
	bool cnt() const {
		return m_cnt;
	}
	Opl3* opl() const {
		return m_opl;
	}
	uint16_t fnum() const {
		return m_fnum;
	}
	uint8_t block() const {
		return m_block;
	}
	/**
	 * @brief Calculate adjusted phase feedback
	 * @return 10.9 bit fractional adjusted phase feedback using m_fb
	 */
	Fractional9 avgFeedback() const {
		if( m_fb == 0) {
			return Fractional9(0);
		}
		return (( m_feedback[0] + m_feedback[1] ) << m_fb)>>9;
	}
	void clearFeedback() {
		m_feedback[0] = m_feedback[1] = 0;
	}
	/**
	 * @brief Push feedback into the queue
	 * @param[in] fb 10 bit non-fractional feedback
	 */
	void pushFeedback( Fractional9 fb ) {
		m_feedback[0] = m_feedback[1];
		m_feedback[1] = fb;
	}

	AbstractChannel( Opl3* opl, int baseAddress );
	virtual ~AbstractChannel() {}

	/**
	 * @post m_fnumh<8 && m_block<8
	 */
	void update_2_KON1_BLOCK3_FNUMH2();

	void update_FNUML8();

	/**
	 * @post m_fb<8
	 */
	void update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();

	void updateChannel();

	std::vector< int16_t > getInFourChannels( int16_t channelOutput );

	virtual std::vector<int16_t> nextSample() = 0;
	virtual void keyOn() = 0;
	virtual void keyOff() = 0;
	virtual void updateOperators() = 0;

	int baseAddress() const {
		return m_channelBaseAddress;
	}
};
}

#endif
