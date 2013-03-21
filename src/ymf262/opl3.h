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

#ifndef PPP_OPL_OPL3_H
#define PPP_OPL_OPL3_H

#include <cstdint>
#include <array>

#include "operator.h"
#include "channel2op.h"
#include "channel4op.h"
#include "bassdrumchannel.h"
#include "highhatsnaredrumchannel.h"
#include "tomtomtopcymbalchannel.h"
#include "disabledchannel.h"
#include <stream/iserializable.h>

#include <ymf262/ppplay_opl_export.h>

namespace opl
{
class PPPLAY_OPL_EXPORT Opl3 : public ISerializable
{
public:
	static constexpr int MasterClock = 14.32e6;
	static constexpr int SampleRate = MasterClock / 288;

private:
	uint8_t m_registers[0x200];

	// The YMF262 has 36 operators:
	Operator::Ptr m_operators[2][36];
	// The YMF262 has 18 2-op channels.
	// Each 2-op channel can be at a serial or parallel operator configuration:
	Channel2Op::Ptr m_channels2op[2][9];
	Channel4Op::Ptr m_channels4op[2][3];
	AbstractChannel::Ptr m_channels[2][9];
	DisabledChannel::Ptr m_disabledChannel;

	BassDrumChannel::Ptr m_bassDrumChannel;
	HighHatSnareDrumChannel::Ptr m_highHatSnareDrumChannel;
	TomTomTopCymbalChannel::Ptr m_tomTomTopCymbalChannel;

	bool m_nts;
	//! @brief Depth of amplitude. This register is used in EnvelopeGenerator.getEnvelope();
	bool m_dam;
	//! @brief Depth of vibrato. This register is used in PhaseGenerator.getPhase();
	bool m_dvb;
	bool m_ryt;
	bool m_bd, m_sd, m_tc, m_hh;
	//! @brief OPL2/OPL3 mode selection. This register is used in OPL3.read(), OPL3.write() and Operator.getOperatorOutput();
	bool m_new;
	//! @brief 2-op/4-op channel selection. This register is used here to configure the OPL3.channels[] array.
	uint8_t m_connectionsel;
	//! @brief 13 bits
	uint16_t m_vibratoIndex;
	//! @brief 14 bits, wraps around after 13*1024
	uint16_t m_tremoloIndex;
	//! @brief Random number generator
	uint32_t m_rand;

public:
	uint32_t randBit() const {
		return m_rand&1;
	}
	bool isNew() const {
		return m_new;
	}
	uint8_t readReg( uint16_t index ) const {
		return m_registers[index];
	}
	void writeReg( uint16_t index, uint8_t val ) {
		write((index>>8) & 1, index&0xff, val);
	}
	bool nts() const {
		return m_nts;
	}
	Operator* topCymbalOperator() const {
		return m_operators[0][0x15].get();
	}
	Operator* highHatOperator() const {
		return m_operators[0][0x11].get();
	}
	Operator* snareDrumOperator() const {
		return m_operators[0][0x14].get();
	}
	Operator* tomTomOperator() const {
		return m_operators[0][0x12].get();
	}
	bool dvb() const {
		return m_dvb;
	}
	uint16_t vibratoIndex() const {
		return m_vibratoIndex;
	}
	bool dam() const {
		return m_dam;
	}
	bool ryt() const {
		return m_ryt;
	}
	uint16_t tremoloIndex() const {
		return m_tremoloIndex;
	}

	void read( std::array< int16_t, 4 >* dest ) ;

	Opl3();

	AbstractArchive& serialize( AbstractArchive* archive );
private:
	void initOperators() ;
	void initChannels2op() ;
	void initChannels4op() ;
	void initRhythmChannels() ;
	void initChannels() ;
	void update_1_NTS1_6() ;
	void update_DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1() ;
	void update_7_NEW1() ;
	void setEnabledChannels() ;
	void update_2_CONNECTIONSEL6() ;
	void set4opConnections() ;
	void setRhythmMode() ;
	void write( int array, int address, uint8_t data ) ;
};
}

#endif
