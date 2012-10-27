#ifndef PPP_OPL_OPL3_H
#define PPP_OPL_OPL3_H

#include "operator.h"
#include "highhatoperator.h"
#include "snaredrumoperator.h"
#include "tomtomoperator.h"
#include "channel2op.h"
#include "channel4op.h"
#include "bassdrumchannel.h"
#include "highhatsnaredrumchannel.h"
#include "tomtomtopcymbalchannel.h"
#include "disabledchannel.h"

namespace opl
{
class Opl3
{
public:
	// OPL3-wide registers offsets:
	static constexpr int
	_1_NTS1_6_Offset = 0x08,
	DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1_Offset = 0xBD,
	_7_NEW1_Offset = 0x105,
	_2_CONNECTIONSEL6_Offset = 0x104;

	static constexpr double sampleRate = 49700;

	static int loadTables() {
		loadVibratoTable();
		loadTremoloTable();
		return 1;
	}
	static const int loadTablesVar = loadTables();

	static double vibratoTable[2][8192];
	static void loadVibratoTable() {

		// According to the YMF262 datasheet, the OPL3 vibrato repetition rate is 6.1 Hz.
		// According to the YMF278B manual, it is 6.0 Hz.
		// The information that the vibrato table has 8 levels standing 1024 samples each
		// was taken from the emulator by Jarek Burczynski and Tatsuyuki Satoh,
		// with a frequency of 6,06689453125 Hz, what  makes sense with the difference
		// in the information on the datasheets.

		// The first array is used when DVB=0 and the second array is used when DVB=1.
		vibratoTable = new double[2][8192];

		static constexpr double semitone = std::pow( 2, 1 / 12d );
		// A cent is 1/100 of a semitone:
		static constexpr double cent = std::pow( semitone, 1 / 100d );

		// When dvb=0, the depth is 7 cents, when it is 1, the depth is 14 cents.
		static constexpr double DVB0 = std::pow( cent, 7 );
		static constexpr double DVB1 = std::pow( cent, 14 );
		int i;
		for( i = 0; i < 1024; i++ )
			vibratoTable[0][i] = vibratoTable[1][i] = 1;
		for( ; i < 2048; i++ ) {
			vibratoTable[0][i] = std::sqrt( DVB0 );
			vibratoTable[1][i] = std::sqrt( DVB1 );
		}
		for( ; i < 3072; i++ ) {
			vibratoTable[0][i] = DVB0;
			vibratoTable[1][i] = DVB1;
		}
		for( ; i < 4096; i++ ) {
			vibratoTable[0][i] = std::sqrt( DVB0 );
			vibratoTable[1][i] = std::sqrt( DVB1 );
		}
		for( ; i < 5120; i++ )
			vibratoTable[0][i] = vibratoTable[1][i] = 1;
		for( ; i < 6144; i++ ) {
			vibratoTable[0][i] = 1 / std::sqrt( DVB0 );
			vibratoTable[1][i] = 1 / std::sqrt( DVB1 );
		}
		for( ; i < 7168; i++ ) {
			vibratoTable[0][i] = 1 / DVB0;
			vibratoTable[1][i] = 1 / DVB1;
		}
		for( ; i < 8192; i++ ) {
			vibratoTable[0][i] = 1 / std::sqrt( DVB0 );
			vibratoTable[1][i] = 1 / std::sqrt( DVB1 );
		}

	}

