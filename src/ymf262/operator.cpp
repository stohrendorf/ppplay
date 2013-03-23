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

#include "operator.h"
#include "opl3.h"

#include <boost/assert.hpp>

namespace opl
{
void Operator::update_AM1_VIB1_EGT1_KSR1_MULT4()
{

	uint8_t am1_vib1_egt1_ksr1_mult4 = m_opl->readReg( m_operatorBaseAddress + Operator::AM1_VIB1_EGT1_KSR1_MULT4_Offset );

	m_am  = am1_vib1_egt1_ksr1_mult4 & 0x80;
	m_vib = am1_vib1_egt1_ksr1_mult4 & 0x40;
	m_egt = am1_vib1_egt1_ksr1_mult4 & 0x20;
	m_ksr = am1_vib1_egt1_ksr1_mult4 & 0x10;
	m_mult = am1_vib1_egt1_ksr1_mult4 & 0x0F;
	
	m_phaseGenerator.setFrequency( m_f_number, m_block, m_mult );
	m_envelopeGenerator.setKsr( m_ksr );
	m_envelopeGenerator.setAttackRate( m_ar );
	m_envelopeGenerator.setDecayRate( m_dr );
	m_envelopeGenerator.setReleaseRate( m_rr );
}

void Operator::update_KSL2_TL6()
{

	const uint8_t ksl2_tl6 = m_opl->readReg( m_operatorBaseAddress + Operator::KSL2_TL6_Offset );

	m_ksl = ( ksl2_tl6 >> 6 ) & 3;
	m_tl  =  ksl2_tl6 & 0x3F;

	m_envelopeGenerator.setAttennuation( m_f_number, m_block, m_ksl );
	m_envelopeGenerator.setTotalLevel( m_tl );
}

void Operator::update_AR4_DR4()
{
	const uint8_t ar4_dr4 = m_opl->readReg( m_operatorBaseAddress + Operator::AR4_DR4_Offset );

	m_ar = ( ar4_dr4 >> 4 ) & 0x0f;
	m_dr =  ar4_dr4 & 0x0F;

	m_envelopeGenerator.setAttackRate( m_ar );
	m_envelopeGenerator.setDecayRate( m_dr );
}

void Operator::update_SL4_RR4()
{

	const uint8_t sl4_rr4 = m_opl->readReg( m_operatorBaseAddress + Operator::SL4_RR4_Offset );

	m_sl = ( sl4_rr4 >> 4 ) & 0x0f;
	m_rr =  sl4_rr4 & 0x0F;

	m_envelopeGenerator.setActualSustainLevel( m_sl );
	m_envelopeGenerator.setReleaseRate( m_rr );
}

void Operator::update_5_WS3()
{
	m_ws = m_opl->readReg( m_operatorBaseAddress + Operator::_5_WS3_Offset ) & 0x07;
}

int16_t Operator::handleTopCymbal()
{
	// The Top Cymbal operator uses his own phase together with the High Hat phase.
	const uint16_t highHatPhase = m_opl->highHatOperator()->m_phase;
	const uint16_t phaseBit = (((m_phase & 0x88) ^ ((m_phase<<5) & 0x80)) | ((highHatPhase ^ (highHatPhase<<2)) & 0x20)) ? 0x02 : 0x00;
	
	m_envelopeGenerator.advance( m_egt, m_am );

	m_phase = (1+phaseBit)<<8;

	uint8_t waveform = m_ws;
	if( m_opl->isNew() ) {
		waveform &= 0x07;
	}
	else {
		waveform &= 0x03;
	}
	
	return getOutput(m_phase, waveform);
}

int16_t Operator::handleHighHat(uint16_t modulator)
{
	const uint16_t cymbalPhase = m_opl->topCymbalOperator()->m_phase;
	const uint16_t phasebit = ( ( ( m_phase & 0x88 ) ^( ( m_phase << 5 ) & 0x80 ) ) | ( ( cymbalPhase ^( cymbalPhase << 2 ) ) & 0x20 ) ) ? 0x02 : 0x00;
	const uint16_t noisebit = m_opl->randBit();
	m_phase = (( phasebit << 8 ) | ( 0x34 << ( phasebit ^( noisebit << 1 ) ) ));
	m_envelopeGenerator.advance( m_egt, m_am );
	return getOutput( modulator + m_phase, m_ws );
}

int16_t Operator::handleSnareDrum( uint16_t modulator )
{
	if( m_envelopeGenerator.isOff() ) {
		return 0;
	}

	m_envelopeGenerator.advance( m_egt, m_am );

	uint8_t phaseBit = ( m_phase >> 9 ) & 1;
	uint8_t noiseBit = m_opl->randBit();
	m_phase = ( ( 1 + phaseBit ) ^ noiseBit ) << 1;
	return getOutput( modulator + (m_phase<<1), m_ws );
}

int16_t Operator::nextSample( uint16_t modulator )
{
	if( m_opl->ryt() ) {
		// here the rythm mode is handled
		static constexpr int HighHatOperatorBaseAddress = 0x11;
		static constexpr int TopCymbalOperatorBaseAddress = 0x15;
		static constexpr int SnareDrumOperatorBaseAddress = 0x14;
		
		switch(m_operatorBaseAddress) {
			case TopCymbalOperatorBaseAddress:
				return handleTopCymbal();
			case HighHatOperatorBaseAddress:
				return handleHighHat(modulator);
			case SnareDrumOperatorBaseAddress:
				return handleSnareDrum(modulator);
		}
	}
	
	if( m_envelopeGenerator.isOff() ) {
		return 0;
	}

	m_envelopeGenerator.advance( m_egt, m_am );

	uint8_t ws = m_ws;
	// If it is in OPL2 mode, use first four waveforms only:
	if( m_opl->isNew() ) {
		ws &= 0x07;
	}
	else {
		ws &= 0x03;
	}

	m_phase = m_phaseGenerator.advance( m_vib );

	return getOutput( modulator + m_phase, ws );
}

void Operator::keyOn()
{
	if( m_ar > 0 ) {
		m_envelopeGenerator.keyOn();
		m_phaseGenerator.keyOn();
	}
	else {
		m_envelopeGenerator.setOff();
	}
}

void Operator::keyOff()
{
	m_envelopeGenerator.keyOff();
}

void Operator::updateOperator( uint16_t f_num, uint8_t blk )
{
	m_f_number = f_num&0x3ff;
	m_block = blk&0x07;
	update_AM1_VIB1_EGT1_KSR1_MULT4();
	update_KSL2_TL6();
	update_AR4_DR4();
	update_SL4_RR4();
	update_5_WS3();
}

Operator::Operator( Opl3* opl, int baseAddress ) : m_opl( opl ), m_operatorBaseAddress( baseAddress ), m_phaseGenerator( opl ), m_envelopeGenerator( opl ),
	m_phase(0), m_am( false ), m_vib( false ), m_ksr( false ), m_egt( false ), m_mult( 0 ), m_ksl( 0 ), m_tl( 0 ),
	m_ar( 0 ), m_dr( 0 ), m_sl( 0 ), m_rr( 0 ), m_ws( 0 ),
	m_f_number( 0 ), m_block( 0 )
{
}

namespace
{
constexpr uint16_t sinLogTable[256] = {
	2137, 1731, 1543, 1419, 1326, 1252, 1190, 1137, 1091, 1050, 1013, 979, 949, 920, 894, 869,
	846, 825, 804, 785, 767, 749, 732, 717, 701, 687, 672, 659, 646, 633, 621, 609,
	598, 587, 576, 566, 556, 546, 536, 527, 518, 509, 501, 492, 484, 476, 468, 461,
	453, 446, 439, 432, 425, 418, 411, 405, 399, 392, 386, 380, 375, 369, 363, 358,
	352, 347, 341, 336, 331, 326, 321, 316, 311, 307, 302, 297, 293, 289, 284, 280,
	276, 271, 267, 263, 259, 255, 251, 248, 244, 240, 236, 233, 229, 226, 222, 219,
	215, 212, 209, 205, 202, 199, 196, 193, 190, 187, 184, 181, 178, 175, 172, 169,
	167, 164, 161, 159, 156, 153, 151, 148, 146, 143, 141, 138, 136, 134, 131, 129,
	127, 125, 122, 120, 118, 116, 114, 112, 110, 108, 106, 104, 102, 100, 98, 96,
	94, 92, 91, 89, 87, 85, 83, 82, 80, 78, 77, 75, 74, 72, 70, 69,
	67, 66, 64, 63, 62, 60, 59, 57, 56, 55, 53, 52, 51, 49, 48, 47,
	46, 45, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30,
	29, 28, 27, 26, 25, 24, 23, 23, 22, 21, 20, 20, 19, 18, 17, 17,
	16, 15, 15, 14, 13, 13, 12, 12, 11, 10, 10, 9, 9, 8, 8, 7,
	7, 7, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2,
	2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0
};
constexpr uint16_t sinExpTable[256] = {
	0, 3, 6, 8, 11, 14, 17, 20, 22, 25, 28, 31, 34, 37, 40, 42,
	45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90,
	93, 96, 99, 102, 105, 108, 111, 114, 117, 120, 123, 126, 130, 133, 136, 139,
	142, 145, 148, 152, 155, 158, 161, 164, 168, 171, 174, 177, 181, 184, 187, 190,
	194, 197, 200, 204, 207, 210, 214, 217, 220, 224, 227, 231, 234, 237, 241, 244,
	248, 251, 255, 258, 262, 265, 268, 272, 276, 279, 283, 286, 290, 293, 297, 300,
	304, 308, 311, 315, 318, 322, 326, 329, 333, 337, 340, 344, 348, 352, 355, 359,
	363, 367, 370, 374, 378, 382, 385, 389, 393, 397, 401, 405, 409, 412, 416, 420,
	424, 428, 432, 436, 440, 444, 448, 452, 456, 460, 464, 468, 472, 476, 480, 484,
	488, 492, 496, 501, 505, 509, 513, 517, 521, 526, 530, 534, 538, 542, 547, 551,
	555, 560, 564, 568, 572, 577, 581, 585, 590, 594, 599, 603, 607, 612, 616, 621,
	625, 630, 634, 639, 643, 648, 652, 657, 661, 666, 670, 675, 680, 684, 689, 693,
	698, 703, 708, 712, 717, 722, 726, 731, 736, 741, 745, 750, 755, 760, 765, 770,
	774, 779, 784, 789, 794, 799, 804, 809, 814, 819, 824, 829, 834, 839, 844, 849,
	854, 859, 864, 869, 874, 880, 885, 890, 895, 900, 906, 911, 916, 921, 927, 932,
	937, 942, 948, 953, 959, 964, 969, 975, 980, 986, 991, 996, 1002, 1007, 1013, 1018
};

/*
                 /-\
                |   |
Waveform 1      +   +   +  ABCD
                    |   |
                     \-/

                 /-\
                |   |
Waveform 2      +   +---+  ABCX

                 /-\ /-\
                |   |   |
Waveform 3      +   +   +  ABAB

                 /+  /+
                | | | |
Waveform 4      + +-+ +--  AXAX

                 ^ ^
                | | |
Waveform 5      +-+-+----  EEXX

                 ^
                | |
Waveform 6      +-+-+----  EFXX
                  | |
                   v

                +---+
                |   |
Waveform 7      +   +   +  GGHH
                    |   |
                    +---+

                |\
                | \
Waveform 8      +  ---  +  IJKL
                      \ |
                       \|
*/
/**
 * @brief Calculate logarithmic sine value
 * @param[in] ws Waveform selector (3 bits)
 * @param[in] phi Phase (10 bits used)
 * @return Logarithmic value, 0..2137, bit 15 is the sign bit
 */
uint16_t sinLog( uint8_t ws, uint16_t phi )
{
	uint8_t index = phi;

	switch( ws | ( phi & 0x0300 ) ) {
			// rising quarter wave  Shape A
		case 0x0000:
		case 0x0001:
		case 0x0002:
		case 0x0202:
		case 0x0003:
		case 0x0203:
			return sinLogTable[index];

			// falling quarter wave  Shape B
		case 0x0100:
		case 0x0101:
		case 0x0102:
		case 0x0302:
			return sinLogTable[index ^ 0xFF];

			// rising quarter wave -ve  Shape C
		case 0x0200:
			return sinLogTable[index] | 0x8000;

			// falling quarter wave -ve  Shape D
		case 0x0300:
			return sinLogTable[index ^ 0xFF] | 0x8000;

			// fast wave +ve  Shape E
		case 0x0004:
		case 0x0005:
		case 0x0105:
			return sinLogTable[( ( index << 1 ) ^( ( index & 0x80 ) ? 0x1FF : 0x00 ) )];

			// fast wave -ve  Shape F
		case 0x0104:
			return sinLogTable[( ( index << 1 ) ^( ( index & 0x80 ) ? 0x1FF : 0x00 ) )] | 0x8000;

			// square wave +ve  Shape G
		case 0x0006:
		case 0x0106:
			return 0;

			// square wave -ve  Shape H
		case 0x0206:
		case 0x0306:
			return 0x8000;

			// Shape I
		case 0x0007:
			return index << 3;

			// Shape J
		case 0x0107:
			return index << 3 | 0x800;

			// Shape K
		case 0x0207:
			return ( index ^ 0xFF ) << 3 | 0x8800;

			// Shape L
		case 0x0307:
			return ( index ^ 0xFF ) << 3 | 0x8000;
	}
	// Shape X
	return 0x0C00;
}

/*
Below is code to convert by signed logarithmic wave form lookup to a linear output as a signed short integer.
The input parameter is the output of the waveform lookup plus any attenuation. Every 256 added to the input, represents a halving of the linear output (-3dB).
*/
/**
 * @brief Calculate exponential value from logarithmic value
 * @param[in] expVal Exponent calculated by sinLogWs
 */
int16_t sinExp( uint16_t expVal )
{
	const bool signBit = expVal & 0x8000;

	expVal &= 0x7FFF;
	// expVal: 0..2137+511*8 = 0..6225
	// result: 0..1018+1024
	uint32_t result = 0x0400 + sinExpTable[( expVal & 0xff ) ^ 0xFF];
	result <<= 1;
	result >>= ( expVal >> 8 ); // exp

	if( signBit ) {
		return -result-1;
	}
	else {
		return result;
	}
}

// 16 env units are ~3dB and halve the output
int16_t oplSin( uint8_t ws, uint16_t phase, uint16_t env )
{
	return sinExp( sinLog( ws, phase ) + (env<<3) );
}

}

int16_t Operator::getOutput( uint16_t outputPhase, uint8_t ws )
{
	if( m_envelopeGenerator.isSilent() || m_envelopeGenerator.isOff() ) {
		return 0;
	}
	return oplSin( ws, outputPhase, m_envelopeGenerator.envelope() );
	//return waveform[int( outputPhase + modulator + 1024 ) % 1024] * m_envelope;
}

light4cxx::Logger* Operator::logger()
{
	return light4cxx::Logger::get("opl.operator");
}

}