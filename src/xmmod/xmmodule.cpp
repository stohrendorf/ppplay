/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @ingroup XmModule
 * @{
 */

#include <boost/exception/all.hpp>

#include "xmmodule.h"
#include "xmorder.h"
#include "xmchannel.h"
#include "xmcell.h"
#include "xmpattern.h"
#include "xminstrument.h"

#include "stream/fbinstream.h"
#include "stuff/moduleregistry.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace ppp
{
namespace xm
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#pragma pack(push,1)
struct XmHeader {
	char id[17]; // "Extended Module: "
	char title[20];
	uint8_t endOfFile; // 0x1a
	char trackerName[20];
	uint16_t version;
	// offsetof(headerSize) == 60
	uint32_t headerSize;
	uint16_t songLength;
	uint16_t restartPos;
	uint16_t numChannels;
	uint16_t numPatterns;
	uint16_t numInstruments;
	uint16_t flags;
	uint16_t defaultSpeed;
	uint16_t defaultTempo;
	//uint8_t orders[256];
};
#pragma pack(pop)

static constexpr std::array<const uint16_t, 12 * 8> g_PeriodTable = {{
		907, 900, 894, 887, 881, 875, 868, 862, 856, 850, 844, 838, 832, 826, 820, 814,
		808, 802, 796, 791, 785, 779, 774, 768, 762, 757, 752, 746, 741, 736, 730, 725,
		720, 715, 709, 704, 699, 694, 689, 684, 678, 675, 670, 665, 660, 655, 651, 646,
		640, 636, 632, 628, 623, 619, 614, 610, 604, 601, 597, 592, 588, 584, 580, 575,
		570, 567, 563, 559, 555, 551, 547, 543, 538, 535, 532, 528, 524, 520, 516, 513,
		508, 505, 502, 498, 494, 491, 487, 484, 480, 477, 474, 470, 467, 463, 460, 457
	}
};
#endif

XmModule::XmModule( int maxRpt ):
	GenModule( maxRpt ),
	m_amiga( false ), m_patterns(), m_instruments(), m_channels(),
	m_noteToPeriod(), m_jumpRow( ~0 ), m_jumpOrder( ~0 ),
	m_isPatLoop( false ), m_doPatJump( false ), m_restartPos( 0 ),
	m_currentPatternDelay( 0 ), m_requestedPatternDelay( 0 )
{
}

XmModule::~XmModule()
{
	deleteAll(m_channels);
	deleteAll(m_patterns);
	deleteAll(m_instruments);
}

bool XmModule::load( const std::string& filename )
{
	XmHeader hdr;
	FBinStream file( filename );
	metaInfo().filename = filename;
	file.read( reinterpret_cast<char*>( &hdr ), sizeof( hdr ) );
	if( !std::equal( hdr.id, hdr.id + 17, "Extended Module: " ) || hdr.endOfFile != 0x1a /*|| hdr.numChannels > 32*/ ) {
		logger()->warn( L4CXX_LOCATION, "XM Header invalid" );
		return false;
	}
	if( hdr.version != 0x0104 ) {
		logger()->warn( L4CXX_LOCATION, "Unsupported XM Version %#x", hdr.version );
		return false;
	}
	for( int i = 0; i < ( hdr.songLength & 0xff ); i++ ) {
		uint8_t tmp;
		file.read( &tmp );
		addOrder( new XmOrder( tmp ) );
	}
	file.seek( hdr.headerSize + offsetof( XmHeader, headerSize ) );
	{
		metaInfo().title = boost::algorithm::trim_copy( stringncpy( hdr.title, 20 ) );
	}
	m_restartPos = hdr.restartPos;
	{
		std::string tmp = boost::algorithm::trim_copy( stringncpy( hdr.trackerName, 20 ) );
		if( !tmp.empty() ) {
			metaInfo().trackerInfo = tmp;
		}
		else {
			metaInfo().trackerInfo = "<unknown>";
		}
	}
	setTempo( hdr.defaultTempo & 0xff );
	setSpeed( hdr.defaultSpeed & 0xff );
	state().globalVolume = 0x40;
	m_amiga = ( hdr.flags & 1 ) == 0;
	m_channels.clear();
	for( int i = 0; i < hdr.numChannels; i++ )
		m_channels.push_back( new XmChannel( this ) );
	for( uint_fast16_t i = 0; i < hdr.numPatterns; i++ ) {
		XmPattern* pat = new XmPattern( hdr.numChannels );
		m_patterns.push_back( pat );
		if( !pat->load( file ) ) {
			logger()->error( L4CXX_LOCATION, "Pattern loading error" );
			return false;
		}
	}
	while( m_patterns.size() < 256 ) {
		m_patterns.push_back( XmPattern::createDefaultPattern( hdr.numChannels ) );
	}
	for( uint_fast16_t i = 0; i < hdr.numInstruments; i++ ) {
		XmInstrument* ins = new XmInstrument();
		m_instruments.push_back( ins );
		if( !ins->load( file ) ) {
			logger()->error( L4CXX_LOCATION, "Instrument loading error" );
			return false;
		}
	}
	if( m_amiga ) {
		logger()->debug( L4CXX_LOCATION, "Initializing Amiga period table" );
		uint16_t destOfs = 0;
		for( int octave = 10; octave > 0; octave-- ) {
			uint16_t octaveMask = ~( 0xffff << ( 10 - octave ) );
			for( uint16_t amigaVal : g_PeriodTable ) {
				amigaVal = ( ( amigaVal << 6 ) + octaveMask ) >> ( 11 - octave );
				m_noteToPeriod.at( destOfs ) = m_noteToPeriod.at( destOfs + 1 ) = amigaVal;
				destOfs += 2;
			}
		}
		for( size_t i = 0; i < (m_noteToPeriod.size()-1)/2; i++ ) {
			m_noteToPeriod.at( i * 2 + 1 ) = ( m_noteToPeriod.at( i * 2 + 0 ) + m_noteToPeriod.at( i * 2 + 2 ) ) >> 1;
		}
	}
	else {
		logger()->debug( L4CXX_LOCATION, "Initializing linear frequency table" );
		uint16_t val = 121 * 16;
		for( size_t i = 0; i < m_noteToPeriod.size(); i++ ) {
			m_noteToPeriod[ i ] = val * 4;
			val--;
		}
	}
	state().pattern = orderAt( 0 )->index();
// 	addMultiSong( StateIterator() );
// 	multiSongAt( 0 ).newState()->archive( this ).finishSave();
// 	multiTrackAt(0).startOrder = playbackInfo().order;
	return true;
}

size_t XmModule::internal_buildTick( AudioFrameBuffer* buffer )
{
	if( buffer && !buffer->get() ) {
		buffer->reset( new AudioFrameBuffer::element_type );
	}
	if( buffer ) {
		MixerFrameBuffer mixerBuffer( new MixerFrameBuffer::element_type( tickBufferLength() ) );
		XmPattern* currPat = m_patterns.at( state().pattern );
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			XmChannel* chan = m_channels[ currTrack ];
			BOOST_ASSERT( chan!=nullptr );
			XmCell* cell = currPat->cellAt( currTrack, state().row );
			chan->update( cell, false );
			chan->mixTick( &mixerBuffer );
		}
		buffer->get()->resize( mixerBuffer->size() );
		MixerSampleFrame* mixerBufferPtr = &mixerBuffer->front();
		BasicSampleFrame* bufPtr = &buffer->get()->front();
		for( size_t i = 0; i < mixerBuffer->size(); i++ ) {  // postprocess...
			*bufPtr = mixerBufferPtr->rightShiftClip( 2 );
			bufPtr++;
			mixerBufferPtr++;
		}
	}
	else {
		XmPattern* currPat = m_patterns.at( state().pattern );
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			XmChannel* chan = m_channels[ currTrack ];
			BOOST_ASSERT( chan!=nullptr );
			XmCell* cell = currPat->cellAt( currTrack, state().row );
			chan->update( cell, true );
			chan->mixTick( nullptr );
		}
	}
	nextTick();
	if( !adjustPosition( !buffer ) ) {
		if( buffer ) {
			buffer->reset();
		}
		return 0;
	}
	state().playedFrames += tickBufferLength();
	return tickBufferLength();
}

