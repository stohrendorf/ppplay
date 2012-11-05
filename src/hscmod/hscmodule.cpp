#include "hscmodule.h"

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
	return res;
}

size_t Module::internal_buildTick( AudioFrameBuffer* buf )
{
	BOOST_ASSERT( buf != nullptr );
	if( !*buf ) {
		buf->reset( new AudioFrameBuffer::element_type );
	}
	( *buf )->resize( opl::Opl3::SampleRate / 18.2 );
	return ( *buf )->size();
}

std::string Module::internal_channelCellString( size_t idx ) const
{
	return "---"; // FIXME
}

int Module::internal_channelCount() const
{
	return 9;
}

std::string Module::internal_channelStatus( size_t idx ) const
{
	return "blubber"; // FIXME
}

bool Module::load( Stream* stream )
{
	if( !boost::ends_with( boost::to_lower_copy( stream->name() ), ".hsc" ) || stream->size() > 59187 ) {
		return false;
	}
	stream->seek( 0 );
	stream->read( static_cast<char*>( m_instr ), 128 * 12 );
	for( int i = 0; i < 128; i++ ) {
		m_instr[i][2] ^= ( m_instr[i][2] & 0x40 ) << 1;
		m_instr[i][3] ^= ( m_instr[i][3] & 0x40 ) << 1;
		m_instr[i][11] >>= 4;
	}
	stream->read( m_orders, 51 );
	stream->read( static_cast<char*>( m_patterns ), 50 * 64 * 9 );
	return true;
}

Module::Module( int maxRpt, ppp::Sample::Interpolation inter ) : ppp::AbstractModule( maxRpt, inter ), m_opl()
{
}

Module::~Module()
{
}

AbstractArchive& Module::serialize( AbstractArchive* data )
{
	return ppp::AbstractModule::serialize( data );
}

