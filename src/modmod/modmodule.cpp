#include "modmodule.h"
#include "modorder.h"

#include "stream/iarchive.h"
#include "stream/fbinstream.h"

#include "stuff/moduleregistry.h"

#include <boost/exception/all.hpp>
#include <boost/format.hpp>

namespace ppp
{
namespace mod
{

GenModule::Ptr ModModule::factory( const std::string& filename, uint32_t frequency, uint8_t maxRpt )
{
	ModModule::Ptr result( new ModModule( maxRpt ) );
	if( !result->load( filename ) ) {
		return GenModule::Ptr();
	}
	if( !result->initialize( frequency ) ) {
		return GenModule::Ptr();
	}
	return result;
}

ModModule::ModModule( uint8_t maxRpt ): GenModule( maxRpt ),
	m_samples(), m_patterns(), m_channels(), m_patLoopRow( -1 ),
	m_patLoopCount( -1 ), m_breakRow( -1 ), m_patDelayCount( -1 ), m_breakOrder( ~0 )
{
}

ModModule::~ModModule() = default;

ModSample::Ptr ModModule::sampleAt( size_t idx ) const
{
	if( idx == 0 ) {
		return ModSample::Ptr();
	}
	idx--;
	if( idx >= m_samples.size() ) {
		return ModSample::Ptr();
	}
	return m_samples.at( idx );
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
	logger()->info( L4CXX_LOCATION, boost::format( "Opening '%s'" ) % filename );
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
	logger()->debug( L4CXX_LOCATION, boost::format( "%d-channel, ID '%s', Tracker '%s'" ) % ( meta.channels + 0 ) % meta.id % meta.tracker );
	metaInfo().trackerInfo = meta.tracker;
	for( int i = 0; i < meta.channels; i++ ) {
		m_channels.push_back( ModChannel::Ptr( new ModChannel( this ) ) );
	}
	stream.seek( 20 );
	for( uint8_t i = 0; i < 31; i++ ) {
		ModSample::Ptr smp( new ModSample() );
		if( !smp->loadHeader( stream ) ) {
			logger()->warn( L4CXX_LOCATION, "Sample header could not be loaded" );
			return false;
		}
		m_samples.push_back( smp );
	}
	uint8_t maxPatNum = 0;
	{
		// load orders
		uint8_t songLen;
		stream.read( &songLen );
		if( songLen > 128 ) {
			songLen = 128;
		}
		logger()->debug( L4CXX_LOCATION, boost::format( "Song length: %d" ) % ( songLen + 0 ) );
		uint8_t tmp;
		stream.read( &tmp ); // skip the restart pos
		for( uint8_t i = 0; i < songLen; i++ ) {
			stream.read( &tmp );
			if( tmp >= 64 ) {
				continue;
			}
			if( tmp > maxPatNum ) {
				maxPatNum = tmp;
			}
			logger()->trace(L4CXX_LOCATION, boost::format("Order %d index: %d")%(i+0)%(tmp+0));
			addOrder( GenOrder::Ptr( new ModOrder( tmp ) ) );
		}
		stream.seekrel(128-songLen);
	}
	stream.seekrel( 4 ); // skip the ID
	logger()->debug( L4CXX_LOCATION, boost::format( "%d patterns @ %#x" ) % ( maxPatNum + 0 ) % stream.pos() );
	for( uint8_t i = 0; i <= maxPatNum; i++ ) {
		ModPattern::Ptr pat( new ModPattern() );
		logger()->debug(L4CXX_LOCATION, boost::format("Loading pattern %u")%(i+0));
		if( !pat->load( stream, meta.channels ) ) {
			logger()->warn( L4CXX_LOCATION, "Could not load pattern" );
			return false;
		}
		m_patterns.push_back( pat );
	}
	logger()->debug( L4CXX_LOCATION, boost::format( "Sample start @ %#x" ) % stream.pos() );
	for( const ModSample::Ptr & smp : m_samples ) {
		if( !smp->loadData( stream ) ) {
			logger()->warn( L4CXX_LOCATION, "Could not load sample data" );
		}
	}
	logger()->debug( L4CXX_LOCATION, boost::format( "pos=%#x size=%#x delta=%#x" ) % stream.pos() % stream.size() % ( stream.size() - stream.pos() ) );
	return stream.good();
}

size_t ModModule::internal_buildTick( AudioFrameBuffer* buf )
{
	try {
		//PPP_TEST(!buf);
		if( buf && !buf->get() ) {
			buf->reset( new AudioFrameBuffer::element_type );
		}
		if( state().tick == 0 ) {
			checkGlobalFx();
		}
		//buf->resize(getTickBufLen());
		//buf->clear();
		if( orderAt( state().order )->playbackCount() >= maxRepeat() ) {
			logger()->info( L4CXX_LOCATION, "Song end reached: Maximum repeat count reached" );
			if(buf) {
				buf->reset();
			}
			return 0;
		}
		// update channels...
		state().pattern = orderAt( state().order )->index();
		ModPattern::Ptr currPat = getPattern( state().pattern );
		if( !currPat ) {
			return 0;
		}
		if(buf) {
			MixerFrameBuffer mixerBuffer( new MixerFrameBuffer::element_type( tickBufferLength() ) );
			for( unsigned short currTrack = 0; currTrack < channelCount(); currTrack++ ) {
				ModChannel::Ptr chan = m_channels.at( currTrack );
				BOOST_ASSERT( chan.use_count() > 0 );
				ModCell::Ptr cell = currPat->cellAt( currTrack, state().row );
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
			for( unsigned short currTrack = 0; currTrack < channelCount(); currTrack++ ) {
				ModChannel::Ptr chan = m_channels.at( currTrack );
				BOOST_ASSERT( chan.use_count() > 0 );
				ModCell::Ptr cell = currPat->cellAt( currTrack, state().row );
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
	BOOST_ASSERT( orderCount() != 0 );
	bool orderChanged = false;
	if( state().tick == 0 ) {
		m_patDelayCount = -1;
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
	BOOST_ASSERT( idx < m_channels.size() );
	return m_channels.at( idx )->cellString();
}

std::string ModModule::internal_channelStatus( size_t idx ) const
{
	BOOST_ASSERT( idx < m_channels.size() );
	return m_channels.at( idx )->statusString();
}

IArchive& ModModule::serialize( IArchive* data )
{
	GenModule::serialize( data )
	% m_breakRow
	% m_breakOrder
	% m_patLoopRow
	% m_patLoopCount
	% m_patDelayCount;
	for( ModChannel::Ptr & chan : m_channels ) {
		if( !chan ) {
			continue;
		}
		data->archive( chan.get() );
	}
	return *data;
}

uint8_t ModModule::internal_channelCount() const
{
	return m_channels.size();
}

bool ModModule::existsSample( size_t idx ) const
{
	if( idx == 0 || idx > 30 ) {
		return false;
	}
	return m_samples.at( idx - 1 )->length() > 0;
}

void ModModule::checkGlobalFx()
{
	try {
		state().pattern = orderAt( state().order )->index();
		ModPattern::Ptr currPat = getPattern( state().pattern );
		if( !currPat )
			return;
		// check for pattern loops
		int patLoopCounter = 0;
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			ModCell::Ptr cell = currPat->cellAt( currTrack, state().row );
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
		uint8_t patDelayCounter = 0;
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			ModCell::Ptr cell = currPat->cellAt( currTrack, state().row );
			if( !cell ) continue;
			if( cell->effect() == 0x0f ) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if( fx != 0x0e ) continue;
			if( highNibble( fxVal ) != 0x0e ) continue;
			if( lowNibble( fxVal ) == 0 ) continue;
			if( ++patDelayCounter != 1 ) continue;
			if( m_patDelayCount != -1 ) continue;
			m_patDelayCount = lowNibble( fxVal );
		}
		if( m_patDelayCount > 1 )
			m_patDelayCount--;
		else
			m_patDelayCount = -1;
		// now check for breaking effects
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			if( m_patLoopCount != -1 ) break;
			ModCell::Ptr cell = currPat->cellAt( currTrack, state().row );
			if( !cell ) continue;
			if( cell->effect() == 0x0f ) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if( fx == 0x0b ) {
				m_breakOrder = fxVal;
			}
			else if( fx == 0x0d ) {
				m_breakRow = highNibble( fxVal ) * 10 + lowNibble( fxVal );
				logger()->info( L4CXX_LOCATION, boost::format( "Row %1%: Break pattern to row %2%" ) % state().row % ( m_breakRow + 0 ) );
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

ModPattern::Ptr ModModule::getPattern( size_t idx ) const
{
	//if( idx >= m_patterns.size() ) return ModPattern::Ptr();
	return m_patterns.at( idx );
}

light4cxx::Logger::Ptr ModModule::logger()
{
	return light4cxx::Logger::get( GenModule::logger()->name() + ".mod" );
}


}
}
