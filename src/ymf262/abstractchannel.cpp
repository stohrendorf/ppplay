#include "abstractchannel.h"
#include "opl3.h"

namespace opl
{
AbstractChannel::AbstractChannel( Opl3* opl, int baseAddress ) : m_opl(opl), m_channelBaseAddress( baseAddress ),
	m_fnuml( 0 ), m_fnumh( 0 ), m_kon( 0 ), m_block( 0 ), m_cha( 0 ), m_chb( 0 ), m_chc( 0 ), m_chd( 0 ),
	m_fb( 0 ), m_cnt( 0 ), m_feedback {0, 0} {
}
void AbstractChannel::update_2_KON1_BLOCK3_FNUMH2()
{

	int _2_kon1_block3_fnumh2 = m_opl->readReg(m_channelBaseAddress + AbstractChannel::_2_KON1_BLOCK3_FNUMH2_Offset);

	// Frequency Number (hi-register) and Block. These two registers, together with fnuml,
	// sets the Channel´s base frequency;
	m_block = ( _2_kon1_block3_fnumh2 & 0x1C ) >> 2;
	m_fnumh = _2_kon1_block3_fnumh2 & 0x03;
	updateOperators();

	// Key On. If changed, calls Channel.keyOn() / keyOff().
	int newKon   = ( _2_kon1_block3_fnumh2 & 0x20 ) >> 5;
	if( newKon != m_kon ) {
		if( newKon == 1 ) keyOn();
		else keyOff();
		m_kon = newKon;
	}
}
void AbstractChannel::update_FNUML8()
{
	int fnuml8 = m_opl->readReg(m_channelBaseAddress + AbstractChannel::FNUML8_Offset);
	// Frequency Number, low register.
	m_fnuml = fnuml8 & 0xFF;
	updateOperators();
}
void AbstractChannel::update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1()
{
	int chd1_chc1_chb1_cha1_fb3_cnt1 = m_opl->readReg(m_channelBaseAddress + AbstractChannel::CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset);
	m_chd   = ( chd1_chc1_chb1_cha1_fb3_cnt1 & 0x80 ) >> 7;
	m_chc   = ( chd1_chc1_chb1_cha1_fb3_cnt1 & 0x40 ) >> 6;
	m_chb   = ( chd1_chc1_chb1_cha1_fb3_cnt1 & 0x20 ) >> 5;
	m_cha   = ( chd1_chc1_chb1_cha1_fb3_cnt1 & 0x10 ) >> 4;
	m_fb    = ( chd1_chc1_chb1_cha1_fb3_cnt1 & 0x0E ) >> 1;
	m_cnt   = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x01;
	updateOperators();
}
void AbstractChannel::updateChannel()
{
	update_2_KON1_BLOCK3_FNUMH2();
	update_FNUML8();
	update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();
}
std::vector< double > AbstractChannel::getInFourChannels( double channelOutput )
{
	std::vector<double> output( 4 );

	if( !m_opl->isNew() )
		output[0] = output[1] = output[2] = output[3] = channelOutput;
	else {
		output[0] = ( m_cha == 1 ) ? channelOutput : 0;
		output[1] = ( m_chb == 1 ) ? channelOutput : 0;
		output[2] = ( m_chc == 1 ) ? channelOutput : 0;
		output[3] = ( m_chd == 1 ) ? channelOutput : 0;
	}

	return output;
}
}

