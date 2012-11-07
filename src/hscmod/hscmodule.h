#ifndef HSCMODULE_H
#define HSCMODULE_H

#include "genmod/abstractmodule.h"
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
	struct Channel {
		constexpr Channel() : instr( 0 ), slide( 0 ), frq( 0 ) {}
		uint8_t instr;
		int8_t slide;
		uint16_t frq;
	};
	Channel m_channels[9];
	uint8_t m_speed;
	uint8_t m_speedCountdown;
	uint8_t m_fnum[9];
	uint8_t m_row;
	uint8_t m_bd;
	bool m_mode6;
	uint8_t m_patBreak;
	uint8_t m_fadeIn;
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
	virtual std::string internal_channelStatus( size_t idx ) const;
	virtual std::string internal_channelCellString( size_t idx ) const;
	virtual int internal_channelCount() const;

	void storeInstr( uint8_t chan, uint8_t instr );
	void setVolume( uint8_t chan, uint8_t volCarrier, uint8_t volModulator );
	bool update( bool estimate );
	void setFreq( uint8_t chan, uint16_t frq );
	
	static light4cxx::Logger* logger();
};
}

#endif
