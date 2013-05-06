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

#include "envelopegenerator.h"
#include "operator.h"
#include "opl3.h"

namespace opl
{

void EnvelopeGenerator::setAttennuation( uint16_t f_number, uint8_t block, uint8_t ksl )
{
	m_fnum = f_number & 0x3ff;
	m_block = block & 0x07;
	m_ksl = ksl & 0x03;

	if( m_ksl == 0 ) {
		m_kslAdd = 0;
		return;
	}

	// 1.5 dB att. for base 2 of oct. 7
	// created by: round(8*log2( 10^(dbMax[msb]/10) ));
	static constexpr int ddArray[16] = {
		0, 24, 32, 37, 40, 43, 45, 47, 48, 50, 51, 52, 53, 54, 55, 56
	};
	int tmp = ddArray[m_fnum >> 6] - 8*(7-m_block);
	if( tmp <= 0 ) {
		m_kslAdd = 0;
		return;
	}
	m_kslAdd = tmp;
	switch( m_ksl ) {
		case 0:
			// done
			break;
		case 1:
			// 3 db
			m_kslAdd <<= 1;
			break;
		case 2:
			// no change, 1.5 dB
			break;
		case 3:
			// 6 dB
			m_kslAdd <<= 2;
			break;
	}
}

uint8_t EnvelopeGenerator::calculateRate( uint8_t rate ) const
{
	if( rate == 0 ) {
		return 0;
	}
	// calculate key scale number
	uint8_t res = ( m_fnum >> ( m_opl->nts() ? 8 : 9 ) ) & 0x1;
	res |= m_block<<1;
	if( !m_ksr ) {
		res >>= 2;
	}
	res += rate << 2;
	return std::min<uint8_t>(63, res);
}

uint8_t EnvelopeGenerator::advanceCounter(uint8_t rate)
{
	if(rate == 0) {
		return 0;
	}
	const uint8_t effectiveRate = calculateRate(rate);
	// rateHi <= 15
	const uint8_t rateHi = effectiveRate>>2;
	// rateLo <= 3
	const uint8_t rateLo = effectiveRate&3;
	// 4 <= Delta <= (7<<15)
	m_counter += uint32_t(4|rateLo)<<rateHi;
	// res <= 7
	uint8_t res = m_counter>>15;
	m_counter &= (1<<15)-1;
	return res;
}

void EnvelopeGenerator::attenuate( uint8_t rate )
{
	m_env += advanceCounter(rate);
}

void EnvelopeGenerator::attack()
{
	uint8_t overflow = advanceCounter(m_ar);
	if( overflow==0 ) {
		return;
	}
	m_env -= (m_env>>3)*overflow + 1;
	if(m_env>Silence) { // overflow is possible
		m_env = 0;
	}
}

uint16_t EnvelopeGenerator::advance( bool egt, bool am )
{
	const uint16_t oldEnv = m_env;
	switch( m_stage ) {
		case Stage::ATTACK:
			attack();
			if( m_env==0 ) {
				m_stage = Stage::DECAY;
			}
			break;

		case Stage::DECAY:
			if( m_env >= uint32_t(m_sl)<<4 ) {
				m_stage = Stage::SUSTAIN;
				break;
			}
			attenuate(m_dr);
			break;

		case Stage::SUSTAIN:
			if(!egt) {
				m_stage = Stage::RELEASE;
			}
			break;

		case Stage::RELEASE:
			attenuate(m_rr);
			break;
	}
	
	if( m_env >= Silence ) {
		m_env = Silence;
	}
	
	int total = oldEnv + (m_tl<<2) + m_kslAdd;

	if( am ) {
		int amVal = m_opl->tremoloIndex() >> 8;
		if( amVal > 26 ) {
			amVal = ( 2 * 26 ) + ~amVal;
		}
		BOOST_ASSERT( amVal>=0 && amVal<=26 );
		if( !m_opl->dam() ) {
			amVal >>= 2;
		}
		total += amVal;
	}

	if( total >= Silence ) {
		m_total = Silence;
	}
	else {
		m_total = total;
	}
	return m_total;
}

}
