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
#include "topcymbaloperator.h"
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
	Operator::Ptr m_operators[2][0x20];
	// The YMF262 has 18 2-op channels.
	// Each 2-op channel can be at a serial or parallel operator configuration:
	Channel2Op::Ptr m_channels2op[2][9];
	Channel4Op::Ptr m_channels4op[2][3];
	AbstractChannel::Ptr m_channels[2][9];
	DisabledChannel::Ptr m_disabledChannel;

	BassDrumChannel::Ptr m_bassDrumChannel;
	HighHatSnareDrumChannel::Ptr m_highHatSnareDrumChannel;
	TomTomTopCymbalChannel::Ptr m_tomTomTopCymbalChannel;
	HighHatOperator::Ptr m_highHatOperator;
	SnareDrumOperator::Ptr m_snareDrumOperator;
	TomTomOperator::Ptr m_tomTomOperator;
	TopCymbalOperator::Ptr m_topCymbalOperator;
	Operator::Ptr m_highHatOperatorInNonRhythmMode;
	Operator::Ptr m_snareDrumOperatorInNonRhythmMode;
	Operator::Ptr m_tomTomOperatorInNonRhythmMode;
	Operator::Ptr m_topCymbalOperatorInNonRhythmMode;

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
	//! @brief 13 bits
	uint16_t m_vibratoIndex;
	//! @brief 14 bits, wraps around after 13*1024
	uint16_t m_tremoloIndex;

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
		write(index>>8, index&0xff, val);
	}
	bool nts() const {
		return m_nts;
	}
	Operator* topCymbalOperator() const {
		return m_topCymbalOperator.get();
	}
	Operator* highHatOperator() const {
		return m_highHatOperator.get();
	}
	Operator* snareDrumOperator() const {
		return m_snareDrumOperator.get();
	}
	Operator* tomTomOperator() const {
		return m_tomTomOperator.get();
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
	uint16_t tremoloIndex() const {
		return m_tremoloIndex;
	}

	std::vector<short> read() ;

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
	void write( int array, int address, int data ) ;
};
}

#endif
