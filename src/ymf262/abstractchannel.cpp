#include "abstractchannel.h"
#include "opl3.h"

namespace opl
{

const int AbstractChannel::FeedbackShift[8] = {15, 6, 5, 4, 3, 2, 1, 0};

AbstractChannel::AbstractChannel( Opl3* opl, int baseAddress ) : m_opl( opl ), m_channelBaseAddress( baseAddress ),
	m_fnuml( 0 ), m_fnumh( 0 ), m_kon( false ), m_block( 0 ), m_cha( false ), m_chb( false ), m_chc( false ), m_chd( false ),
	m_fb( 0 ), m_feedback {0, 0}, m_cnt( false ) {
}
void AbstractChannel::update_2_KON1_BLOCK3_FNUMH2()
{

	int _2_kon1_block3_fnumh2 = m_opl->readReg( m_channelBaseAddress + AbstractChannel::_2_KON1_BLOCK3_FNUMH2_Offset );

	// Frequency Number (hi-register) and Block. These two registers, together with fnuml,
	// sets the Channel´s base frequency;
	m_block = ( _2_kon1_block3_fnumh2 & 0x1C ) >> 2;
	m_fnumh = _2_kon1_block3_fnumh2 & 0x03;
	updateOperators();

	bool newKon   = ( _2_kon1_block3_fnumh2 & 0x20 ) >> 5;
	if( newKon != m_kon ) {
		if( newKon )
			keyOn();
		else
			keyOff();
		m_kon = newKon;
	}
}
void AbstractChannel::update_FNUML8()
{
	int fnuml8 = m_opl->readReg( m_channelBaseAddress + AbstractChannel::FNUML8_Offset );
	m_fnuml = fnuml8 & 0xFF;
	updateOperators();
}
void AbstractChannel::update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1()
{
	int chd1_chc1_chb1_cha1_fb3_cnt1 = m_opl->readReg( m_channelBaseAddress + AbstractChannel::CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset );
	m_chd = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x80;
	m_chc = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x40;
	m_chb = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x20;
	m_cha = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x10;
	m_fb  = ( chd1_chc1_chb1_cha1_fb3_cnt1 & 0x0E ) >> 1;
	m_cnt = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x01;
	updateOperators();
}
void AbstractChannel::updateChannel()
{
	update_2_KON1_BLOCK3_FNUMH2();
	update_FNUML8();
	update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();
}
std::vector< int16_t > AbstractChannel::getInFourChannels( int16_t channelOutput )
{
	std::vector<int16_t> output( 4 );

	if( !m_opl->isNew() )
		output[0] = output[1] = output[2] = output[3] = channelOutput;
	else {
		output[0] = m_cha ? channelOutput : 0;
		output[1] = m_chb ? channelOutput : 0;
		output[2] = m_chc ? channelOutput : 0;
		output[3] = m_chd ? channelOutput : 0;
	}

	return output;
}
}
