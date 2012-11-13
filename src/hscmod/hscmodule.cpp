#include "hscmodule.h"
#include <genmod/abstractorder.h>

#include <boost/algorithm/string.hpp>

namespace hsc
{

namespace
{
constexpr uint16_t note_table[12] = {363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647, 686};

constexpr uint8_t op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};
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
	const size_t BufferSize = frequency()/18.2;
	if( buf ) {
		if( !buf->get() ) {
			buf->reset( new AudioFrameBuffer::element_type );
		}
		buf->get()->resize( BufferSize );
		for(size_t i=0; i<BufferSize; i++) {
			// TODO panning?
			std::vector<int16_t> data = m_opl.read();
			(**buf)[i].left = data[0]+data[1];
			(**buf)[i].right = data[2]+data[3];
		}
		state().playedFrames += BufferSize;
	}
	return BufferSize;
}

std::string Module::internal_channelCellString( size_t /*idx*/ ) const
{
	return "---"; // FIXME
}

int Module::internal_channelCount() const
{
	return 9;
}

std::string Module::internal_channelStatus( size_t /*idx*/ ) const
{
	return "blubber"; // FIXME
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
	metaInfo().filename = stream->name();
	metaInfo().title = stream->name();
	metaInfo().trackerInfo = "HSC Tracker";
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
	m_instr {{0}}, m_patterns(), m_channels(), m_speed( 2 ), m_speedCountdown( 1 ), m_fnum(),
	m_bd( 0 ), m_mode6( false ), m_patBreak( 0 ), m_fadeIn( 0 )
{
}

Module::~Module()
{
}

AbstractArchive& Module::serialize( AbstractArchive* data )
{
	return ppp::AbstractModule::serialize( data )
	.array(m_channels, 9)
	.array(m_fnum, 9)
	% m_speed
	% m_speedCountdown
	% m_bd
	% m_mode6
	% m_patBreak
	% m_fadeIn
	% m_opl;
}

void Module::storeInstr( uint8_t chan, uint8_t instr )
{
	if( m_channels[chan].instr == instr ) {
		return;
	}
	const uint8_t op = op_table[chan];
	
	const uint8_t* insData = m_instr[instr];
	m_channels[chan].instr = instr;
	m_opl.writeReg( 0xb0 + chan, 0 ); // stop old note
	// set instrument
	m_opl.writeReg( 0xc0 + chan, insData[8] );
	m_opl.writeReg( 0x23 + op, insData[0] );      // carrier
	m_opl.writeReg( 0x20 + op, insData[1] );      // modulator
	m_opl.writeReg( 0x63 + op, insData[4] );      // bits 0..3 = decay; 4..7 = attack
	m_opl.writeReg( 0x60 + op, insData[5] );
	m_opl.writeReg( 0x83 + op, insData[6] );      // 0..3 = release; 4..7 = sustain
	m_opl.writeReg( 0x80 + op, insData[7] );
	m_opl.writeReg( 0xe3 + op, insData[9] );      // bits 0..1 = waveform
	m_opl.writeReg( 0xe0 + op, insData[10] );
	
	setVolume( chan, insData[2] & 63, insData[3] & 63 );
}

void Module::setVolume( uint8_t chan, uint8_t volCarrier, uint8_t volModulator )
{
	const uint8_t* insData = m_instr[m_channels[chan].instr];
	const uint8_t op = op_table[chan];

	m_opl.writeReg( 0x43 + op, volCarrier | ( insData[2] & ~63 ) );
	if( insData[8] & 1 ) // carrier
		m_opl.writeReg( 0x40 + op, volModulator | ( insData[3] & ~63 ) );
	else
		m_opl.writeReg( 0x40 + op, insData[3] ); // modulator
}

bool Module::update(bool estimate)
{
	m_speedCountdown--;                      // player m_speed handling
	if( m_speedCountdown )
		return true;		// nothing done

	if( m_fadeIn )					// fade-in handling
		m_fadeIn--;

	uint8_t pattnr = state().pattern;
	if( pattnr == 0xff ) {			// arrangement handling
		return false;				// set end-flag
	}
	else if( ( pattnr & 128 ) && ( pattnr <= 0xb1 ) ) { // goto pattern "nr"
		setOrder( pattnr & 127, estimate );
		state().row = 0;
		pattnr = state().pattern;
		return false;
	}

	uint32_t pattoff = state().row * 9;
	for( uint_fast8_t chan = 0; chan < 9; chan++ ) {			// handle all channels
		uint8_t note = m_patterns[pattnr][pattoff].note;
		uint8_t effect = m_patterns[pattnr][pattoff].effect;
		logger()->debug(L4CXX_LOCATION, "chan=%d note=%d effect=%d", int(chan), int(note), int(effect));
		pattoff++;

		if( note & 0x80 ) {                  // set instrument
			storeInstr( chan, effect );
			continue;
		}
		uint8_t eff_op = effect & 0x0f;
		uint8_t inst = m_channels[chan].instr;
		if( note != 0 ) {
			m_channels[chan].slide = 0;
		}

		switch( effect & 0xf0 ) {			// effect handling
			case 0:								// global effect
				/* The following fx are unimplemented on purpose:
				 * 02 - Slide Mainvolume up
				 * 03 - Slide Mainvolume down (here: fade in)
				 * 04 - Set Mainvolume to 0
				 *
				 * This is because i've never seen any HSC modules using the fx this way.
				 * All modules use the fx the way, i've implemented it.
				 */
				switch( eff_op ) {
					case 1:
						m_patBreak++;
						break;	// jump to next pattern
					case 3:
						m_fadeIn = 31;
						break;	// fade in (divided by 2)
					case 5:
						m_mode6 = true;
						break;	// 6 voice mode on
					case 6:
						m_mode6 = false;
						break;	// 6 voice mode off
				}
				break;
			case 0x20:
			case 0x10:		                    // manual slides
				if( effect & 0x10 ) {
					m_channels[chan].frq += eff_op;
					m_channels[chan].slide += eff_op;
				}
				else {
					m_channels[chan].frq -= eff_op;
					m_channels[chan].slide -= eff_op;
				}
				if( !note )
					setFreq( chan, m_channels[chan].frq );
				break;
			case 0x50:							// set percussion instrument (unimplemented)
				break;
			case 0x60:							// set feedback
				m_opl.writeReg( 0xc0 + chan, ( m_instr[m_channels[chan].instr][8] & 1 ) + ( eff_op << 1 ) );
				break;
			case 0xa0: {	                    // set carrier volume
				uint8_t vol = eff_op << 2;
				m_opl.writeReg( 0x43 + op_table[chan], vol | ( m_instr[m_channels[chan].instr][2] & ~63 ) );
			}
			break;
			case 0xb0: {	                    // set modulator volume
				uint8_t vol = eff_op << 2;
				if( m_instr[inst][8] & 1 )
					m_opl.writeReg( 0x40 + op_table[chan], vol | ( m_instr[m_channels[chan].instr][3] & ~63 ) );
				else
					m_opl.writeReg( 0x40 + op_table[chan], vol | ( m_instr[inst][3] & ~63 ) );
			}
			break;
			case 0xc0: {	                    // set instrument volume
				uint8_t db = eff_op << 2;
				m_opl.writeReg( 0x43 + op_table[chan], db | ( m_instr[m_channels[chan].instr][2] & ~63 ) );
				if( m_instr[inst][8] & 1 )
					m_opl.writeReg( 0x43 + op_table[chan], db | ( m_instr[m_channels[chan].instr][2] & ~63 ) );
			}
			break;
			case 0xd0:
				m_patBreak++;
				setOrder(eff_op, estimate);
				break;	// position jump
			case 0xf0: // set m_speed
				m_speed = eff_op;
				m_speedCountdown = ++m_speed;
				break;
		}

		if( m_fadeIn )						// fade-in volume setting
			setVolume( chan, m_fadeIn * 2, m_fadeIn * 2 );

		if( !note )						// note handling
			continue;
		note--;

		if( ( note == 0x7f - 1 ) || ( ( note / 12 ) & ~7 ) ) { // pause (7fh)
			m_fnum[chan] &= ~0x20; // key off
			m_opl.writeReg( 0xb0 + chan, m_fnum[chan] );
			continue;
		}

		// play the note
		uint8_t Okt = ( ( note / 12 ) & 7 ) << 2;
		uint16_t Fnr = note_table[( note % 12 )] + m_instr[inst][11] + m_channels[chan].slide;
		m_channels[chan].frq = Fnr;
		if( !m_mode6 || chan < 6 )
			m_fnum[chan] = Okt | 0x20;
		else
			m_fnum[chan] = Okt;		// never set key for drums
		m_opl.writeReg( 0xb0 + chan, 0 );
		setFreq( chan, Fnr );
		if( m_mode6 ) {
			switch( chan ) {		// play drums
				case 6:
					m_opl.writeReg( 0xbd, m_bd & ~16 );
					m_bd |= 48;
					break;	// bass drum
				case 7:
					m_opl.writeReg( 0xbd, m_bd & ~1 );
					m_bd |= 33;
					break;	// hihat
				case 8:
					m_opl.writeReg( 0xbd, m_bd & ~2 );
					m_bd |= 34;
					break;	// cymbal
			}
			m_opl.writeReg( 0xbd, m_bd );
		}
	}

	m_speedCountdown = m_speed;		// player m_speed-timing
	if( m_patBreak ) {		// do post-effect handling
		state().row = 0;			// pattern break!
		m_patBreak = 0;
		setOrder( (state().order+1)%50, estimate );
		if( state().order == 0 )
			return false;
	}
	else {
		state().row++;
		state().row &= 63;		// advance in pattern data
		if( state().row == 0 ) {
			setOrder( (state().order+1)%50, estimate );
			if( state().order == 0 )
				return false;
		}
	}
	return state().pattern != 0xff;		// still playing
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
