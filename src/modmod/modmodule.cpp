#include "modmodule.h"
#include "modorder.h"
#include "modsample.h"
#include "modchannel.h"
#include "modpattern.h"
#include "modcell.h"

#include "stream/iarchive.h"
#include "stream/fbinstream.h"

#include "stuff/moduleregistry.h"

#include <boost/exception/all.hpp>
#include <boost/format.hpp>

#include <array>

namespace ppp
{
namespace mod
{

GenModule::Ptr ModModule::factory( const std::string& filename, uint32_t frequency, int maxRpt )
{
	ModModule* result = new ModModule( maxRpt );
	if( !result->load( filename ) ) {
		delete result;
		return GenModule::Ptr();
	}
	if( !result->initialize( frequency ) ) {
		delete result;
		return GenModule::Ptr();
	}
	return GenModule::Ptr(result);
}

ModModule::ModModule( int maxRpt ): GenModule( maxRpt ),
	m_samples(), m_patterns(), m_channels(), m_patLoopRow( -1 ),
	m_patLoopCount( -1 ), m_breakRow( -1 ), m_patDelayCount( -1 ), m_breakOrder( ~0 )
{
}

ModModule::~ModModule()
{
	deleteAll(m_samples);
	deleteAll(m_patterns);
	deleteAll(m_channels);
}

ModSample* ModModule::sampleAt( size_t idx ) const
{
	if( idx == 0 ) {
		return nullptr;
	}
	idx--;
	if( idx >= m_samples.size() ) {
		return nullptr;
	}
	return m_samples[ idx ];
}

namespace
{
/**
 * @brief Maps module IDs to their respective channel counts
 */
struct IdMetaInfo {
	const std::string id;
	const uint8_t channels;
	const std::string tracker;
};

const std::array<const IdMetaInfo, 31> idMetaData  = {{
		{"M.K.", 4, "ProTracker"},
		{"M!K!", 4, "ProTracker"},
		{"FLT4", 4, "Startrekker"},
		{"FLT8", 8, "Startrekker"},
		{"CD81", 8, "Falcon"}, //< @todo Check tracker name
		{"TDZ1", 1, "TakeTracker"},
		{"TDZ2", 2, "TakeTracker"},
		{"TDZ3", 3, "TakeTracker"},
		{"5CHN", 5, "TakeTracker"},
		{"7CHN", 7, "TakeTracker"},
		{"9CHN", 9, "TakeTracker"},
		{"11CH", 11, "TakeTracker"},
		{"13CH", 13, "TakeTracker"},
		{"15CH", 15, "TakeTracker"},
		{"2CHN", 2, "FastTracker"},
		{"4CHN", 4, "FastTracker"},
		{"6CHN", 6, "FastTracker"},
		{"8CHN", 8, "FastTracker"},
		{"10CH", 10, "FastTracker"},
		{"12CH", 12, "FastTracker"},
		{"14CH", 14, "FastTracker"},
		{"16CH", 16, "FastTracker"},
		{"18CH", 18, "FastTracker"},
		{"20CH", 20, "FastTracker"},
		{"22CH", 22, "FastTracker"},
		{"24CH", 24, "FastTracker"},
		{"26CH", 26, "FastTracker"},
		{"28CH", 28, "FastTracker"},
		{"30CH", 30, "FastTracker"},
		{"32CH", 32, "FastTracker"},
		{"OCTA", 8, "Octalyzer"}, //< @todo Check tracker name
	}
};

IdMetaInfo findMeta( BinStream* stream )
{
	static const IdMetaInfo none = {"", 0, ""};
	char id[5];
	stream->read( id, 4 );
	id[4] = '\0';
	for( const IdMetaInfo & mi : idMetaData ) {
		if( id == mi.id ) {
			return mi;
		}
	}
	// revert...
	stream->seekrel( -4 );
	return none;
}

} // anonymous namespace

bool ModModule::load( const std::string& filename )
{
	logger()->info( L4CXX_LOCATION, "Opening '%s'", filename );
	FBinStream stream( filename );
	if( !stream.isOpen() ) {
		logger()->warn( L4CXX_LOCATION, "Could not open file" );
		return false;
	}
	metaInfo().filename = filename;
	setTempo( 125 );
	setSpeed( 6 );
	state().globalVolume = 0x40;
	char modName[20];
	stream.read( modName, 20 );
	metaInfo().title = stringncpy( modName, 20 );
	// check 31-sample mod
	logger()->info( L4CXX_LOCATION, "Probing meta-info for 31-sample mod..." );
	stream.seek( 1080 );
	IdMetaInfo meta = findMeta( &stream );
	if( meta.channels == 0 ) {
		logger()->warn( L4CXX_LOCATION, "Could not find a valid module ID" );
		return false;
	}
	logger()->debug( L4CXX_LOCATION, "%d-channel, ID '%s', Tracker '%s'", int(meta.channels), meta.id, meta.tracker );
	metaInfo().trackerInfo = meta.tracker;
	for( int i = 0; i < meta.channels; i++ ) {
		m_channels.push_back( new ModChannel( this, ((i+1)&2)==0 ) );
	}
	stream.seek( 20 );
	for( int i = 0; i < 31; i++ ) {
		ModSample* smp = new ModSample();
		m_samples.push_back( smp );
		if( !smp->loadHeader( stream ) ) {
			logger()->warn( L4CXX_LOCATION, "Sample header could not be loaded" );
			return false;
		}
	}
	uint8_t maxPatNum = 0;
	{
		// load orders
		uint8_t songLen;
		stream.read( &songLen );
		if( songLen > 128 ) {
			songLen = 128;
		}
		logger()->debug( L4CXX_LOCATION, "Song length: %d", int(songLen) );
		uint8_t tmp;
		stream.read( &tmp ); // skip the restart pos
		for( uint_fast8_t i = 0; i < songLen; i++ ) {
			stream.read( &tmp );
			if( tmp >= 64 ) {
				continue;
			}
			if( tmp > maxPatNum ) {
				maxPatNum = tmp;
			}
			logger()->trace(L4CXX_LOCATION, "Order %d index: %d", int(i), int(tmp));
			addOrder( new ModOrder( tmp ) );
		}
		stream.seekrel(128-songLen);
	}
	stream.seekrel( 4 ); // skip the ID
	logger()->debug( L4CXX_LOCATION, "%d patterns @ %#x", int(maxPatNum), stream.pos() );
	for( uint_fast8_t i = 0; i <= maxPatNum; i++ ) {
		ModPattern* pat = new ModPattern();
		m_patterns.push_back( pat );
		logger()->debug(L4CXX_LOCATION, "Loading pattern %u", int(i));
		if( !pat->load( stream, meta.channels ) ) {
			logger()->warn( L4CXX_LOCATION, "Could not load pattern" );
			return false;
		}
	}
	logger()->debug( L4CXX_LOCATION, "Sample start @ %#x", stream.pos() );
	for( auto& smp : m_samples ) {
		if( !smp->loadData( stream ) ) {
			logger()->warn( L4CXX_LOCATION, "Could not load sample data" );
		}
	}
	logger()->debug( L4CXX_LOCATION, "pos=%#x size=%#x delta=%#x", stream.pos(), stream.size(), stream.size() - stream.pos() );
	return stream.good();
}

size_t ModModule::internal_buildTick( AudioFrameBuffer* buf )
{
	if(state().tick==0 && state().order >= orderCount()) {
		if(buf) {
			buf->reset();
		}
		return 0;
	}
	try {
		if( buf && !buf->get() ) {
			buf->reset( new AudioFrameBuffer::element_type );
		}
		if( state().tick == 0 ) {
			checkGlobalFx();
		}
		if( orderAt( state().order )->playbackCount() >= maxRepeat() ) {
			logger()->info( L4CXX_LOCATION, "Song end reached: Maximum repeat count reached" );
			if(buf) {
				buf->reset();
			}
			return 0;
		}
		// update channels...
		state().pattern = orderAt( state().order )->index();
		ModPattern* currPat = getPattern( state().pattern );
		if( !currPat ) {
			return 0;
		}
		if(buf) {
			MixerFrameBuffer mixerBuffer( new MixerFrameBuffer::element_type( tickBufferLength() ) );
			for( int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
				ModChannel* chan = m_channels[ currTrack ];
				ModCell* cell = currPat->cellAt( currTrack, state().row );
				chan->update( cell, false ); // m_patDelayCount != -1);
				chan->mixTick( &mixerBuffer );
			}
			buf->get()->resize( mixerBuffer->size() );
			MixerSampleFrame* mixerBufferPtr = &mixerBuffer->front();
			BasicSampleFrame* bufPtr = &buf->get()->front();
			for( size_t i = 0; i < mixerBuffer->size(); i++ ) {  // postprocess...
				*bufPtr = mixerBufferPtr->rightShiftClip(2);
				bufPtr++;
				mixerBufferPtr++;
			}
		}
		else {
			for( int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
				ModChannel* chan = m_channels[ currTrack ];
				ModCell* cell = currPat->cellAt( currTrack, state().row );
				chan->update( cell, false ); // m_patDelayCount != -1);
				chan->mixTick( nullptr );
			}
		}
		nextTick();
		if( !adjustPosition( !buf ) ) {
			logger()->info( L4CXX_LOCATION, "Song end reached: adjustPosition() failed" );
			if(buf) {
				buf->reset();
			}
			return 0;
		}
		state().playedFrames += tickBufferLength();
		return tickBufferLength();
	}
	catch( ... ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
}

bool ModModule::adjustPosition( bool estimateOnly )
{
	bool orderChanged = false;
	if( m_patDelayCount != -1 ) {
		m_patDelayCount--;
	}
	if( state().tick == 0 && m_patDelayCount == -1 ) {
		if( m_breakOrder != 0xffff ) {
			logger()->debug( L4CXX_LOCATION, "Order break" );
			if( m_breakOrder < orderCount() ) {
				setOrder( m_breakOrder, estimateOnly, m_breakRow == -1 );
				orderChanged = true;
			}
			setRow( 0 );
		}
		if( m_breakRow != -1 ) {
			if( m_breakRow <= 63 ) {
				setRow( m_breakRow );
			}
			if( m_breakOrder == 0xffff ) {
				if( m_patLoopCount == -1 ) {
					logger()->debug( L4CXX_LOCATION, "Row break" );
					setOrder( state().order + 1, estimateOnly );
					orderChanged = true;
				}
				//else {
				//	LOG_MESSAGE(stringf("oO... aPatLoopCount=%d",aPatLoopCount));
				//}
			}
		}
		if( m_breakRow == -1 && m_breakOrder == 0xffff && m_patDelayCount == -1 ) {
			setRow( ( state().row + 1 ) & 0x3f );
			if( state().row == 0 ) {
				setOrder( state().order + 1, estimateOnly );
				orderChanged = true;
			}
		}
		m_breakRow = -1;
		m_breakOrder = ~0;
	}
	if( state().order >= orderCount() ) {
		logger()->debug( L4CXX_LOCATION, "state().order>=orderCount()" );
		return false;
	}
	if( orderChanged ) {
		m_patLoopRow = 0;
		m_patLoopCount = -1;
		state().pattern = orderAt( state().order )->index();
		setOrder(state().order, estimateOnly);
	}
	return true;
}

std::string ModModule::internal_channelCellString( size_t idx ) const
{
	if( idx >= m_channels.size() ) {
		throw std::out_of_range("Requested channel index out of range");
	}
	return m_channels[ idx ]->cellString();
}

std::string ModModule::internal_channelStatus( size_t idx ) const
{
	if( idx >= m_channels.size() ) {
		throw std::out_of_range("Requested channel index out of range");
	}
	return m_channels[ idx ]->statusString();
}

IArchive& ModModule::serialize( IArchive* data )
{
	GenModule::serialize( data )
	% m_breakRow
	% m_breakOrder
	% m_patLoopRow
	% m_patLoopCount
	% m_patDelayCount;
	for( auto& chan : m_channels ) {
		if( !chan ) {
			continue;
		}
		data->archive( chan );
	}
	return *data;
}

int ModModule::internal_channelCount() const
{
	return m_channels.size();
}

bool ModModule::existsSample( size_t idx ) const
{
	if( idx == 0 || idx > 30 ) {
		return false;
	}
	return m_samples[ idx - 1 ]->length() > 0;
}

void ModModule::checkGlobalFx()
{
	try {
		state().pattern = orderAt( state().order )->index();
		ModPattern* currPat = getPattern( state().pattern );
		if( !currPat )
			return;
		// check for pattern loops
		int patLoopCounter = 0;
		for( int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			ModCell* cell = currPat->cellAt( currTrack, state().row );
			if( !cell ) continue;
			if( cell->effect() == 0x0f ) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if( fx != 0x0e ) continue;
			if( highNibble( fxVal ) != 0x06 ) continue;
			if( lowNibble( fxVal ) == 0x00 ) {  // loop start
				m_patLoopRow = state().row;
			}
			else { // loop return
				patLoopCounter++;
				if( m_patLoopCount == -1 ) {  // first loop return -> set loop count
					m_patLoopCount = lowNibble( fxVal );
					m_breakRow = m_patLoopRow;
				}
				else if( m_patLoopCount > 1 ) {  // non-initial return -> decrease loop counter
					m_patLoopCount--;
					m_breakRow = m_patLoopRow;
				}
				else { // loops done...
					if( patLoopCounter == 1 ) {  // one loop, all ok
						m_patLoopCount = -1;
						m_breakRow = -1;
						m_patLoopRow = state().row + 1;
					}
					else { // we got an "infinite" loop...
						m_patLoopCount = 127;
						m_breakRow = m_patLoopRow;
						logger()->warn( L4CXX_LOCATION, "Infinite pattern loop detected" );
					}
				}
			}
		}
		// check for pattern delays
		for( int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			ModCell* cell = currPat->cellAt( currTrack, state().row );
			if( !cell ) continue;
			if( cell->effect() == 0x0f ) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if( fx != 0x0e ) continue;
			if( highNibble( fxVal ) != 0x0e ) continue;
			if( lowNibble( fxVal ) == 0 ) continue;
			if( m_patDelayCount != -1 ) continue;
			m_patDelayCount = lowNibble( fxVal );
		}
		if( m_patDelayCount > 1 )
			m_patDelayCount--;
		else
			m_patDelayCount = -1;
		// now check for breaking effects
		for( int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			if( m_patLoopCount != -1 ) break;
			ModCell* cell = currPat->cellAt( currTrack, state().row );
			if( !cell ) continue;
			if( cell->effect() == 0x0f ) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if( fx == 0x0b ) {
				m_breakOrder = fxVal;
			}
			else if( fx == 0x0d ) {
				m_breakRow = highNibble( fxVal ) * 10 + lowNibble( fxVal );
				logger()->info( L4CXX_LOCATION, "Row %1%: Break pattern to row %2%", state().row , int(m_breakRow) );
			}
		}
	}
	catch( boost::exception& e ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
	catch( ... ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( "Unknown exception" ) );
	}
}

ModPattern* ModModule::getPattern( size_t idx ) const
{
	if( idx >= m_patterns.size() ) {
		return nullptr;
	}
	return m_patterns[ idx ];
}

light4cxx::Logger* ModModule::logger()
{
	return light4cxx::Logger::get( GenModule::logger()->name() + ".mod" );
}


}
}
