/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @ingroup S3mMod
 * @{
 */

#include <boost/exception/all.hpp>

#include "s3mmodule.h"
#include "s3morder.h"
#include "s3mbase.h"
#include "s3mchannel.h"
#include "s3mpattern.h"
#include "s3msample.h"
#include "s3mcell.h"

#include "stream/fbinstream.h"

#include "stuff/moduleregistry.h"

#include <boost/format.hpp>

namespace ppp
{

namespace s3m
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS
enum {
	s3mFlagSt2Vibrato   = 0x01, //!< @brief Use st2 Vibrato, not supported
	s3mFlagSt2Tempo     = 0x02, //!< @brief Use st2 Tempo, not supported
	s3mFlagAmigaSlides  = 0x04, //!< @brief Use Amiga slides, not supported
	s3mFlag0volOpt      = 0x08, //!< @brief Turn off looping samples when Volume is 0 for at least 2 rows
	s3mFlagAmigaLimits  = 0x10, //!< @brief Stop pitch slide at b-5
	s3mFlagEnableSB     = 0x20, //!< @brief Enable filter with sfx/sb, not supported
	s3mFlag300Slides    = 0x40, //!< @brief Use STv3.00 Volume Slides: Slides even on tick 0
	s3mFlagSpecial      = 0x80, //!< @brief Module contains special custom data
	s3mChanUnused       = 0xff, //!< @brief Channel is unused
	s3mFlagChanDisabled = 0x80  //!< @brief Channel is disabled
};

typedef uint16_t ParaPointer;

inline int16_t s3mPostProcess( int32_t sample )
{
	return clip( sample >> 2, -32768, 32767 );
}

#pragma pack(push,1)
struct S3mModuleHeader {
	char title[28]; //!< @brief Title of the module, Zero-terminated
	uint8_t type[2]; //!< @brief Module type (@c 0x161a for s3m)
	uint16_t rsvd1; //!< @brief undefined
	uint16_t
	ordNum, //!< @brief Number of orders
	smpNum, //!< @brief Number of samples
	patNum, //!< @brief Number of patterns
	flags, //!< @brief Flags
	createdWith, //!< @brief Created with tracker (@code &0xfff @endcode=version, @code >>12 @endcode=tracker)
	ffv; //!< @brief File format version (1=signed samples/deprecated, 2=unsigned samples)
	char id[4]; //!< @brief @verbatim 'SCRM' @endverbatim
	uint8_t
	globalVolume, //!< @brief Global volume
	initialSpeed, //!< @brief Initial speed
	initialTempo, //!< @brief Initial tempo
	masterVolume, //!< @brief Master volume
	ultraClickRemoval, //!< @brief Don't bother with it...
	defaultPannings; //!< @brief @c 252 when there are default pannings present after the header
	uint8_t rsvd2[8]; //!< @brief Reserved
	ParaPointer specialPtr; //!< @brief ::ParaPointer to additional data
	uint8_t pannings[32];//!< @brief Channel pannings
};
#pragma pack(pop)
#endif

S3mModule::S3mModule( int maxRpt ) : GenModule( maxRpt ),
	m_breakRow( ~0 ), m_breakOrder( ~0 ), m_patLoopRow( -1 ), m_patLoopCount( -1 ), m_patDelayCount( -1 ),
	m_customData( false ), m_samples(), m_patterns(), m_channels(), m_usedChannels( 0 ),
	m_amigaLimits( false ), m_fastVolSlides( false ), m_st2Vibrato( false ), m_zeroVolOpt( false )
{
	try {
		for( uint16_t i = 0; i < 256; i++ ) {
			addOrder( new S3mOrder( s3mOrderEnd ) );
			m_patterns.push_back( nullptr );
			m_samples.push_back( nullptr );
		}
		for( S3mChannel*& chan : m_channels ) {
				chan = new S3mChannel( this );
			}
		}
	catch( boost::exception& e ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
	catch( ... ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( "Unknown exception" ) );
	}
}

S3mModule::~S3mModule()
{
	deleteAll(m_channels);
	deleteAll(m_patterns);
	deleteAll(m_samples);
}

bool S3mModule::load( const std::string& fn )
{
	try {
		//LOG_MESSAGE("Opening '%s'", fn.c_str());
		FBinStream str( fn );
		if( !str.isOpen() ) {
			//LOG_WARNING("%s could not be opened", fn.c_str());
			return false;
		}
		metaInfo().filename = fn;
		S3mModuleHeader s3mHdr;
		try {
			str.seek( 0 );
			str.read( reinterpret_cast<char*>( &s3mHdr ), sizeof( s3mHdr ) );
		}
		catch( ... ) {
			return false;
		}
		if( str.fail() ) {  // header read completely?
			logger()->warn( L4CXX_LOCATION, "Header Error" );
			return false;
		}
		if( !std::equal( s3mHdr.id, s3mHdr.id + 4, "SCRM" ) ) {
			logger()->warn( L4CXX_LOCATION, "Header ID Error" );
			return false;
		}
		s3mHdr.ordNum &= 0xff;
		s3mHdr.patNum &= 0xff;
		s3mHdr.smpNum &= 0xff;
		// many modules have 0x00 here, but they_re still correct
		//LOG_TEST_WARN(s3mHdr.type[0] != 0x1a);
		if( s3mHdr.createdWith == 0x1300 )
			s3mHdr.flags |= s3mFlag300Slides;
		m_st2Vibrato = ( s3mHdr.flags & s3mFlagSt2Vibrato ) != 0;
		if( m_st2Vibrato ) logger()->debug( L4CXX_LOCATION, "ST2 Vibrato (not supported)" );
		if( s3mHdr.flags & s3mFlagSt2Tempo ) logger()->debug( L4CXX_LOCATION, "ST2 Tempo (not supported)" );
		m_fastVolSlides = ( s3mHdr.flags & s3mFlag300Slides ) != 0;
		if( m_fastVolSlides ) logger()->debug( L4CXX_LOCATION, "ST v3.00 Volume Slides" );
		m_zeroVolOpt = ( s3mHdr.flags & s3mFlag0volOpt ) != 0;
		if( m_zeroVolOpt ) logger()->debug( L4CXX_LOCATION, "Zero-volume Optimization" );
		if( s3mHdr.flags & s3mFlagAmigaSlides ) logger()->debug( L4CXX_LOCATION, "Amiga slides (not supported)" );
		m_amigaLimits = ( s3mHdr.flags & s3mFlagAmigaLimits ) != 0;
		if( m_amigaLimits ) logger()->debug( L4CXX_LOCATION, "Amiga limits" );
		if( s3mHdr.flags & s3mFlagSpecial ) logger()->debug( L4CXX_LOCATION, "Special data present" );
		if( s3mHdr.defaultPannings == 0xFC ) logger()->debug( L4CXX_LOCATION, "Default Pannings present" );
		unsigned char schismTest = 0;
		switch( ( s3mHdr.createdWith >> 12 ) & 0x0f ) {
			case s3mTIdScreamTracker:
				metaInfo().trackerInfo = "ScreamTracker v";
				break;
			case s3mTIdImagoOrpheus:
				metaInfo().trackerInfo = "Imago Orpheus v";
				break;
			case s3mTIdImpulseTracker:
				metaInfo().trackerInfo = "Impulse Tracker v";
				break;
				// the following IDs were found in the Schism Tracker sources
			case s3mTIdSchismTracker:
				metaInfo().trackerInfo = "Schism Tracker v";
				schismTest = 1;
				break;  // some versions of Schism Tracker use ID 1
			case s3mTIdOpenMPT:
				metaInfo().trackerInfo = "OpenMPT v";
				break;
			default:
				metaInfo().trackerInfo = (boost::format( "Unknown Tracker (%x) v" ) % ( s3mHdr.createdWith >> 12 )).str();
		}
		metaInfo().trackerInfo =  (boost::format( "%s%x.%02x" ) % trackerInfo() % ( ( s3mHdr.createdWith >> 8 ) & 0x0f ) % ( s3mHdr.createdWith & 0xff )).str();
		setTempo( s3mHdr.initialTempo );
		//m_playbackInfo.speed = s3mHdr.initialSpeed;
		setSpeed( s3mHdr.initialSpeed );
		state().globalVolume = s3mHdr.globalVolume;
		for( S3mChannel* chan : m_channels ) {
			chan->recalcVolume();
		}
		// parse flags
		m_customData = ( s3mHdr.ffv & s3mFlagSpecial ) != 0;
		// now read the orders...
		str.seek( 0x60 );
		for( int i = 0; i < s3mHdr.ordNum; i++ ) {
			if( !str.good() ) {
				return false;
			}
			uint8_t tmpOrd;
			str.read( &tmpOrd );
			orderAt( i )->setIndex( tmpOrd );
		}
		uint8_t defPans[32];
		if( s3mHdr.defaultPannings == 0xFC ) {
			try {
				str.seek( 0x60 + s3mHdr.ordNum + 2 * s3mHdr.smpNum + 2 * s3mHdr.patNum );
				str.read( defPans, 32 );
			}
			catch( ... ) {
				return false;
			}
			for( int i = 0; i < 32; i++ ) {
				schismTest |= ( defPans[i] & 0x10 ) != 0;
			}
		}
		// load the samples
		logger()->info( L4CXX_LOCATION, "Loading samples..." );
		for( int i = 0; i < s3mHdr.smpNum; i++ ) {
			if( !str.good() ) {
				return false;
			}
			str.seek( s3mHdr.ordNum + 0x60 + 2 * i );
			if( !str.good() ) {
				return false;
			}
			ParaPointer pp;
			str.read( &pp );
			if( pp == 0 )
				continue;
			m_samples.at( i ) = new S3mSample();
			if( !( m_samples.at( i )->load( str, pp * 16, ( ( s3mHdr.createdWith >> 12 ) & 0x0f ) == s3mTIdImagoOrpheus ) ) ) {
				return false;
			}
			if( !str.good() ) {
				return false;
			}
			schismTest |= m_samples.at( i )->isHighQuality();
		}
		if( schismTest != 0 ) {
			logger()->info( L4CXX_LOCATION, "Enabling Schism Tracker compatibility mode" );
		}
		// ok, samples loaded, now load the patterns...
		logger()->info( L4CXX_LOCATION, "Loading patterns..." );
		for( int i = 0; i < s3mHdr.patNum; i++ ) {
			str.seek( s3mHdr.ordNum + 2 * s3mHdr.smpNum + 0x60 + 2 * i );
			ParaPointer pp;
			str.read( &pp );
			if( pp == 0 )
				continue;
			S3mPattern* pat = new S3mPattern();
			m_patterns.at( i ) = pat;
			if( !pat->load( str, pp * 16 ) ) {
				return false;
			}
			if( !str.good() ) {
				return false;
			}
		}
		//str.close();
		// set pannings...
		logger()->info( L4CXX_LOCATION, "Preparing channels..." );
		for( int i = 0; i < 32; i++ ) {
			S3mChannel* s3mChan = new S3mChannel( this );
			m_channels.at( i ) = s3mChan;
			if( ( s3mHdr.pannings[i] & 0x80 ) != 0 ) {
				s3mChan->disable();
				continue;
			}
			else {
				s3mChan->enable();
				m_usedChannels = i + 1;
			}
			if( s3mHdr.defaultPannings != 0xFC ) {  // no pannings
				if( ( s3mHdr.masterVolume & 0x80 ) != 0 ) { // stereo
					if( ( s3mHdr.pannings[i] & 0x08 ) != 0 ) // left channel
						s3mChan->setPanning( 0x03 * 0x40 / 0x0f );
					else // right channel
						s3mChan->setPanning( 0x0c * 0x40 / 0x0f );
				}
				else { // mono
					s3mChan->setPanning( 0x20 );
				}
			}
			else { // panning settings are there...
				if( ( defPans[i] & 0x20 ) == 0 ) { // use defaults
					if( ( s3mHdr.masterVolume & 0x80 ) != 0 ) { // stereo
						if( ( s3mHdr.pannings[i] & 0x08 ) != 0 ) // left channel
							s3mChan->setPanning( 0x03 * 0x40 / 0x0f );
						else // right channel
							s3mChan->setPanning( 0x0c * 0x40 / 0x0f );
					}
					else { // mono
						s3mChan->setPanning( 0x20 );
					}
				}
				else { // use panning settings...
					s3mChan->setPanning( ( defPans[i] & 0x0f ) * 0x40 / 0x0f );
				}
			}
		}
		metaInfo().title = stringncpy( s3mHdr.title, 28 );
		return true;
	}
	catch( boost::exception& e ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
	catch( ... ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( "Unknown exception" ) );
	}
}

bool S3mModule::existsSample( int16_t idx )
{
	idx--;
	if( !inRange<int>( idx, 0, m_samples.size() - 1 ) )
		return false;
	return m_samples.at( idx ) != nullptr;
}

uint8_t S3mModule::internal_channelCount() const
{
	return m_usedChannels;
}

void S3mModule::checkGlobalFx()
{
	try {
		state().pattern = orderAt( state().order )->index();
		S3mPattern* currPat = getPattern( state().pattern );
		if( !currPat ) {
			return;
		}
		// check for pattern loops
		int patLoopCounter = 0;
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mCell* cell = currPat->cellAt( currTrack, state().row );
			if( !cell || cell->effect() == s3mEmptyCommand ) {
				continue;
			}
			uint8_t fx = cell->effect();
			if( fx != s3mFxSpecial ) {
				continue;
			}
			uint8_t fxVal = cell->effectValue();
			if( highNibble( fxVal ) != s3mSFxPatLoop ) {
				continue;
			}
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
						m_breakRow = ~0;
						m_patLoopRow = state().row + 1;
					}
					else { // we got an "infinite" loop...
						m_patLoopCount = 127;
						m_breakRow = m_patLoopRow;
						logger()->info( L4CXX_LOCATION, "Infinite pattern loop detected" );
					}
				}
			}
		}
		// check for pattern delays
		uint8_t patDelayCounter = 0;
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			const S3mCell* cell = currPat->cellAt( currTrack, state().row );
			if( !cell || cell->effect() == s3mEmptyCommand ) {
				continue;
			}
			uint8_t fx = cell->effect();
			if( fx != s3mFxSpecial ) {
				continue;
			}
			uint8_t fxVal = cell->effectValue();
			if( highNibble( fxVal ) != s3mSFxPatDelay || lowNibble( fxVal ) == 0 ) {
				continue;
			}
			if( ++patDelayCounter != 1 || m_patDelayCount != -1 ) {
				continue;
			}
			m_patDelayCount = lowNibble( fxVal );
		}
		if( m_patDelayCount > 1 ) {
			m_patDelayCount--;
		}
		else {
			m_patDelayCount = -1;
		}
		// now check for breaking effects
		for( uint8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			if( m_patLoopCount != -1 ) {
				break;
			}
			const S3mCell* cell = currPat->cellAt( currTrack, state().row );
			if( !cell || cell->effect() == s3mEmptyCommand ) {
				continue;
			}
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if( fx == s3mFxJumpOrder ) {
				m_breakOrder = fxVal;
			}
			else if( fx == s3mFxBreakPat ) {
				m_breakRow = highNibble( fxVal ) * 10 + lowNibble( fxVal );
				logger()->debug( L4CXX_LOCATION, boost::format( "Row %d: Break pattern to row %d" ) % state().row % m_breakRow );
			}
		}
	}
	catch( ... ) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
}