	// The OPL3 tremolo repetition rate is 3.7 Hz.
	static constexpr double tremoloFrequency = 3.7;
	static constexpr int tremoloTableLength = sampleRate / tremoloFrequency;
	// First array used when AM = 0 and second array used when AM = 1.
	static double tremoloTable[2][tremoloTableLength];
	static void loadTremoloTable() {

		// The tremolo depth is -1 dB when DAM = 0, and -4.8 dB when DAM = 1.
		static constexpr double tremoloDepth[] = { -1, -4.8};

		//  According to the YMF278B manual's OPL3 section graph,
		//              the tremolo waveform is not
		//   \      /   a sine wave, but a single triangle waveform.
		//    \    /    Thus, the period to achieve the tremolo depth is T/2, and
		//     \  /     the increment in each T/2 section uses a frequency of 2*f.
		//      \/      Tremolo varies from 0 dB to depth, to 0 dB again, at frequency*2:
		static constexpr double tremoloIncrement[] = {
			calculateIncrement( tremoloDepth[0], 0, 1 / ( 2 * tremoloFrequency ) ),
			calculateIncrement( tremoloDepth[1], 0, 1 / ( 2 * tremoloFrequency ) )
		};

		// This is undocumented. The tremolo starts at the maximum attenuation,
		// instead of at 0 dB:
		tremoloTable[0][0] = tremoloDepth[0];
		tremoloTable[1][0] = tremoloDepth[1];
		int counter = 0;
		// The first half of the triangle waveform:
		while( tremoloTable[0][counter] < 0 ) {
			counter++;
			tremoloTable[0][counter] = tremoloTable[0][counter - 1] + tremoloIncrement[0];
			tremoloTable[1][counter] = tremoloTable[1][counter - 1] + tremoloIncrement[1];
		}
		// The second half of the triangle waveform:
		while( tremoloTable[0][counter] > tremoloDepth[0] && counter < tremoloTableLength - 1 ) {
			counter++;
			tremoloTable[0][counter] = tremoloTable[0][counter - 1] - tremoloIncrement[0];
			tremoloTable[1][counter] = tremoloTable[1][counter - 1] - tremoloIncrement[1];
		}

	}

	static constexpr double calculateIncrement( double begin, double end, double period ) {
		return ( end - begin ) / sampleRate * ( 1 / period );
	}

private:
	int m_registers[0x200];

	// The YMF262 has 36 operators:
	Operator* operators[2][0x20];
	// The YMF262 has 18 2-op channels.
	// Each 2-op channel can be at a serial or parallel operator configuration:
	Channel2Op* channels2op[2][9];
	Channel4Op* channels4op[2][3];
	Channel* channels[2][9];
	DisabledChannel disabledChannel;

	BassDrumChannel* bassDrumChannel;
	HighHatSnareDrumChannel* highHatSnareDrumChannel;
	TomTomTopCymbalChannel* tomTomTopCymbalChannel;
	HighHatOperator* highHatOperator;
	SnareDrumOperator* snareDrumOperator;
	TomTomOperator* tomTomOperator;
	TopCymbalOperator* topCymbalOperator;
	Operator* highHatOperatorInNonRhythmMode;
	Operator* snareDrumOperatorInNonRhythmMode;
	Operator* tomTomOperatorInNonRhythmMode;
	Operator* topCymbalOperatorInNonRhythmMode;

	int nts, dam, dvb, ryt, bd, sd, tom, tc, hh, _new, connectionsel;
	int vibratoIndex, tremoloIndex;

	// The methods read() and write() are the only
	// ones needed by the user to interface with the emulator.
	// read() returns one frame at a time, to be played at 49700 Hz,
	// with each frame being four 16-bit samples,
	// corresponding to the OPL3 four output channels CHA...CHD.
public:
	std::vector<short> read() {
		std::vector<short> output( 4 );
		std::vector<double> outputBuffer( 4 );
		std::vector<double> channelOutput;

		for( int outputChannelNumber = 0; outputChannelNumber < 4; outputChannelNumber++ )
			outputBuffer[outputChannelNumber] = 0;

		// If _new = 0, use OPL2 mode with 9 channels. If _new = 1, use OPL3 18 channels;
		for( int array = 0; array < ( _new + 1 ); array++ )
			for( int channelNumber = 0; channelNumber < 9; channelNumber++ ) {
				// Reads output from each OPL3 channel, and accumulates it in the output buffer:
				channelOutput = channels[array][channelNumber].getChannelOutput();
				for( int outputChannelNumber = 0; outputChannelNumber < 4; outputChannelNumber++ )
					outputBuffer[outputChannelNumber] += channelOutput[outputChannelNumber];
			}

		// Normalizes the output buffer after all channels have been added,
		// with a maximum of 18 channels,
		// and multiplies it to get the 16 bit signed output.
		for( int outputChannelNumber = 0; outputChannelNumber < 4; outputChannelNumber++ )
			output[outputChannelNumber] =
				( short )( outputBuffer[outputChannelNumber] / 18 * 0x7FFF );

		// Advances the OPL3-wide vibrato index, which is used by
		// PhaseGenerator.getPhase() in each Operator.
		vibratoIndex++;
		if( vibratoIndex >= OPL3Data.vibratoTable[dvb].length ) vibratoIndex = 0;
		// Advances the OPL3-wide tremolo index, which is used by
		// EnvelopeGenerator.getEnvelope() in each Operator.
		tremoloIndex++;
		if( tremoloIndex >= OPL3Data.tremoloTable[dam].length ) tremoloIndex = 0;

		return output;
	}