void Module::storeInstr( uint8_t chan, uint8_t instr )
{
	const uint8_t* insData = m_instr[instr];
	const uint8_t op = op_table[chan];
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

void Module::setVolume( uint8_t chan, uint8_t volCarrier, uint8 volModulator )
{
	const uint8_t* insData = m_instr[instr];
	const uint8_t op = op_table[chan];

	m_opl.writeReg( 0x43 + op, volCarrier | ( insData[2] & ~63 ) );
	if( insData[8] & 1 ) // carrier
		m_opl.writeReg( 0x40 + op, volModulator | ( insData[3] & ~63 ) );
	else
		m_opl.writeReg( 0x40 + op, insData[3] ); // modulator
}

bool Module::update()
{
	// general vars
	unsigned char		chan, pattnr, inst, vol, Okt, db;
	unsigned short	Fnr;

	m_speedCountdown--;                      // player m_speed handling
	if( m_speedCountdown )
		return !songend;		// nothing done

	if( fadein )					// fade-in handling
		fadein--;

	pattnr = m_orders[m_order];
	if( pattnr == 0xff ) {			// arrangement handling
		songend = 1;				// set end-flag
		m_order = 0;
		pattnr = m_orders[m_order];
	}
	else if( ( pattnr & 128 ) && ( pattnr <= 0xb1 ) ) { // goto pattern "nr"
		m_order = m_orders[m_order] & 127;
		m_row = 0;
		pattnr = m_orders[m_order];
		songend = 1;
	}

	uint32_t pattoff = m_row * 9;
	for( chan = 0; chan < 9; chan++ ) {			// handle all channels
		uint8_t note = m_patterns[pattnr][pattoff].note;
		uint8_t effect = m_patterns[pattnr][pattoff].effect;
		pattoff++;

		if( note & 128 ) {                  // set instrument
			storeInstr( chan, effect );
			continue;
		}
		uint8_t eff_op = effect & 0x0f;
		inst = m_channels[chan].instr;
		if( note )
			m_channels[chan].slide = 0;

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
						pattbreak++;
						break;	// jump to next pattern
					case 3:
						fadein = 31;
						break;	// fade in (divided by 2)
					case 5:
						mode6 = 1;
						break;	// 6 voice mode on
					case 6:
						mode6 = 0;
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
					setfreq( chan, m_channels[chan].frq );
				break;
			case 0x50:							// set percussion instrument (unimplemented)
				break;
			case 0x60:							// set feedback
				m_opl.writeReg( 0xc0 + chan, ( m_instr[m_channels[chan].instr][8] & 1 ) + ( eff_op << 1 ) );
				break;
			case 0xa0:		                    // set carrier volume
				vol = eff_op << 2;
				m_opl.writeReg( 0x43 + op_table[chan], vol | ( m_instr[m_channels[chan].instr][2] & ~63 ) );
				break;
			case 0xb0:		                    // set modulator volume
				vol = eff_op << 2;
				if( m_instr[inst][8] & 1 )
					m_opl.writeReg( 0x40 + op_table[chan], vol | ( m_instr[m_channels[chan].instr][3] & ~63 ) );
				else
					m_opl.writeReg( 0x40 + op_table[chan], vol | ( m_instr[inst][3] & ~63 ) );
				break;
			case 0xc0:		                    // set instrument volume
				db = eff_op << 2;
				m_opl.writeReg( 0x43 + op_table[chan], db | ( m_instr[m_channels[chan].instr][2] & ~63 ) );
				if( m_instr[inst][8] & 1 )
					m_opl.writeReg( 0x40 + op_table[chan], db | ( m_instr[m_channels[chan].instr][3] & ~63 ) );
				break;
			case 0xd0:
				pattbreak++;
				m_order = eff_op;
				songend = 1;
				break;	// position jump
			case 0xf0: // set m_speed
				m_speed = eff_op;
				m_speedCountdown = ++m_speed;
				break;
		}

		if( fadein )						// fade-in volume setting
			setVolume( chan, fadein * 2, fadein * 2 );

		if( !note )						// note handling
			continue;
		note--;

		if( ( note == 0x7f - 1 ) || ( ( note / 12 ) & ~7 ) ) { // pause (7fh)
			m_fnum[chan] &= ~32;
			m_opl.writeReg( 0xb0 + chan, m_fnum[chan] );
			continue;
		}

		// play the note
		if( mtkmode )		// imitate MPU-401 Trakker bug
			note--;
		Okt = ( ( note / 12 ) & 7 ) << 2;
		Fnr = note_table[( note % 12 )] + m_instr[inst][11] + m_channels[chan].slide;
		m_channels[chan].frq = Fnr;
		if( !mode6 || chan < 6 )
			m_fnum[chan] = Okt | 32;
		else
			m_fnum[chan] = Okt;		// never set key for drums
		m_opl.writeReg( 0xb0 + chan, 0 );
		setFreq( chan, Fnr );
		if( mode6 ) {
			switch( chan ) {		// play drums
				case 6:
					m_opl.writeReg( 0xbd, bd & ~16 );
					bd |= 48;
					break;	// bass drum
				case 7:
					m_opl.writeReg( 0xbd, bd & ~1 );
					bd |= 33;
					break;	// hihat
				case 8:
					m_opl.writeReg( 0xbd, bd & ~2 );
					bd |= 34;
					break;	// cymbal
			}
			m_opl.writeReg( 0xbd, bd );
		}
	}

	m_speedCountdown = m_speed;		// player m_speed-timing
	if( pattbreak ) {		// do post-effect handling
		m_row = 0;			// pattern break!
		pattbreak = 0;
		m_order++;
		m_order %= 50;
		if( !m_order )
			songend = 1;
	}
	else {
		m_row++;
		m_row &= 63;		// advance in pattern data
		if( !m_row ) {
			m_order++;
			m_order %= 50;
			if( !m_order )
				songend = 1;
		}
	}
	return !songend;		// still playing
}

void Module::setFreq( uint8_t chan, uint16_t frq )
{
	m_fnum[chan] = ( m_fnum[chan] & ~3 ) | ( frq >> 8 );

	m_opl.writeReg( 0xa0 + chan, frq & 0xff );
	m_opl.writeReg( 0xb0 + chan, m_fnum[chan] );
}

}
