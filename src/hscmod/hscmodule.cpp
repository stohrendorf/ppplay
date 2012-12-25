#include "hscmodule.h"
#include <genmod/abstractorder.h>
#include <genmod/genbase.h>
#include <genmod/breseninter.h>

#include <boost/algorithm/string.hpp>

namespace hsc
{
	
namespace
{
constexpr uint8_t ChanToCarrier[9] = {3,4,5,11,12,13,19,20,21};
constexpr uint16_t NoteToFnum[12] = { 8555, 8577, 8600, 8624, 8650, 8677, 8706, 8736, 8769, 8803, 8839, 8878 };
}

ppp::AbstractModule* Module::factory( Stream* stream, uint32_t frequency, int maxRpt, ppp::Sample::Interpolation inter )
{
	Module* res = new Module( maxRpt, inter );
	if( !res->load( stream ) ) {
		delete res;
		return nullptr;
	}
	res->initialize( frequency );
	return res;
}

size_t Module::internal_buildTick( AudioFrameBuffer* buf )
{
	if( !update(buf == nullptr) ) {
		logger()->info( L4CXX_LOCATION, "Update failed, song end reached" );
		if(buf) {
			buf->reset();
		}
		return 0;
	}
	if( state().order >= orderCount() ) {
		logger()->info( L4CXX_LOCATION, "Song end reached" );
		if(buf) {
			buf->reset();
		}
		return 0;
	}
	if( orderAt( state().order )->playbackCount() >= maxRepeat() ) {
		logger()->info( L4CXX_LOCATION, "Song end reached: Maximum repeat count reached" );
		if(buf) {
			buf->reset();
		}
		return 0;
	}
	const size_t BufferSize = frequency()/18.20676;
	if( buf ) {
		if( !buf->get() ) {
			buf->reset( new AudioFrameBuffer::element_type );
		}
		buf->get()->resize( BufferSize );
		ppp::BresenInterpolation interp(frequency(), opl::Opl3::SampleRate);
		for(size_t i=0; i<BufferSize; i++) {
			// TODO panning?
			std::vector<int16_t> data = m_opl.read();
			(**buf)[i].left = data[0]+data[1];
			(**buf)[i].right = data[2]+data[3];
			if( interp.next() == 2 ) {
				m_opl.read(); // skip a sample
			}
			interp = 0;
		}
		state().playedFrames += BufferSize;
	}
	return BufferSize;
}

int Module::internal_channelCount() const
{
	return 9;
}

ppp::ChannelState Module::internal_channelStatus( size_t idx ) const
{
	return m_channels[idx].state;
}

class Order : public ppp::AbstractOrder
{
	DISABLE_COPY( Order )
	Order() = delete;
public:
	inline Order( uint8_t idx ) : AbstractOrder( idx ) {
	}
	virtual bool isUnplayed() const
	{
		return playbackCount() == 0;
	}
};

bool Module::load( Stream* stream )
{
	if( !boost::ends_with( boost::to_lower_copy( stream->name() ), ".hsc" ) || stream->size() > 59187 ) {
		logger()->debug(L4CXX_LOCATION, "Invalid filename or size mismatch (size=%d)", stream->size());
		return false;
	}
	noConstMetaInfo().filename = stream->name();
	noConstMetaInfo().title = stream->name();
	noConstMetaInfo().trackerInfo = "HSC Tracker";
	setTempo(125);
	setSpeed(2);
	
	stream->seek( 0 );
	stream->read( reinterpret_cast<char*>( m_instr ), 128 * 12 );
	for( int i = 0; i < 128; i++ ) {
		m_instr[i][2] ^= ( m_instr[i][2] & 0x40 ) << 1;
		m_instr[i][3] ^= ( m_instr[i][3] & 0x40 ) << 1;
		m_instr[i][11] >>= 4;
	}
	uint8_t orders[51];
	stream->read( orders, 51 );
	for(int i=0; i<51 && orders[i]!=0xff; i++) {
		addOrder(new Order(orders[i]));
	}
	stream->read( reinterpret_cast<char*>( m_patterns ), stream->size()-stream->pos() );
	m_opl.writeReg( 1, 32 );
	m_opl.writeReg( 8, 128 );
	m_opl.writeReg( 0xbd, 0 );

	for( int i = 0; i < 9; i++ ) {
		storeInstr( i, i );
	}
	return true;
}

Module::Module( int maxRpt, ppp::Sample::Interpolation inter ) : ppp::AbstractModule( maxRpt, inter ), m_opl(),
	m_instr {{0}}, m_patterns(), m_channels(), m_speedCountdown( 1 ), m_fnum(),
	m_bd( 0 ), m_mode6( false ), m_patBreak( 0 )
{
}

Module::~Module()
{
}

AbstractArchive& Module::Channel::serialize(AbstractArchive* archive)
{
	*archive
	% instr
	% fnum
	% updateFnum
	% tlCarrier
	% updateTlCarrier
	% tlModulator
	% updateTlModulator
	% slide
	% state;
	return *archive;
}

AbstractArchive& Module::serialize( AbstractArchive* data )
{
	ppp::AbstractModule::serialize( data );
	for(int i=0; i<9; i++) {
		data->archive(m_channels+i);
	}
	data->array(m_fnum, 9)
	% m_speedCountdown
	% m_bd
	% m_mode6
	% m_patBreak
	% m_opl;
	return *data;
}

void Module::storeInstr( uint8_t chan, uint8_t instr )
{
	if( m_channels[chan].instr == instr ) {
		return;
	}
	m_channels[chan].instr = instr;
	const InsData& data = m_instr[instr];
	m_opl.writeReg(0xb0 + chan, 0);
	m_opl.writeReg(0xc0 + chan, data[8]);
	uint8_t slotC = ChanToCarrier[chan];
	uint8_t slotM = ChanToCarrier[chan]-3;
	m_opl.writeReg(0x20 + slotC, data[0]);
	m_opl.writeReg(0x20 + slotM, data[1]);
	m_opl.writeReg(0x60 + slotC, data[4]);
	m_opl.writeReg(0x60 + slotM, data[5]);
	m_opl.writeReg(0x80 + slotC, data[6]);
	m_opl.writeReg(0x80 + slotM, data[7]);
	m_opl.writeReg(0xe0 + slotC, data[9]);
	m_opl.writeReg(0xe0 + slotM, data[10]);
	m_opl.writeReg(0x40 + slotC, data[2]);
	m_opl.writeReg(0x40 + slotM, data[3]);
	m_channels[chan].tlCarrier = data[2];
	m_channels[chan].updateTlCarrier = true;
	m_channels[chan].tlModulator = data[3];
	m_channels[chan].updateTlModulator = true;
	m_channels[chan].slide = data[11]>>4;
}

void Module::setNote( uint8_t chan, uint8_t note )
{
	BOOST_ASSERT( note != 0 );
	if( note == 0x7f ) {
		m_channels[chan].state.note = ppp::ChannelState::KeyOff;
		// key off
		m_channels[chan].fnum &= ~0x2000;
		m_channels[chan].updateFnum = true;
		return;
	}
	note--;
	m_opl.writeReg(0xb0 + chan, 0);
	m_channels[chan].state.note = note;
	m_channels[chan].state.active = true;
	m_channels[chan].state.noteTriggered = true;
	m_channels[chan].fnum = NoteToFnum[ note%12 ] + ((note/12)<<10);
	m_channels[chan].updateFnum = true;
}

bool Module::update(bool estimate)
{
	m_speedCountdown--;
	if( m_speedCountdown )
		return true;
	m_speedCountdown = state().speed;
	
	//BEGIN handleEffects
	for( uint_fast8_t chan = 0; chan < 9; chan++ ) {
		uint32_t pattoff = state().row * 9 + chan;
		const uint8_t note = m_patterns[state().pattern][pattoff].note;
		const uint8_t effect = m_patterns[state().pattern][pattoff].effect;
		
		if( estimate ) {
			if( note&0x80 ) {
				continue;
			}
			if( effect == 0 ) {
				continue;
			}
			else if( effect == 1 ) {
				// next pattern
				state().row = 0x3f;
				continue;
			}
			switch( effect>>4 ) {
				case 0x01:
				case 0x02:
				case 0x0a:
				case 0x0b:
				case 0x0c:
					break;
				default:
					setSpeed( m_speedCountdown = (effect&0x0f)+1 );
			}
			continue;
		}
		
		if( m_speedCountdown == state().speed ) {
			m_channels[chan].state.noteTriggered = false;
			m_channels[chan].state.fxDesc = ppp::fxdesc::NullFx;
			if( m_channels[chan].state.note == ppp::ChannelState::KeyOff ) {
				m_channels[chan].state.note = ppp::ChannelState::NoNote;
			}
		}
		
		if(note == 0x80) {
			storeInstr(chan, effect&0x7f);
			m_channels[chan].state.cell = stringFmt("III %02X", (effect&0x7f) + 0);
			continue;
		}
		
		if(note != 0) {
			setNote(chan, note&0x7f);
			if( (note&0x7f) == 0x7f ) {
				m_channels[chan].state.cell = "Pau ";
			}
			else {
				m_channels[chan].state.cell = stringFmt("%s%d ", ppp::NoteNames[(note-1)%12], (note-1)/12+0);
			}
		}
		else {
			m_channels[chan].state.cell = "... ";
		}
		m_channels[chan].state.cell += stringFmt("%02X", effect+0);
		
		if( effect == 0 ) {
			continue;
		}
		else if( effect == 1 ) {
			m_channels[chan].state.fxDesc = ppp::fxdesc::PatternBreak;
			// next pattern
			state().row = 0x3f;
			continue;
		}
		
		switch(effect>>4) {
			case 0x01:
				{
					m_channels[chan].state.fxDesc = ppp::fxdesc::PitchSlideUp;
					// slide up
					uint8_t val = m_channels[chan].fnum;
					val += (effect&0x0f) + 1;
					m_channels[chan].fnum = (m_channels[chan].fnum & 0xff00) | val;
					m_channels[chan].updateFnum = true;
				}
				break;
			case 0x02:
				{
					m_channels[chan].state.fxDesc = ppp::fxdesc::PitchSlideDown;
					// slide down
					uint8_t val = m_channels[chan].fnum;
					val -= (effect&0x0f) + 1;
					m_channels[chan].fnum = (m_channels[chan].fnum & 0xff00) | val;
					m_channels[chan].updateFnum = true;
				}
				break;
			case 0x0a:
				m_channels[chan].state.fxDesc = ppp::fxdesc::SetVolume;
				// set carrier volume
				m_channels[chan].tlCarrier = (effect&0x0f)<<2;
				m_channels[chan].updateTlCarrier = true;
				break;
			case 0x0b:
				// set modulator volume
				m_channels[chan].tlModulator = (effect&0x0f)<<2;
				m_channels[chan].updateTlModulator = true;
				break;
			case 0x0c:
				// set volume
				{
					m_channels[chan].state.fxDesc = ppp::fxdesc::SetVolume;
					uint8_t val = (effect&0x0f)<<2;
					m_channels[chan].tlCarrier = val;
					m_channels[chan].updateTlCarrier = true;
					if( m_instr[m_channels[chan].instr][8] & 1 ) {
						m_channels[chan].tlModulator = val;
						m_channels[chan].updateTlModulator = true;
					}
				}
				break;
			default:
				m_channels[chan].state.fxDesc = ppp::fxdesc::SetSpeed;
				// set speed
				setSpeed( m_speedCountdown = (effect&0x0f)+1 );
		}
	}
	//END

	state().row++;
	if( 0x40 == state().row ) {
		state().row = 0;
		uint8_t order = state().order+1;
		uint8_t pat = state().pattern;
		if( pat & 0x80 ) {
			if( pat == 0xff ) {
				pat = 0x80;
			}
			pat &= 0x7f;
			if( pat >= 49 ) {
				pat = 0;
			}
			order = pat;
		}
		setOrder(order, estimate);
		if( order >= orderCount() ) {
			return false;
		}
	}
	
	if( !estimate ) {
		//BEGIN updateChanData
		for(int i=0; i<9; i++) {
			if(m_channels[i].slide!=0 || m_channels[i].updateFnum) {
				m_channels[i].updateFnum = false;
				m_opl.writeReg(0xa0 + i, m_channels[i].fnum & 0xff);
				m_opl.writeReg(0xb0 + i, m_channels[i].fnum >> 8);
			}
			const uint8_t carrierSlot = ChanToCarrier[i];
			const uint8_t modSlot = carrierSlot-3;
			if( m_channels[i].updateTlCarrier ) {
				m_channels[i].updateTlCarrier = false;
				m_opl.writeReg(0x40 + carrierSlot, m_channels[i].tlCarrier);
				m_channels[i].state.volume = 100 - (m_channels[i].tlCarrier&0x3f) * 100 / 0x3f;
			}
			if( m_channels[i].updateTlModulator ) {
				m_channels[i].updateTlModulator = false;
				m_opl.writeReg(0x40 + modSlot, m_channels[i].tlModulator);
			}
		}
		//END
	}
	return true;
}

void Module::setFreq( uint8_t chan, uint16_t frq )
{
	m_fnum[chan] = ( m_fnum[chan] & ~3 ) | ( frq >> 8 );

	m_opl.writeReg( 0xa0 + chan, frq & 0xff );
	m_opl.writeReg( 0xb0 + chan, m_fnum[chan] );
}

light4cxx::Logger* Module::logger()
{
	return light4cxx::Logger::get( AbstractModule::logger()->name() + ".hsc" );
}

}