	void write( int array, int address, int data ) {
		// The OPL3 has two registers arrays, each with adresses ranging
		// from 0x00 to 0xF5.
		// This emulator uses one array, with the two original register arrays
		// starting at 0x00 and at 0x100.
		int registerAddress = ( array << 8 ) | address;
		// If the address is out of the OPL3 memory map, returns.
		if( registerAddress < 0 || registerAddress >= 0x200 ) return;

		registers[registerAddress] = data;
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
				// Unique registers for the entire OPL3:
				if( array == 1 ) {
					if( address == 0x04 )
						update_2_CONNECTIONSEL6();
					else if( address == 0x05 )
						update_7_NEW1();
				}
				else if( address == 0x08 ) update_1_NTS1_6();
				break;

			case 0xA0:
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
					channels[array][address & 0x0F].update_2_KON1_BLOCK3_FNUMH2();
					break;
				}
				// 0xA0...0xA8 keeps fnum(l) for each channel.
				if( ( address & 0xF0 ) == 0xA0 && address <= 0xA8 )
					channels[array][address & 0x0F].update_FNUML8();
				break;
				// 0xC0...0xC8 keeps cha,chb,chc,chd,fb,cnt for each channel:
			case 0xC0:
				if( address <= 0xC8 )
					channels[array][address & 0x0F].update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();
				break;

				// Registers for each of the 36 Operators:
			default:
				int operatorOffset = address & 0x1F;
				if( operators[array][operatorOffset] == null ) break;
				switch( address & 0xE0 ) {
						// 0x20...0x35 keeps am,vib,egt,ksr,mult for each operator:
					case 0x20:
						operators[array][operatorOffset].update_AM1_VIB1_EGT1_KSR1_MULT4();
						break;
						// 0x40...0x55 keeps ksl,tl for each operator:
					case 0x40:
						operators[array][operatorOffset].update_KSL2_TL6();
						break;
						// 0x60...0x75 keeps ar,dr for each operator:
					case 0x60:
						operators[array][operatorOffset].update_AR4_DR4();
						break;
						// 0x80...0x95 keeps sl,rr for each operator:
					case 0x80:
						operators[array][operatorOffset].update_SL4_RR4();
						break;
						// 0xE0...0xF5 keeps ws for each operator:
					case 0xE0:
						operators[array][operatorOffset].update_5_WS3();
				}
		}
	}

	Opl3()
		: m_registers(), nts( 0 ), dam( 0 ), dvb( 0 ), ryt( 0 ), bd( 0 ), sd( 0 ), tom( 0 ), tc( 0 ), hh( 0 ), _new( 0 ), connectionsel( 0 ),
		  vibratoIndex( 0 ), tremoloIndex( 0 ), channels() {
		nts = dam = dvb = ryt = bd = sd = tom = tc = hh = _new = connectionsel = 0;
		vibratoIndex = tremoloIndex = 0;

		initOperators();
		initChannels2op();
		initChannels4op();
		initRhythmChannels();
		initChannels();
	}