bool S3mModule::adjustPosition( bool estimateOnly )
{
	BOOST_ASSERT( orderCount() != 0 );
	bool orderChanged = false;
	if( state().tick == 0 ) {
		m_patDelayCount = -1;
		if( m_breakOrder != 0xffff ) {
			if( m_breakOrder < orderCount() ) {
				setOrder( m_breakOrder, estimateOnly );
				orderChanged = true;
			}
			setRow( 0 );
		}
		if( m_breakRow != 0xffff ) {
			if( m_breakRow <= 63 ) {
				setRow( m_breakRow );
			}
			if( m_breakOrder == 0xffff ) {
				if( m_patLoopCount == -1 ) {
					setOrder( state().order + 1, estimateOnly );
					orderChanged = true;
				}
			}
		}
		if( ( m_breakRow == 0xffff ) && ( m_breakOrder == 0xffff ) && ( m_patDelayCount == -1 ) ) {
			setRow( ( state().row + 1 ) & 0x3f );
			if( state().row == 0 ) {
				setOrder( state().order + 1, estimateOnly );
				orderChanged = true;
			}
		}
		m_breakRow = m_breakOrder = ~0;
	}
	state().pattern = orderAt( state().order )->index();
	// skip "--" and "++" marks
	while( state().pattern >= 254 ) {
		if( state().pattern == s3mOrderEnd ) {
			return false;
		}
		setOrder( state().order + 1, estimateOnly );
		orderChanged = true;
		if( state().order >= orderCount() ) {
			logger()->info( L4CXX_LOCATION, "Song end reached: End of orders" );
			return false;
		}
		state().pattern = orderAt( state().order )->index();
	}
	if( orderChanged ) {
		m_patLoopRow = 0;
		m_patLoopCount = -1;
	}
	return true;
}

