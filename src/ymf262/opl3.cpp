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

#include "opl3.h"
#include <stream/abstractarchive.h>

namespace opl
{

namespace
{
// OPL3-wide registers offsets:
constexpr int _1_NTS1_6_Offset = 0x08;
constexpr int DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1_Offset = 0xBD;
constexpr int _7_NEW1_Offset = 0x105;
constexpr int _2_CONNECTIONSEL6_Offset = 0x104;

// The OPL3 tremolo repetition rate is 3.7 Hz.
constexpr int TremoloTableLength = 13 * 1024;
}


Opl3::Opl3() : m_registers(), m_operators(), m_channels2op(), m_channels4op(), m_channels(), m_disabledChannel(),
	m_bassDrumChannel(), m_highHatSnareDrumChannel(), m_tomTomTopCymbalChannel(),
	m_nts( false ), m_dam( false ), m_dvb( false ), m_ryt( false ), m_bd( false ), m_sd( false ), m_tc( false ), m_hh( false ),
	m_new( false ), m_connectionsel( 0 ), m_vibratoIndex( 0 ), m_tremoloIndex( 0 ), m_rand(1)
{
	initOperators();
	initChannels2op();
	initChannels4op();
	initRhythmChannels();
	initChannels();
}

void Opl3::read(std::array<int16_t,4>* dest)
{
	std::array<int32_t, 4> outputBuffer;
	std::fill_n(outputBuffer.begin(), 4, 0);

	// If _new = 0, use OPL2 mode with 9 channels. If _new = 1, use OPL3 18 channels;
	for( int array = 0; array < ( m_new + 1 ); array++ ) {
		for( int channelNumber = 0; channelNumber < 9; channelNumber++ ) {
			// Reads output from each OPL3 channel, and accumulates it in the output buffer:
			std::array<int16_t,4> chanOutput;
			m_channels[array][channelNumber]->nextSample(&chanOutput);
			for( int outputChannelNumber = 0; outputChannelNumber < 4; outputChannelNumber++ ) {
				outputBuffer[outputChannelNumber] += chanOutput[outputChannelNumber];
			}
		}
		break;
	}
// 	std::cout << '\n';

	// Normalizes the output buffer after all channels have been added,
	// with a maximum of 18 channels,
	// and multiplies it to get the 16 bit signed output.
	if(dest) {
		for( int outputChannelNumber = 0; outputChannelNumber < 4; outputChannelNumber++ ) {
			(*dest)[outputChannelNumber] = outputBuffer[outputChannelNumber]>>1;
		}
	}

	// Advances the OPL3-wide vibrato index, which is used by
	// PhaseGenerator.getPhase() in each Operator.
	m_vibratoIndex++;
	m_vibratoIndex &= 0x1fff;
	// Advances the OPL3-wide tremolo index, which is used by
	// EnvelopeGenerator.getEnvelope() in each Operator.
	m_tremoloIndex++;
	m_tremoloIndex %= TremoloTableLength;

	// verified on real chip
	if (m_rand&1) {
		m_rand ^= 0x800302;
	}
	m_rand >>= 1;
}

void Opl3::write( int array, int address, uint8_t data )
{
	// The OPL3 has two registers arrays, each with adresses ranging
	// from 0x00 to 0xF5.
	// This emulator uses one array, with the two original register arrays
	// starting at 0x00 and at 0x100.
	int registerAddress = ( array << 8 ) | address;
	// If the address is out of the OPL3 memory map, returns.
	if( registerAddress < 0 || registerAddress >= 0x200 ) return;

	m_registers[registerAddress] = data;
	switch( address & 0xE0 ) {
			// The first 3 bits masking gives the type of the register by using its base address:
			// 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
			// When it is needed, we further separate the register type inside each base address,
			// which is the case of 0x00 and 0xA0.

			// Through out this emulator we will use the same name convention to
			// reference a byte with several bit registers.
			// The name of each bit register will be followed by the number of bits
			// it occupies inside the byte.
			// Numbers without accompanying names are unused bits.
		case 0x00:
			// ARC_CONTROL
			// Unique registers for the entire OPL3:
			if( array == 1 ) {
				if( address == 0x04 )
					update_2_CONNECTIONSEL6();
				else if( address == 0x05 )
					update_7_NEW1();
			}
			else if( address == 0x08 )
				update_1_NTS1_6();
			break;

		case 0xA0:
			// ARC_FREQ_NUM
			// 0xBD is a control register for the entire OPL3:
			if( address == 0xBD ) {
				if( array == 0 )
					update_DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1();
				break;
			}
			// Registers for each channel are in A0-A8, B0-B8, C0-C8, in both register arrays.
			// 0xB0...0xB8 keeps kon,block,fnum(h) for each channel.
			if( ( address & 0xF0 ) == 0xB0 && address <= 0xB8 ) {
				// If the address is in the second register array, adds 9 to the channel number.
				// The channel number is given by the last four bits, like in A0,...,A8.
				m_channels[array][address & 0x0F]->update_2_KON1_BLOCK3_FNUMH2();
				break;
			}
			// 0xA0...0xA8 keeps fnum(l) for each channel.
			if( ( address & 0xF0 ) == 0xA0 && address <= 0xA8 )
				m_channels[array][address & 0x0F]->update_FNUML8();
			break;
			// 0xC0...0xC8 keeps cha,chb,chc,chd,fb,cnt for each channel:
		case 0xC0:
			// ARC_FEEDBACK
			if( address <= 0xC8 )
				m_channels[array][address & 0x0F]->update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();
			break;

			// Registers for each of the 36 Operators:
		default:
			int operatorOffset = address & 0x1F;
			if( m_operators[array][operatorOffset] == nullptr )
				break;
			switch( address & 0xE0 ) {
				case 0x20:
					// ARC_TVS_KSR_MUL
					// 0x20...0x35 keeps am,vib,egt,ksr,mult for each operator:
					m_operators[array][operatorOffset]->update_AM1_VIB1_EGT1_KSR1_MULT4();
					break;
				case 0x40:
					// ARC_KSL_OUTLEV
					// 0x40...0x55 keeps ksl,tl for each operator:
					m_operators[array][operatorOffset]->update_KSL2_TL6();
					break;
				case 0x60:
					// 0x60...0x75 keeps ar,dr for each operator:
					// ARC_ATTR_DECR
					m_operators[array][operatorOffset]->update_AR4_DR4();
					break;
				case 0x80:
					// 0x80...0x95 keeps sl,rr for each operator:
					// ARC_SUSL_RELR
					m_operators[array][operatorOffset]->update_SL4_RR4();
					break;
				case 0xE0:
					// 0xE0...0xF5 keeps ws for each operator:
					// ARC_WAVE_SEL
					m_operators[array][operatorOffset]->update_5_WS3();
			}
	}
}
void Opl3::initOperators()
{
	for( int array = 0; array < 2; array++ ) {
		for( int group = 0; group <= 0x10; group += 8 ) {
			for( int offset = 0; offset < 6; offset++ ) {
				int baseAddress = ( array << 8 ) | ( group + offset );
				m_operators[array][group + offset].reset( new Operator( this, baseAddress ) );
			}
		}
	}
}
void Opl3::initChannels2op()
{
	for( int array = 0; array < 2; array++ ) {
		for( int channelNumber = 0; channelNumber < 3; channelNumber++ ) {
			int baseAddress = ( array << 8 ) | channelNumber;
			// Channels 1, 2, 3 -> Operator offsets 0x0,0x3; 0x1,0x4; 0x2,0x5
			m_channels2op[array][channelNumber].reset( new Channel2Op( this, baseAddress, m_operators[array][channelNumber].get(), m_operators[array][channelNumber + 0x3].get() ) );
			// Channels 4, 5, 6 -> Operator offsets 0x8,0xB; 0x9,0xC; 0xA,0xD
			m_channels2op[array][channelNumber + 3].reset( new Channel2Op( this, baseAddress + 3, m_operators[array][channelNumber + 0x8].get(), m_operators[array][channelNumber + 0xB].get() ) );
			// Channels 7, 8, 9 -> Operators 0x10,0x13; 0x11,0x14; 0x12,0x15
			m_channels2op[array][channelNumber + 6].reset( new Channel2Op( this, baseAddress + 6, m_operators[array][channelNumber + 0x10].get(), m_operators[array][channelNumber + 0x13].get() ) );
		}
	}
}
void Opl3::initChannels4op()
{
	for( int array = 0; array < 2; array++ ) {
		for( int channelNumber = 0; channelNumber < 3; channelNumber++ ) {
			int baseAddress = ( array << 8 ) | channelNumber;
			// Channels 1, 2, 3 -> Operators 0x0,0x3,0x8,0xB; 0x1,0x4,0x9,0xC; 0x2,0x5,0xA,0xD;
			m_channels4op[array][channelNumber].reset( new Channel4Op( this, baseAddress, m_operators[array][channelNumber].get(), m_operators[array][channelNumber + 0x3].get(), m_operators[array][channelNumber + 0x8].get(), m_operators[array][channelNumber + 0xB].get() ) );
		}
	}
}
void Opl3::initRhythmChannels()
{
	m_bassDrumChannel.reset( new BassDrumChannel( this ) );
	m_highHatSnareDrumChannel.reset( new HighHatSnareDrumChannel( this ) );
	m_tomTomTopCymbalChannel.reset( new TomTomTopCymbalChannel( this ) );
}
void Opl3::initChannels()
{
	// Channel is an abstract class that can be a 2-op, 4-op, rhythm or disabled channel,
	// depending on the OPL3 configuration at the time.
	// channels[] inits as a 2-op serial channel array:
	for( int array = 0; array < 2; array++ ) {
		for( int i = 0; i < 9; i++ ) {
			m_channels[array][i] = m_channels2op[array][i];
		}
	}

	// Unique instance to fill future gaps in the Channel array,
	// when there will be switches between 2op and 4op mode.
	m_disabledChannel.reset( new DisabledChannel( this ) );
}
void Opl3::update_1_NTS1_6()
{
	// Note Selection. This register is used in Channel.updateOperators() implementations,
	// to calculate the channel´s Key Scale Number.
	// The value of the actual envelope rate follows the value of
	// OPL3.nts,Operator.keyScaleNumber and Operator.ksr
	m_nts = m_registers[_1_NTS1_6_Offset] & 0x40;
}
void Opl3::update_DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1()
{
	const uint8_t dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 = m_registers[DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1_Offset];
	m_dam = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x80;
	m_dvb = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x40;

	bool new_ryt = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x20;
	if( new_ryt != m_ryt ) {
		m_ryt = new_ryt;
		setRhythmMode();
	}

	bool new_bd  = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x10;
	if( new_bd != m_bd ) {
		m_bd = new_bd;
		if( m_bd ) {
			m_bassDrumChannel->op1()->keyOn();
			m_bassDrumChannel->op2()->keyOn();
		}
	}

	bool new_sd  = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x08;
	if( new_sd != m_sd ) {
		m_sd = new_sd;
		if( m_sd ) {
			snareDrumOperator()->keyOn();
		}
	}
	
	if( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x04 ) {
		tomTomOperator()->keyOn();
	}

	bool new_tc  = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x02;
	if( new_tc != m_tc ) {
		m_tc = new_tc;
		if( m_tc )
			topCymbalOperator()->keyOn();
	}

	bool new_hh  = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x01;
	if( new_hh != m_hh ) {
		m_hh = new_hh;
		if( m_hh )
			highHatOperator()->keyOn();
	}

}
void Opl3::update_7_NEW1()
{
	int _7_new1 = m_registers[_7_NEW1_Offset];
	m_new = _7_new1 & 0x01;
	if( m_new )
		setEnabledChannels();
	set4opConnections();
}
void Opl3::setEnabledChannels()
{
	for( int array = 0; array < 2; array++ )
		for( int i = 0; i < 9; i++ ) {
			int baseAddress = m_channels[array][i]->baseAddress();
			m_registers[baseAddress + AbstractChannel::CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset] |= 0xF0;
			m_channels[array][i]->update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();
		}
}
void Opl3::update_2_CONNECTIONSEL6()
{
	// This method is called only if _new is set.
	int _2_connectionsel6 = m_registers[_2_CONNECTIONSEL6_Offset];
	m_connectionsel = ( _2_connectionsel6 & 0x3F );
	set4opConnections();
}
void Opl3::set4opConnections()
{

	// bits 0, 1, 2 sets respectively 2-op channels (1,4), (2,5), (3,6) to 4-op operation.
	// bits 3, 4, 5 sets respectively 2-op channels (10,13), (11,14), (12,15) to 4-op operation.
	for( int array = 0; array < 2; array++ )
		for( int i = 0; i < 3; i++ ) {
			if( m_new ) {
				int shift = array * 3 + i;
				int connectionBit = ( m_connectionsel >> shift ) & 0x01;
				if( connectionBit == 1 ) {
					m_channels[array][i] = m_channels4op[array][i];
					m_channels[array][i + 3] = m_disabledChannel;
					m_channels[array][i]->updateChannel();
					continue;
				}
			}
			m_channels[array][i] = m_channels2op[array][i];
			m_channels[array][i + 3] = m_channels2op[array][i + 3];
			m_channels[array][i]->updateChannel();
			m_channels[array][i + 3]->updateChannel();
		}
}
void Opl3::setRhythmMode()
{
	if( m_ryt ) {
		m_channels[0][6] = m_bassDrumChannel;
		m_channels[0][7] = m_highHatSnareDrumChannel;
		m_channels[0][8] = m_tomTomTopCymbalChannel;
	}
	else {
		for( int i = 6; i < 9; i++ ) {
			m_channels[0][i] = m_channels2op[0][i];
		}
	}
	for( int i = 6; i <= 8; i++ ) {
		m_channels[0][i]->updateChannel();
	}
}

AbstractArchive& Opl3::serialize( AbstractArchive* archive )
{
	archive->array(m_registers, 512);
	if( archive->isLoading() ) {
		for(int i=0; i<512; i++) {
			// update all operators and channels...
			//! @todo This is pretty bad, but better than nothing...
			writeReg(i, m_registers[i]);
		}
	}
	return *archive;
}

}