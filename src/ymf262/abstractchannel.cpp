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
#include <stream/abstractarchive.h>

namespace opl
{

AbstractChannel::AbstractChannel( Opl3* opl, int baseAddress, const std::initializer_list<Operator*>& ops ) :
	m_opl( opl ), m_channelBaseAddress( baseAddress ),
	m_fnum( 0 ), m_kon( false ), m_block( 0 ), m_ch( 0 ),
	m_fb( 0 ), m_feedback {0, 0}, m_cnt( false ),
	m_operators(ops)
{
}

void AbstractChannel::update_2_KON1_BLOCK3_FNUMH2()
{

	const uint8_t _2_kon1_block3_fnumh2 = m_opl->readReg( m_channelBaseAddress + AbstractChannel::_2_KON1_BLOCK3_FNUMH2_Offset );

	// Frequency Number (hi-register) and Block. These two registers, together with fnuml,
	// sets the Channel's base frequency;
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
	m_fnum = (m_fnum&0x0300) | m_opl->readReg( m_channelBaseAddress + AbstractChannel::FNUML8_Offset );
	updateOperators();
}

void AbstractChannel::update_CH4_FB3_CNT1()
{
	const uint8_t ch4_fb3_cnt1 = m_opl->readReg( m_channelBaseAddress + AbstractChannel::CH4_FB3_CNT1_Offset );
	m_ch = ch4_fb3_cnt1>>4;
	m_fb  = ( ch4_fb3_cnt1>>1 ) & 7;
	m_cnt = ch4_fb3_cnt1 & 0x01;
	updateOperators();
}

void AbstractChannel::updateChannel()
{
	update_2_KON1_BLOCK3_FNUMH2();
	update_FNUML8();
	update_CH4_FB3_CNT1();
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
		for(int i=0; i<4; i++) {
			(*dest)[i] = (m_ch>>i)&1 ? channelOutput : 0;
		}
	}
}

light4cxx::Logger* AbstractChannel::logger()
{
	return light4cxx::Logger::get("opl.channel");
}

AbstractArchive& AbstractChannel::serialize(AbstractArchive* archive)
{
	*archive % m_fnum % m_kon % m_block
	% m_ch % m_fb % m_feedback[0] % m_feedback[1] % m_cnt;
	return *archive;
}

Operator* AbstractChannel::op(size_t idx) noexcept
{
	if(idx<=0 || idx>=m_operators.size()+1)
		return nullptr;
	return m_operators[idx-1];
}

void AbstractChannel::keyOff()
{
	if(isRhythmChannel())
		return;
	for(Operator* op : m_operators)
		op->keyOff();
}

void AbstractChannel::keyOn()
{
	for(Operator* op : m_operators)
		op->keyOn();
}

void AbstractChannel::updateOperators()
{
	for(Operator* op : m_operators)
		op->updateOperator(fnum(), block());
}

bool AbstractChannel::isRhythmChannel() const {
    return m_operators.size()==2 && opl()->ryt() && baseAddress()>=6 && baseAddress()<=8;
}

}