size_t S3mModule::internal_buildTick( AudioFrameBuffer* buf )
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
	S3mPattern* currPat = getPattern( state().pattern );
	if( !currPat ) {
		logger()->error(L4CXX_LOCATION, "Did not find a pattern for current order");
		if(buf) {
			buf->reset();
		}
		return 0;
	}
	if( buf ) {
		MixerFrameBuffer mixerBuffer( new MixerFrameBuffer::element_type( tickBufferLength() ) );
		for( uint_fast8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mChannel* chan = m_channels.at( currTrack );
			BOOST_ASSERT( chan != nullptr );
			const S3mCell* cell = currPat->cellAt( currTrack, state().row );
			chan->update( cell, m_patDelayCount != -1, false );
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
		for( uint_fast8_t currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mChannel* chan = m_channels.at( currTrack );
			BOOST_ASSERT( chan!=nullptr );
			const S3mCell* cell = currPat->cellAt( currTrack, state().row );
			chan->update( cell, m_patDelayCount != -1, true );
		}
	}
	state().playedFrames += tickBufferLength();
	nextTick();
	if( !adjustPosition(!buf) ) {
		logger()->info( L4CXX_LOCATION, "Song end reached: adjustPosition() failed" );
		if(buf) {
			buf->reset();
		}
		return 0;
	}
	return tickBufferLength();
}
catch( ... ) {
	BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
}

