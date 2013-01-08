#ifndef HSCMODULE_H
#define HSCMODULE_H

#include "genmod/abstractmodule.h"
#include <genmod/channelstate.h>
#include "ymf262/opl3.h"

namespace hsc
{
class Module : public ppp::AbstractModule
{
	DISABLE_COPY( Module )
private:
	opl::Opl3 m_opl;
	typedef uint8_t InsData[12];
	InsData m_instr[128];
	struct Note {
		constexpr Note() : note( 0 ), effect( 0 ) {}
		uint8_t note;
		uint8_t effect;
	};
	Note m_patterns[50][64 * 9];
	
	struct Channel : public ISerializable
	{
		Channel() : instr( 0xff ), fnum(0), updateFnum(true),
		tlCarrier(0x3f), updateTlCarrier(true), tlModulator(0x3f), updateTlModulator(true), slide(0), state()
		{
		}
		//! @brief Currently used instrument
		uint8_t instr;
		uint16_t fnum;
		bool updateFnum;
		//! @brief Total level of the carrier
		uint8_t tlCarrier;
		bool updateTlCarrier;
		//! @brief Total level of the modulator
		uint8_t tlModulator;
		bool updateTlModulator;
		//! @todo Find the use
		uint8_t slide;
		ppp::ChannelState state;
		
        virtual AbstractArchive& serialize(AbstractArchive* archive);
	};
	
	Channel m_channels[9];
	uint8_t m_speedCountdown;
	uint8_t m_fnum[9];
	uint8_t m_bd;
	bool m_mode6;
	uint8_t m_patBreak;
public:
	static AbstractModule* factory( Stream* stream, uint32_t frequency, int maxRpt, ppp::Sample::Interpolation inter );
protected:
	virtual AbstractArchive& serialize( AbstractArchive* data );
public:
	Module( int maxRpt, ppp::Sample::Interpolation inter );
	virtual ~Module();
	bool load( Stream* stream );
private:
	virtual size_t internal_buildTick( AudioFrameBuffer* buf );
	virtual ppp::ChannelState internal_channelStatus( size_t idx ) const;
	virtual int internal_channelCount() const;

	void storeInstr( uint8_t chan, uint8_t instr );
	bool update( bool estimate );
	void setFreq( uint8_t chan, uint16_t frq );
	void setNote( uint8_t chan, uint8_t note);
	
	static light4cxx::Logger* logger();
};
}

#endif