private:
	void initOperators() {
		int baseAddress;
		for( int array = 0; array < 2; array++ )
			for( int group = 0; group <= 0x10; group += 8 )
				for( int offset = 0; offset < 6; offset++ ) {
					baseAddress = ( array << 8 ) | ( group + offset );
					operators[array][group + offset] = new Operator( baseAddress );
				}

		// Create specific operators to switch when in rhythm mode:
		highHatOperator = new HighHatOperator();
		snareDrumOperator = new SnareDrumOperator();
		tomTomOperator = new TomTomOperator();
		topCymbalOperator = new TopCymbalOperator();

		// Save operators when they are in non-rhythm mode:
		// Channel 7:
		highHatOperatorInNonRhythmMode = operators[0][0x11];
		snareDrumOperatorInNonRhythmMode = operators[0][0x14];
		// Channel 8:
		tomTomOperatorInNonRhythmMode = operators[0][0x12];
		topCymbalOperatorInNonRhythmMode = operators[0][0x15];

	}

	void initChannels2op() {
		for( int array = 0; array < 2; array++ )
			for( int channelNumber = 0; channelNumber < 3; channelNumber++ ) {
				int baseAddress = ( array << 8 ) | channelNumber;
				// Channels 1, 2, 3 -> Operator offsets 0x0,0x3; 0x1,0x4; 0x2,0x5
				channels2op[array][channelNumber]   = new Channel2Op( baseAddress, operators[array][channelNumber], operators[array][channelNumber + 0x3] );
				// Channels 4, 5, 6 -> Operator offsets 0x8,0xB; 0x9,0xC; 0xA,0xD
				channels2op[array][channelNumber + 3] = new Channel2Op( baseAddress + 3, operators[array][channelNumber + 0x8], operators[array][channelNumber + 0xB] );
				// Channels 7, 8, 9 -> Operators 0x10,0x13; 0x11,0x14; 0x12,0x15
				channels2op[array][channelNumber + 6] = new Channel2Op( baseAddress + 6, operators[array][channelNumber + 0x10], operators[array][channelNumber + 0x13] );
			}
	}

	void initChannels4op() {
		for( int array = 0; array < 2; array++ )
			for( int channelNumber = 0; channelNumber < 3; channelNumber++ ) {
				int baseAddress = ( array << 8 ) | channelNumber;
				// Channels 1, 2, 3 -> Operators 0x0,0x3,0x8,0xB; 0x1,0x4,0x9,0xC; 0x2,0x5,0xA,0xD;
				channels4op[array][channelNumber]   = new Channel4Op( baseAddress, operators[array][channelNumber], operators[array][channelNumber + 0x3], operators[array][channelNumber + 0x8], operators[array][channelNumber + 0xB] );
			}
	}

	void initRhythmChannels() {
		bassDrumChannel = new BassDrumChannel();
		highHatSnareDrumChannel = new HighHatSnareDrumChannel();
		tomTomTopCymbalChannel = new TomTomTopCymbalChannel();
	}

	void initChannels() {
		// Channel is an abstract class that can be a 2-op, 4-op, rhythm or disabled channel,
		// depending on the OPL3 configuration at the time.
		// channels[] inits as a 2-op serial channel array:
		for( int array = 0; array < 2; array++ )
			for( int i = 0; i < 9; i++ ) channels[array][i] = channels2op[array][i];

		// Unique instance to fill future gaps in the Channel array,
		// when there will be switches between 2op and 4op mode.
		disabledChannel = new DisabledChannel();
	}

	void update_1_NTS1_6() {
		int _1_nts1_6 = m_registers[OPL3Data._1_NTS1_6_Offset];
		// Note Selection. This register is used in Channel.updateOperators() implementations,
		// to calculate the channel´s Key Scale Number.
		// The value of the actual envelope rate follows the value of
		// OPL3.nts,Operator.keyScaleNumber and Operator.ksr
		nts = ( _1_nts1_6 & 0x40 ) >> 6;
	}

	void update_DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1() {
		int dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 = m_registers[DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1_Offset];
		// Depth of amplitude. This register is used in EnvelopeGenerator.getEnvelope();
		dam = ( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x80 ) >> 7;

		// Depth of vibrato. This register is used in PhaseGenerator.getPhase();
		dvb = ( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x40 ) >> 6;

		int new_ryt = ( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x20 ) >> 5;
		if( new_ryt != ryt ) {
			ryt = new_ryt;
			setRhythmMode();
		}

		int new_bd  = ( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x10 ) >> 4;
		if( new_bd != bd ) {
			bd = new_bd;
			if( bd == 1 ) {
				bassDrumChannel.op1.keyOn();
				bassDrumChannel.op2.keyOn();
			}
		}

		int new_sd  = ( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x08 ) >> 3;
		if( new_sd != sd ) {
			sd = new_sd;
			if( sd == 1 ) snareDrumOperator.keyOn();
		}

		int new_tom = ( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x04 ) >> 2;
		if( new_tom != tom ) {
			tom = new_tom;
			if( tom == 1 ) tomTomOperator.keyOn();
		}

		int new_tc  = ( dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x02 ) >> 1;
		if( new_tc != tc ) {
			tc = new_tc;
			if( tc == 1 ) topCymbalOperator.keyOn();
		}

		int new_hh  = dam1_dvb1_ryt1_bd1_sd1_tom1_tc1_hh1 & 0x01;
		if( new_hh != hh ) {
			hh = new_hh;
			if( hh == 1 ) highHatOperator.keyOn();
		}

	}