std::string S3mModule::internal_channelStatus( size_t idx ) const
{
	const S3mChannel* x = m_channels.at( idx );
	if( !x ) {
		return std::string();
	}
	return x->statusString();
}

std::string S3mModule::internal_channelCellString( size_t idx ) const
{
	const S3mChannel* x = m_channels.at( idx );
	if( !x ) {
		return std::string();
	}
	return x->cellString();
}

IArchive& S3mModule::serialize( IArchive* data )
{
	GenModule::serialize( data )
	% m_breakRow
	% m_breakOrder
	% m_patLoopRow
	% m_patLoopCount
	% m_patDelayCount
	% m_customData;
	for( S3mChannel*& chan : m_channels ) {
		if( !chan ) {
			continue;
		}
		data->archive( chan );
	}
	return *data;
}

GenModule::Ptr S3mModule::factory( const std::string& filename, uint32_t frequency, int maxRpt )
{
	S3mModule::Ptr result( new S3mModule( maxRpt ) );
	if( !result->load( filename ) ) {
		return GenModule::Ptr();
	}
	if( !result->initialize( frequency ) ) {
		return GenModule::Ptr();
	}
	return result;
}

bool S3mModule::hasZeroVolOpt() const
{
	return m_zeroVolOpt;
}

const S3mSample* S3mModule::sampleAt( size_t idx ) const
{
	return m_samples.at(idx);
}

size_t S3mModule::numSamples() const
{
	return m_samples.size();
}

bool S3mModule::st2Vibrato() const
{
	return m_st2Vibrato;
}

bool S3mModule::hasFastVolSlides() const
{
	return m_fastVolSlides;
}

bool S3mModule::hasAmigaLimits() const
{
	return m_amigaLimits;
}

S3mPattern* S3mModule::getPattern( size_t idx ) const
{
	if( idx >= m_patterns.size() ) {
		return nullptr;
	}
	return m_patterns.at( idx );
}

light4cxx::Logger::Ptr S3mModule::logger()
{
	return light4cxx::Logger::get( GenModule::logger()->name() + ".s3m" );
}

}
}

/**
 * @}
 */
