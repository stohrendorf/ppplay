#ifndef PPP_OPL_ABSTRACTCHANNEL_H
#define PPP_OPL_ABSTRACTCHANNEL_H

#include "stuff/utils.h"
#include <vector>

namespace opl
{
class AbstractChannel
{
	DISABLE_COPY( AbstractChannel )
public:
	static constexpr int _2_KON1_BLOCK3_FNUMH2_Offset = 0xB0;
	static constexpr int  FNUML8_Offset = 0xA0;
	static constexpr int  CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset = 0xC0;

	// Feedback rate in fractions of 2*Pi, normalized to (0,1):
	// 0, Pi/16, Pi/8, Pi/4, Pi/2, Pi, 2*Pi, 4*Pi turns to be:
	static constexpr double feedback[] = {0, 1 / 32d, 1 / 16d, 1 / 8d, 1 / 4d, 1 / 2d, 1, 2};
private:
	int m_channelBaseAddress;

	double m_feedback[2];

	int m_fnuml;
	int m_fnumh;
	int m_kon;
	int m_block;
	int m_cha;
	int m_chb;
	int m_chc;
	int m_chd;
	int m_fb;
	int m_cnt;

	// Factor to convert between normalized amplitude to normalized
	// radians. The amplitude maximum is equivalent to 8*Pi radians.
	static constexpr double toPhase = 4;

public:
	AbstractChannel( int baseAddress );
	virtual ~AbstractChannel() {}

	void update_2_KON1_BLOCK3_FNUMH2() ;

	void update_FNUML8() ;

	void update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1() ;

	void updateChannel() ;

protected:
	std::vector<double> getInFourChannels( double channelOutput ) ;

	virtual std::vector<double> getChannelOutput() = 0;
	virtual void keyOn() = 0;
	virtual void keyOff() = 0;
	virtual void updateOperators() = 0;
};
}

#endif
