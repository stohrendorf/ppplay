#ifndef PPP_OPL_OPL3_H
#define PPP_OPL_OPL3_H

#include <cstdint>
#include <array>

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

	static constexpr int MasterClock = 14.32e6;
	static constexpr double sampleRate = MasterClock/288.0;

	// The OPL3 tremolo repetition rate is 3.7 Hz.
	static constexpr int tremoloTableLength = 13*1024;
	static constexpr double calculateIncrement( double begin, double end, double period ) {
		return ( end - begin ) / period;
	}

private:
	uint8_t m_registers[0x200];

	// The YMF262 has 36 operators:
	Operator* m_operators[2][0x20];
	// The YMF262 has 18 2-op channels.
	// Each 2-op channel can be at a serial or parallel operator configuration:
	Channel2Op* m_channels2op[2][9];
	Channel4Op* m_channels4op[2][3];
	AbstractChannel* m_channels[2][9];
	DisabledChannel* m_disabledChannel;

	BassDrumChannel* m_bassDrumChannel;
	HighHatSnareDrumChannel* m_highHatSnareDrumChannel;
	TomTomTopCymbalChannel* m_tomTomTopCymbalChannel;
	HighHatOperator* m_highHatOperator;
	SnareDrumOperator* m_snareDrumOperator;
	TomTomOperator* m_tomTomOperator;
	TopCymbalOperator* m_topCymbalOperator;
	Operator* m_highHatOperatorInNonRhythmMode;
	Operator* m_snareDrumOperatorInNonRhythmMode;
	Operator* m_tomTomOperatorInNonRhythmMode;
	Operator* m_topCymbalOperatorInNonRhythmMode;

	bool m_nts;
	//! @brief Depth of amplitude. This register is used in EnvelopeGenerator.getEnvelope();
	bool m_dam;
	//! @brief Depth of vibrato. This register is used in PhaseGenerator.getPhase();
	bool m_dvb;
	bool m_ryt;
	bool m_bd, m_sd, m_tom, m_tc, m_hh;
	//! @brief OPL2/OPL3 mode selection. This register is used in OPL3.read(), OPL3.write() and Operator.getOperatorOutput();
	bool m_new;
	//! @brief 2-op/4-op channel selection. This register is used here to configure the OPL3.channels[] array.
	uint8_t m_connectionsel;
	int m_vibratoIndex, m_tremoloIndex;

	// The methods read() and write() are the only
	// ones needed by the user to interface with the emulator.
	// read() returns one frame at a time, to be played at 49700 Hz,
	// with each frame being four 16-bit samples,
	// corresponding to the OPL3 four output channels CHA...CHD.
public:
	bool isNew() const {
		return m_new;
	}
	uint8_t readReg( int index ) const {
		return m_registers[index];
	}
	void writeReg( int index, uint8_t val ) {
		m_registers[index] = val;
	}
	bool nts() const {
		return m_nts;
	}
	TopCymbalOperator* topCymbalOperator() const {
		return m_topCymbalOperator;
	}
	HighHatOperator* highHatOperator() const {
		return m_highHatOperator;
	}
	SnareDrumOperator* snareDrumOperator() const {
		return m_snareDrumOperator;
	}
	TomTomOperator* tomTomOperator() const {
		return m_tomTomOperator;
	}
	bool dvb() const {
		return m_dvb;
	}
	int vibratoIndex() const {
		return m_vibratoIndex;
	}
	bool dam() const {
		return m_dam;
	}
	int tremoloIndex() const {
		return m_tremoloIndex;
	}

	std::vector<short> read() ;

	void write( int array, int address, int data ) ;

	Opl3() ;

private:
	void initOperators() ;

	void initChannels2op() ;

	void initChannels4op() ;

	void initRhythmChannels() ;

	void initChannels() ;

	void update_1_NTS1_6() ;

	void update_DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1() ;

	void update_7_NEW1() ;

public:
	void setEnabledChannels() ;

	void update_2_CONNECTIONSEL6() ;

	void set4opConnections() ;

	void setRhythmMode() ;
};
}

#endif