private:
	void update_7_NEW1() {
		int _7_new1 = m_registers[OPL3Data._7_NEW1_Offset];
		// OPL2/OPL3 mode selection. This register is used in
		// OPL3.read(), OPL3.write() and Operator.getOperatorOutput();
		_new = ( _7_new1 & 0x01 );
		if( _new == 1 ) setEnabledChannels();
		set4opConnections();
	}

public:
	void setEnabledChannels() {
		for( int array = 0; array < 2; array++ )
			for( int i = 0; i < 9; i++ ) {
				int baseAddress = channels[array][i].channelBaseAddress;
				registers[baseAddress + ChannelData.CHD1_CHC1_CHB1_CHA1_FB3_CNT1_Offset] |= 0xF0;
				channels[array][i].update_CHD1_CHC1_CHB1_CHA1_FB3_CNT1();
			}
	}

	void update_2_CONNECTIONSEL6() {
		// This method is called only if _new is set.
		int _2_connectionsel6 = m_registers[OPL3Data._2_CONNECTIONSEL6_Offset];
		// 2-op/4-op channel selection. This register is used here to configure the OPL3.channels[] array.
		connectionsel = ( _2_connectionsel6 & 0x3F );
		set4opConnections();
	}

	void set4opConnections() {

		// bits 0, 1, 2 sets respectively 2-op channels (1,4), (2,5), (3,6) to 4-op operation.
		// bits 3, 4, 5 sets respectively 2-op channels (10,13), (11,14), (12,15) to 4-op operation.
		for( int array = 0; array < 2; array++ )
			for( int i = 0; i < 3; i++ ) {
				if( _new == 1 ) {
					int shift = array * 3 + i;
					int connectionBit = ( connectionsel >> shift ) & 0x01;
					if( connectionBit == 1 ) {
						channels[array][i] = channels4op[array][i];
						channels[array][i + 3] = disabledChannel;
						channels[array][i].updateChannel();
						continue;
					}
				}
				channels[array][i] = channels2op[array][i];
				channels[array][i + 3] = channels2op[array][i + 3];
				channels[array][i].updateChannel();
				channels[array][i + 3].updateChannel();
			}
	}

	void setRhythmMode() {
		if( ryt == 1 ) {
			channels[0][6] = bassDrumChannel;
			channels[0][7] = highHatSnareDrumChannel;
			channels[0][8] = tomTomTopCymbalChannel;
			operators[0][0x11] = highHatOperator;
			operators[0][0x14] = snareDrumOperator;
			operators[0][0x12] = tomTomOperator;
			operators[0][0x15] = topCymbalOperator;
		}
		else {
			for( int i = 6; i <= 8; i++ ) channels[0][i] = channels2op[0][i];
			operators[0][0x11] = highHatOperatorInNonRhythmMode;
			operators[0][0x14] = snareDrumOperatorInNonRhythmMode;
			operators[0][0x12] = tomTomOperatorInNonRhythmMode;
			operators[0][0x15] = topCymbalOperatorInNonRhythmMode;
		}
		for( int i = 6; i <= 8; i++ ) channels[0][i].updateChannel();
	}
};
}

#endif
