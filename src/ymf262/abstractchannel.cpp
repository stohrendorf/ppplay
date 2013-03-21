/*
 * PeePeePlayer - an old-fashioned module player
 * Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Original Java Code: Copyright (C) 2008 Robson Cozendey <robson@cozendey.com>
 * 
 * Some code based on forum posts in: http://forums.submarine.org.uk/phpBB/viewforum.php?f=9,
 * Copyright (C) 2010-2013 by carbon14 and opl3
 */

#include "abstractchannel.h"
#include "opl3.h"

namespace opl
{

AbstractChannel::AbstractChannel( Opl3* opl, int baseAddress ) : m_opl( opl ), m_channelBaseAddress( baseAddress ),
	m_fnum( 0 ), m_kon( false ), m_block( 0 ), m_cha( false ), m_chb( false ), m_chc( false ), m_chd( false ),
	m_fb( 0 ), m_feedback {0, 0}, m_cnt( false ) {
}

void AbstractChannel::update_2_KON1_BLOCK3_FNUMH2()
{

	const uint8_t _2_kon1_block3_fnumh2 = m_opl->readReg( m_channelBaseAddress + AbstractChannel::_2_KON1_BLOCK3_FNUMH2_Offset );

	// Frequency Number (hi-register) and Block. These two registers, together with fnuml,
	// sets the Channel´s base frequency;
	m_block = ( _2_kon1_block3_fnumh2 >> 2 ) & 7;
	m_fnum = (m_fnum&0xff) | ((_2_kon1_block3_fnumh2 & 0x03)<<8);
	updateOperators();

	bool newKon = _2_kon1_block3_fnumh2 & 0x20;
	if( newKon != m_kon ) {
		if( newKon ) {
			keyOn();
		}
		else {
			keyOff();
		}
		m_kon = newKon;
	}
}

void AbstractChannel::update_FNUML8()
{
	m_fnum = (m_fnum&0xff00) | m_opl->readReg( m_channelBaseAddress + AbstractChannel::FNUML8_Offset );
	updateOperators();
}

void AbstractChannel::update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1()
{
	const uint8_t chd1_chc1_chb1_cha1_fb3_cnt1 = m_opl->readReg( m_channelBaseAddress + AbstractChannel::CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset );
	m_chd = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x80;
	m_chc = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x40;
	m_chb = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x20;
	m_cha = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x10;
	m_fb  = ( chd1_chc1_chb1_cha1_fb3_cnt1>>1 ) & 7;
	m_cnt = chd1_chc1_chb1_cha1_fb3_cnt1 & 0x01;
	updateOperators();
}

void AbstractChannel::updateChannel()
{
	update_2_KON1_BLOCK3_FNUMH2();
	update_FNUML8();
	update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();
}

void AbstractChannel::getInFourChannels( std::array<int16_t, 4>* dest, int16_t channelOutput )
{
	if(!dest) {
		return;
	}
	if( !m_opl->isNew() ) {
		std::fill_n(dest->begin(), 4, channelOutput);
	}
	else {
		(*dest)[0] = m_cha ? channelOutput : 0;
		(*dest)[1] = m_chb ? channelOutput : 0;
		(*dest)[2] = m_chc ? channelOutput : 0;
		(*dest)[3] = m_chd ? channelOutput : 0;
	}
}

light4cxx::Logger* AbstractChannel::logger()
{
	return light4cxx::Logger::get("opl.channel");
}

}