bool XmModule::adjustPosition( bool estimateOnly )
{
	bool orderChanged = false;
	if( state().tick == 0 ) {
		if( m_requestedPatternDelay != 0 ) {
			m_currentPatternDelay = m_requestedPatternDelay;
			m_requestedPatternDelay = 0;
		}
		if( m_currentPatternDelay != 0 ) {
			m_currentPatternDelay--;
		}
		if( m_isPatLoop || m_doPatJump ) {
			if( m_isPatLoop ) {
				m_isPatLoop = false;
				setRow( m_jumpRow );
			}
			if( m_doPatJump ) {
				setRow( m_jumpRow );
				m_jumpRow = 0;
				m_doPatJump = false;
				m_jumpOrder++;
				if( m_jumpOrder >= orderCount() ) {
					m_jumpOrder = m_restartPos;
				}
				setOrder( m_jumpOrder, estimateOnly );
				orderChanged = true;
			}
		}
		else {
			if( !isRunningPatDelay() ) {
				XmPattern* currPat = m_patterns.at( state().pattern );
				setRow( ( state().row + 1 ) % currPat->numRows() );
				if( state().row == 0 ) {
// 					if( state().order + 1 >= orderCount() ) {
// 						return false;
// 					}
					setOrder( state().order + 1, estimateOnly );
					orderChanged = true;
				}
			}
		}
		m_jumpOrder = m_jumpRow = 0;
		m_doPatJump = m_isPatLoop = false;
	}
	if( state().order >= orderCount() ) {
		return false;
	}
	if( orderChanged ) {
		state().pattern = orderAt( state().order )->index();
		setOrder( state().order, estimateOnly );
	}
	if( orderAt( state().order )->playbackCount() >= maxRepeat() ) {
		return false;
	}
	return true;
}

std::string XmModule::internal_channelStatus( size_t idx ) const
{
	return m_channels.at( idx )->statusString();
}

std::string XmModule::internal_channelCellString( size_t idx ) const
{
	XmChannel* x = m_channels.at( idx );
	if( !x ) {
		return std::string();
	}
	return x->cellString();
}

int XmModule::internal_channelCount() const
{
	return m_channels.size();
}

const XmInstrument* XmModule::getInstrument( int idx ) const
{
	if( !inRange<int>( idx, 1, m_instruments.size() ) ) {
		return nullptr;
	}
	return m_instruments[ idx - 1 ];
}

uint16_t XmModule::noteToPeriod( uint8_t note, int8_t finetune ) const
{
	uint16_t tuned = ( note << 4 ) + ( finetune >> 3 ) + 16; // - 16*7;
	if( tuned >= m_noteToPeriod.size() ) {
		return 0;
	}
	return clip<int>( m_noteToPeriod[ tuned ], 1, 0x7cff );
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
static constexpr std::array<const uint32_t, 12 * 16 * 4> g_linearMult = {{
		0x1000000, 0x1003B2D, 0x1007667, 0x100B1B0, 0x100ED06,
		0x1012869, 0x10163DB, 0x1019F5A, 0x101DAE7, 0x1021681,
		0x102522A, 0x1028DE0, 0x102C9A4, 0x1030576, 0x1034155,
		0x1037D43, 0x103B93E, 0x103F547, 0x104315F, 0x1046D84,
		0x104A9B6, 0x104E5F7, 0x1052246, 0x1055EA3, 0x1059B0D,
		0x105D786, 0x106140C, 0x10650A1, 0x1068D43, 0x106C9F4,
		0x10706B3, 0x107437F, 0x107805A, 0x107BD43, 0x107FA3A,
		0x108373E, 0x1087452, 0x108B173, 0x108EEA2, 0x1092BDF,
		0x109692B, 0x109A685, 0x109E3ED, 0x10A2163, 0x10A5EE7,
		0x10A9C7A, 0x10ADA1A, 0x10B17CA, 0x10B5587, 0x10B9352,
		0x10BD12C, 0x10C0F14, 0x10C4D0B, 0x10C8B10, 0x10CC923,
		0x10D0744, 0x10D4574, 0x10D83B2, 0x10DC1FF, 0x10E005A,
		0x10E3EC3, 0x10E7D3B, 0x10EBBC1, 0x10EFA56, 0x10F38F9,
		0x10F77AB, 0x10FB66B, 0x10FF53A, 0x1103417, 0x1107303,
		0x110B1FD, 0x110F106, 0x111301D, 0x1116F43, 0x111AE78,
		0x111EDBB, 0x1122D0D, 0x1126C6D, 0x112ABDC, 0x112EB5A,
		0x1132AE6, 0x1136A81, 0x113AA2B, 0x113E9E4, 0x11429AB,
		0x1146981, 0x114A966, 0x114E959, 0x115295C, 0x115696D,
		0x115A98D, 0x115E9BB, 0x11629F9, 0x1166A45, 0x116AAA1,
		0x116EB0B, 0x1172B84, 0x1176C0C, 0x117ACA3, 0x117ED48,
		0x1182DFD, 0x1186EC1, 0x118AF94, 0x118F075, 0x1193166,
		0x1197266, 0x119B374, 0x119F492, 0x11A35BF, 0x11A76FB,
		0x11AB845, 0x11AF9A0, 0x11B3B09, 0x11B7C81, 0x11BBE08,
		0x11BFF9F, 0x11C4144, 0x11C82F9, 0x11CC4BD, 0x11D0691,
		0x11D4873, 0x11D8A65, 0x11DCC66, 0x11E0E76, 0x11E5095,
		0x11E92C4, 0x11ED502, 0x11F1750, 0x11F59AC, 0x11F9C18,
		0x11FDE94, 0x120211E, 0x12063B9, 0x120A662, 0x120E91B,
		0x1212BE3, 0x1216EBB, 0x121B1A2, 0x121F499, 0x122379F,
		0x1227AB5, 0x122BDDA, 0x123010F, 0x1234453, 0x12387A7,
		0x123CB0A, 0x1240E7D, 0x1245200, 0x1249592, 0x124D934,
		0x1251CE5, 0x12560A6, 0x125A477, 0x125E857, 0x1262C47,
		0x1267047, 0x126B456, 0x126F876, 0x1273CA5, 0x12780E3,
		0x127C532, 0x1280990, 0x1284DFE, 0x128927C, 0x128D70A,
		0x1291BA7, 0x1296055, 0x129A512, 0x129E9DF, 0x12A2EBC,
		0x12A73A9, 0x12AB8A6, 0x12AFDB3, 0x12B42D0, 0x12B87FD,
		0x12BCD3A, 0x12C1287, 0x12C57E4, 0x12C9D50, 0x12CE2CD,
		0x12D285A, 0x12D6DF8, 0x12DB3A5, 0x12DF962, 0x12E3F2F,
		0x12E850D, 0x12ECAFB, 0x12F10F8, 0x12F5706, 0x12F9D25,
		0x12FE353, 0x1302992, 0x1306FE1, 0x130B640, 0x130FCAF,
		0x131432F, 0x13189BF, 0x131D05F, 0x1321710, 0x1325DD1,
		0x132A4A2, 0x132EB84, 0x1333276, 0x1337978, 0x133C08B,
		0x13407AE, 0x1344EE2, 0x1349626, 0x134DD7B, 0x13524E0,
		0x1356C56, 0x135B3DC, 0x135FB73, 0x136431A, 0x1368AD2,
		0x136D29A, 0x1371A73, 0x137625D, 0x137AA57, 0x137F262,
		0x1383A7E, 0x13882AA, 0x138CAE7, 0x1391334, 0x1395B93,
		0x139A402, 0x139EC81, 0x13A3512, 0x13A7DB3, 0x13AC665,
		0x13B0F28, 0x13B57FC, 0x13BA0E1, 0x13BE9D6, 0x13C32DC,
		0x13C7BF3, 0x13CC51B, 0x13D0E54, 0x13D579E, 0x13DA0F9,
		0x13DEA65, 0x13E33E1, 0x13E7D6F, 0x13EC70E, 0x13F10BE,
		0x13F5A7E, 0x13FA450, 0x13FEE33, 0x1403827, 0x140822C,
		0x140CC42, 0x141166A, 0x14160A2, 0x141AAEC, 0x141F546,
		0x1423FB2, 0x1428A30, 0x142D4BE, 0x1431F5E, 0x1436A0E,
		0x143B4D1, 0x143FFA4, 0x1444A89, 0x144957F, 0x144E086,
		0x1452B9F, 0x14576C9, 0x145C204, 0x1460D51, 0x14658AF,
		0x146A41F, 0x146EFA0, 0x1473B32, 0x14786D6, 0x147D28C,
		0x1481E53, 0x1486A2B, 0x148B615, 0x1490211, 0x1494E1E,
		0x1499A3D, 0x149E66D, 0x14A32AF, 0x14A7F03, 0x14ACB68,
		0x14B17DF, 0x14B6467, 0x14BB101, 0x14BFDAD, 0x14C4A6B,
		0x14C973A, 0x14CE41C, 0x14D310E, 0x14D7E13, 0x14DCB2A,
		0x14E1852, 0x14E658C, 0x14EB2D8, 0x14F0036, 0x14F4DA6,
		0x14F9B27, 0x14FE8BB, 0x1503660, 0x1508418, 0x150D1E1,
		0x1511FBD, 0x1516DAA, 0x151BBAA, 0x15209BB, 0x15257DF,
		0x152A614, 0x152F45C, 0x15342B5, 0x1539121, 0x153DF9F,
		0x1542E2F, 0x1547CD2, 0x154CB86, 0x1551A4D, 0x1556925,
		0x155B811, 0x156070E, 0x156561D, 0x156A53F, 0x156F473,
		0x15743BA, 0x1579313, 0x157E27E, 0x15831FB, 0x158818B,
		0x158D12D, 0x15920E2, 0x15970A9, 0x159C082, 0x15A106E,
		0x15A606D, 0x15AB07E, 0x15B00A1, 0x15B50D7, 0x15BA120,
		0x15BF17B, 0x15C41E8, 0x15C9269, 0x15CE2FB, 0x15D33A1,
		0x15D8459, 0x15DD524, 0x15E2601, 0x15E76F1, 0x15EC7F4,
		0x15F190A, 0x15F6A32, 0x15FBB6D, 0x1600CBB, 0x1605E1C,
		0x160AF8F, 0x1610115, 0x16152AE, 0x161A45A, 0x161F619,
		0x16247EB, 0x16299D0, 0x162EBC7, 0x1633DD2, 0x1638FEF,
		0x163E220, 0x1643463, 0x16486BA, 0x164D923, 0x1652BA0,
		0x1657E30, 0x165D0D2, 0x1662388, 0x1667651, 0x166C92D,
		0x1671C1C, 0x1676F1F, 0x167C234, 0x168155D, 0x1686899,
		0x168BBE9, 0x1690F4B, 0x16962C1, 0x169B64A, 0x16A09E6,
		0x16A5D96, 0x16AB159, 0x16B0530, 0x16B5919, 0x16BAD17,
		0x16C0127, 0x16C554B, 0x16CA983, 0x16CFDCE, 0x16D522C,
		0x16DA69E, 0x16DFB24, 0x16E4FBD, 0x16EA469, 0x16EF92A,
		0x16F4DFD, 0x16FA2E5, 0x16FF7E0, 0x1704CEE, 0x170A210,
		0x170F746, 0x1714C90, 0x171A1ED, 0x171F75F, 0x1724CE3,
		0x172A27C, 0x172F828, 0x1734DE9, 0x173A3BD, 0x173F9A5,
		0x1744FA0, 0x174A5B0, 0x174FBD3, 0x175520B, 0x175A856,
		0x175FEB5, 0x1765529, 0x176ABB0, 0x177024B, 0x17758FA,
		0x177AFBE, 0x1780695, 0x1785D80, 0x178B480, 0x1790B94,
		0x17962BB, 0x179B9F7, 0x17A1147, 0x17A68AB, 0x17AC024,
		0x17B17B1, 0x17B6F51, 0x17BC707, 0x17C1ED0, 0x17C76AE,
		0x17CCEA0, 0x17D26A6, 0x17D7EC1, 0x17DD6F0, 0x17E2F33,
		0x17E878B, 0x17EDFF8, 0x17F3878, 0x17F910D, 0x17FE9B7,
		0x1804275, 0x1809B48, 0x180F42F, 0x1814D2B, 0x181A63B,
		0x181FF60, 0x182589A, 0x182B1E8, 0x1830B4A, 0x18364C2,
		0x183BE4E, 0x18417EF, 0x18471A4, 0x184CB6F, 0x185254E,
		0x1857F41, 0x185D94A, 0x1863367, 0x1868D9A, 0x186E7E1,
		0x187423D, 0x1879CAE, 0x187F733, 0x18851CE, 0x188AC7E,
		0x1890742, 0x189621C, 0x189BD0A, 0x18A180E, 0x18A7326,
		0x18ACE54, 0x18B2997, 0x18B84EF, 0x18BE05C, 0x18C3BDE,
		0x18C9775, 0x18CF321, 0x18D4EE3, 0x18DAABA, 0x18E06A6,
		0x18E62A7, 0x18EBEBE, 0x18F1AEA, 0x18F772B, 0x18FD381,
		0x1902FED, 0x1908C6E, 0x190E905, 0x19145B1, 0x191A272,
		0x191FF49, 0x1925C35, 0x192B937, 0x193164E, 0x193737B,
		0x193D0BD, 0x1942E15, 0x1948B83, 0x194E906, 0x195469E,
		0x195A44D, 0x1960211, 0x1965FEA, 0x196BDDA, 0x1971BDF,
		0x19779F9, 0x197D82A, 0x1983670, 0x19894CC, 0x198F33E,
		0x19951C6, 0x199B064, 0x19A0F17, 0x19A6DE0, 0x19ACCC0,
		0x19B2BB5, 0x19B8AC0, 0x19BE9E1, 0x19C4918, 0x19CA865,
		0x19D07C8, 0x19D6742, 0x19DC6D1, 0x19E2676, 0x19E8632,
		0x19EE603, 0x19F45EB, 0x19FA5E9, 0x1A005FD, 0x1A06627,
		0x1A0C668, 0x1A126BE, 0x1A1872C, 0x1A1E7AF, 0x1A24848,
		0x1A2A8F8, 0x1A309BF, 0x1A36A9B, 0x1A3CB8F, 0x1A42C98,
		0x1A48DB8, 0x1A4EEEE, 0x1A5503B, 0x1A5B19E, 0x1A61318,
		0x1A674A9, 0x1A6D650, 0x1A7380D, 0x1A799E1, 0x1A7FBCC,
		0x1A85DCD, 0x1A8BFE5, 0x1A92214, 0x1A98459, 0x1A9E6B5,
		0x1AA4928, 0x1AAABB2, 0x1AB0E52, 0x1AB7109, 0x1ABD3D7,
		0x1AC36BC, 0x1AC99B8, 0x1ACFCCA, 0x1AD5FF4, 0x1ADC334,
		0x1AE268B, 0x1AE89FA, 0x1AEED7F, 0x1AF511B, 0x1AFB4CE,
		0x1B01899, 0x1B07C7A, 0x1B0E073, 0x1B14482, 0x1B1A8A9,
		0x1B20CE7, 0x1B2713C, 0x1B2D5A8, 0x1B33A2C, 0x1B39EC6,
		0x1B40378, 0x1B46841, 0x1B4CD22, 0x1B5321A, 0x1B59729,
		0x1B5FC4F, 0x1B6618D, 0x1B6C6E3, 0x1B72C4F, 0x1B791D4,
		0x1B7F76F, 0x1B85D22, 0x1B8C2ED, 0x1B928CF, 0x1B98EC9,
		0x1B9F4DA, 0x1BA5B03, 0x1BAC144, 0x1BB279C, 0x1BB8E0B,
		0x1BBF493, 0x1BC5B32, 0x1BCC1E9, 0x1BD28B8, 0x1BD8F9E,
		0x1BDF69C, 0x1BE5DB2, 0x1BEC4E0, 0x1BF2C26, 0x1BF9383,
		0x1BFFAF9, 0x1C06286, 0x1C0CA2B, 0x1C131E9, 0x1C199BE,
		0x1C201AB, 0x1C269B0, 0x1C2D1CE, 0x1C33A03, 0x1C3A250,
		0x1C40AB6, 0x1C47334, 0x1C4DBCA, 0x1C54478, 0x1C5AD3E,
		0x1C6161C, 0x1C67F13, 0x1C6E822, 0x1C75149, 0x1C7BA89,
		0x1C823E0, 0x1C88D51, 0x1C8F6D9, 0x1C9607A, 0x1C9CA34,
		0x1CA3405, 0x1CA9DF0, 0x1CB07F3, 0x1CB720E, 0x1CBDC42,
		0x1CC468E, 0x1CCB0F3, 0x1CD1B70, 0x1CD8607, 0x1CDF0B5,
		0x1CE5B7D, 0x1CEC65D, 0x1CF3156, 0x1CF9C67, 0x1D00792,
		0x1D072D5, 0x1D0DE30, 0x1D149A5, 0x1D1B533, 0x1D220D9,
		0x1D28C98, 0x1D2F871, 0x1D36462, 0x1D3D06C, 0x1D43C8F,
		0x1D4A8CB, 0x1D51520, 0x1D5818E, 0x1D5EE15, 0x1D65AB5,
		0x1D6C76F, 0x1D73441, 0x1D7A12D, 0x1D80E31, 0x1D87B4F,
		0x1D8E887, 0x1D955D7, 0x1D9C341, 0x1DA30C4, 0x1DA9E60,
		0x1DB0C16, 0x1DB79E5, 0x1DBE7CD, 0x1DC55CF, 0x1DCC3EA,
		0x1DD321F, 0x1DDA06D, 0x1DE0ED5, 0x1DE7D56, 0x1DEEBF1,
		0x1DF5AA5, 0x1DFC973, 0x1E0385B, 0x1E0A75C, 0x1E11677,
		0x1E185AB, 0x1E1F4F9, 0x1E26461, 0x1E2D3E3, 0x1E3437E,
		0x1E3B334, 0x1E42303, 0x1E492EC, 0x1E502EE, 0x1E5730B,
		0x1E5E342, 0x1E65392, 0x1E6C3FD, 0x1E73481, 0x1E7A520,
		0x1E815D8, 0x1E886AB, 0x1E8F797, 0x1E9689E, 0x1E9D9BF,
		0x1EA4AFA, 0x1EABC4F, 0x1EB2DBF, 0x1EB9F48, 0x1EC10EC,
		0x1EC82AA, 0x1ECF483, 0x1ED6676, 0x1EDD883, 0x1EE4AAA,
		0x1EEBCEC, 0x1EF2F48, 0x1EFA1BF, 0x1F01450, 0x1F086FC,
		0x1F0F9C2, 0x1F16CA2, 0x1F1DF9E, 0x1F252B3, 0x1F2C5E4,
		0x1F3392F, 0x1F3AC95, 0x1F42015, 0x1F493B0, 0x1F50766,
		0x1F57B36, 0x1F5EF22, 0x1F66328, 0x1F6D748, 0x1F74B84,
		0x1F7BFDB, 0x1F8344C, 0x1F8A8D9, 0x1F91D80, 0x1F99242,
		0x1FA0720, 0x1FA7C18, 0x1FAF12B, 0x1FB665A, 0x1FBDBA3,
		0x1FC5108, 0x1FCC688, 0x1FD3C23, 0x1FDB1D9, 0x1FE27AA,
		0x1FE9D97, 0x1FF139F, 0x1FF89C2
	}
};
#endif

uint32_t XmModule::periodToFrequency( uint16_t period ) const
{
	float pbFrq = frequency();
	constexpr float adjFac = pow( 2, -7.0f / 12 );
	if( m_amiga ) {
		return 8363.0f * 1712.0f * 65536.0f / pbFrq / period * adjFac;
	}
	else {
		uint32_t tmp = 12 * 12 * 16 * 4 - period;
		uint32_t div = 14 - tmp / ( 12 * 16 * 4 );
		uint64_t res = static_cast<uint64_t>( 256.0f * 65536.0f * 8363.0f / pbFrq * g_linearMult[ tmp % ( 12 * 16 * 4 ) ] * adjFac );
		res <<= 8;
		res >>= div;
		return res >> 32;
	}
}

uint16_t XmModule::periodToFineNoteIndex( uint16_t period, int8_t finetune, uint8_t deltaNote ) const
{
	int8_t tuned = ( finetune / 8 + 16 );
	int16_t ofsLo = 0;
	int16_t ofsHi = 1536;
	for( int i = 0; i < 8; i++ ) {
		int16_t ofsMid = ( ofsLo + ofsHi ) >> 1;
		ofsMid &= 0xfff0;
		ofsMid += tuned;
		if( period <= m_noteToPeriod.at( ofsMid - 8 ) ) {
			ofsLo = ofsMid - tuned;
		}
		else {
			ofsHi = ofsMid - tuned;
		}
	}
	int16_t ofs = ofsLo + tuned + deltaNote;
	if( ofs > 1550 ) {
		ofs = 1551;
	}
	return ofs;
}

uint16_t XmModule::glissando( uint16_t period, int8_t finetune, uint8_t deltaNote ) const
{
	return m_noteToPeriod.at( periodToFineNoteIndex( period, finetune, deltaNote ) );
}

void XmModule::doPatternBreak( int16_t next )
{
	if( next <= 0x3f ) {
		m_jumpRow = next;
	}
	else {
		m_jumpRow = 0;
	}
	doJumpPos( state().order );
	m_doPatJump = true;
}

void XmModule::doJumpPos( int16_t next )
{
	m_jumpOrder = next;
}

void XmModule::doPatLoop( int16_t next )
{
	m_jumpRow = next;
	m_isPatLoop = true;
}

IArchive& XmModule::serialize( IArchive* data )
{
	GenModule::serialize( data );
	for( XmChannel*& chan : m_channels ) {
		if( !chan ) {
			continue;
		}
		data->archive( chan );
	}
	*data
	% m_jumpRow
	% m_jumpOrder
	% m_isPatLoop
	% m_doPatJump
	% m_restartPos
	% m_requestedPatternDelay
	% m_currentPatternDelay;
	return *data;
}

GenModule::Ptr XmModule::factory( const std::string& filename, uint32_t frequency, int maxRpt )
{
	XmModule::Ptr result( new XmModule( maxRpt ) );
	if( !result ) {
		return GenModule::Ptr();
	}
	if( !result->load( filename ) ) {
		return GenModule::Ptr();
	}
	if( !result->initialize( frequency ) ) {
		return GenModule::Ptr();
	}
	return result;
}

bool XmModule::isRunningPatDelay() const
{
	return m_currentPatternDelay != 0;
}

void XmModule::doPatDelay( uint8_t counter )
{
	if( isRunningPatDelay() ) {
		return;
	}
	m_requestedPatternDelay = counter + 1;
}

light4cxx::Logger* XmModule::logger()
{
	return light4cxx::Logger::get( GenModule::logger()->name() + ".xm" );
}

}
}

/**
 * @}
 */
