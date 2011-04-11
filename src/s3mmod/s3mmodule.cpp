/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#include "s3mmodule.h"

#include <iostream>

using namespace ppp;
using namespace ppp::s3m;

/**
* @file
* @ingroup S3mMod
* @brief Module definitions for ScreamTracker 3 Modules
*/

/**
* @brief Flags for s3mModuleHeader::flags
* @ingroup S3mMod
*/
enum : uint8_t {
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

/**
* @brief Parapointer, 16-byte aligned
* @ingroup S3mMod
*/
typedef uint16_t ParaPointer;

/**
* @brief Postprocess a sample value
* @param[in] sample The sample to be postprocessed
* @return The postprocessed sample
* @ingroup S3mMod
* @details
* The Future Crew states the following about Post processing:
@verbatim
How ST3 mixes:
	1) volumetable is created in the following way:
		> volumetable[volume][sampledata]=volume*(sampledata-128)/64;
		NOTE: sampledata in memory is unsigned in ST3, so the -128 in the
			formula converts it so that the volumetable output is signed.

	2) postprocessing table is created with this pseudocode:

		> z=mastervol&127;
		> if(z<0x10) z=0x10;
		> c=2048*16/z;
		> a=(2048-c)/2;
		> b=a+c;
		>                     { 0                , if x < a
		> posttable[x+1024] = { (x-a)*256/(b-a)  , if a <= x < b
		>                     { 255              , if x > b

	3) mixing the samples

		output=1024
		for i=0 to number of channels
			output+=volumetable[volume*globalvolume/64][sampledata];
		next
		realoutput=posttable[output]

	This is how the mixing is done in theory. In practice it's a bit
	different for speed reasons, but the result is the same.
@endverbatim
* OK, the last sentence is interesting... My first implementation used only a slightly modified
* version of that algorithm for signed samples. Very, very slow. So I took a closer look at this
* algorithm. Let's write it down to mathematical equations:
\f{eqnarray*}
	c & = & \frac{2^{11}\cdot2^{4}}{z}=2^{15}z^{-1} \\
	a & = & 2^{-1}\left(2^{11}-c\right)=2^{10}-2^{14}z^{-1} \\
	b & = & a+c=2^{10}-2^{14}z^{-1}+2^{15}z^{-1} \\
	& = & 2^{10}+2^{14}z^{-1} \\
	s_{o} & = & \begin{cases}
		0 & \text{if $s_{i} < a$} \\
		2^{8} \frac{s_{i}-a}{b-a} & \text{if $a \leq s_{i} < b$} \\
		255 & \text{if $b \leq s_{i}$}
	\end{cases} \\
	b-a & = & 2^{10}+2^{14}z^{-1} - 2^{10}+2^{14}z^{-1} \\
		& = & 2^{15}z^{-1} \\
		& = & c
\f}
Let's look at the second case:
\f{eqnarray*}
	2^{8} \frac{s_{i}-a}{c} & = & 2^{8} 2^{-15} z \left( s_{i} - \left(2^{10}-2^{14}z^{-1}\right) \right) \\
		& = & 2^{-7} z \left( s_{i} - 2^{10} + 2^{14}z^{-1} \right) \\
		& = & \frac{\left(s_{i}-2^{10}\right)z}{2^{7}} + 2^{7}
\f}
Hell, yes... You see? Keeping in mind that @f$ s_{i} @f$ is @f$ 2^{10} @f$-based and @f$ s_{o} @f$ is @f$ 2^{7} @f$-based, that's @b @e really simple
*/
inline int16_t s3mPostProcess( int32_t sample ) throw() {
	return clip( sample >> 2, -32768, 32767 );
}

#pragma pack(push,1)
/**
 * @typedef S3mModuleHeader
 * @ingroup S3mMod
 * @brief S3M Module Header
 */
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

S3mModule::S3mModule( uint32_t frq, uint8_t maxRpt ) throw( PppException ) : GenModule( frq, maxRpt ),
	m_breakRow( -1 ), m_breakOrder( -1 ), m_patLoopRow( -1 ), m_patLoopCount( -1 ), m_patDelayCount( -1 ),
	m_customData( false ), m_samples(), m_patterns(), m_channels(), m_usedChannels( 0 ), m_orderPlaybackCounts(),
	m_amigaLimits(false), m_fastVolSlides(false), m_st2Vibrato(false), m_zeroVolOpt(false)
{
	try {
		for( uint16_t i = 0; i < 256; i++ ) {
			addOrder( GenOrder::Ptr( new GenOrder( s3mOrderEnd ) ) );
			m_patterns.push_back( S3mPattern::Ptr() );
			m_samples.push_back( S3mSample::Ptr() );
		}
		for( uint8_t i = 0; i < m_channels.size(); i++ ) {
			m_channels[i].reset( new S3mChannel( getPlaybackFrq(), this ) );
		}
		for( std::size_t i = 0; i < m_orderPlaybackCounts.size(); i++ )
			m_orderPlaybackCounts[i] = 0;
	}
	PPP_CATCH_ALL();
}

S3mModule::~S3mModule() throw() {
}

bool S3mModule::load( const std::string& fn ) throw( PppException ) {
	try {
		LOG_MESSAGE( "Opening '%s'", fn.c_str() );
		FBinStream str( fn );
		if( !str.isOpen() ) {
			LOG_WARNING( "%s could not be opened", fn.c_str() );
			return false;
		}
		setFilename( fn );
		S3mModuleHeader s3mHdr;
		try {
			str.seek( 0 );
			str.read( reinterpret_cast<char*>( &s3mHdr ), sizeof( s3mHdr ) );
		}
		catch( ... ) {
			PPP_THROW( "Header could not be read" );
		}
		if( str.fail() ) {  // header read completely?
			LOG_WARNING( "Header Error" );
			return false;
		}
		if( !std::equal( s3mHdr.id, s3mHdr.id + 4, "SCRM" ) ) {
			LOG_WARNING( "Header ID Error" );
			return false;
		}
		s3mHdr.ordNum &= 0xff;
		s3mHdr.patNum &= 0xff;
		s3mHdr.smpNum &= 0xff;
		// many modules have 0x00 here, but they_re still correct
		LOG_TEST_WARN( s3mHdr.type[0] != 0x1a );
		if( s3mHdr.createdWith == 0x1300 )
			s3mHdr.flags |= s3mFlag300Slides;
		m_st2Vibrato = (s3mHdr.flags & s3mFlagSt2Vibrato)!=0;
		if( m_st2Vibrato ) LOG_WARNING( "ST2 Vibrato (not supported)" );
		if( s3mHdr.flags & s3mFlagSt2Tempo ) LOG_MESSAGE( "ST2 Tempo (not supported)" );
		m_fastVolSlides = (s3mHdr.flags & s3mFlag300Slides)!=0;
		if( m_fastVolSlides ) LOG_MESSAGE( "ST v3.00 Volume Slides" );
		m_zeroVolOpt = (s3mHdr.flags & s3mFlag0volOpt)!=0;
		if( m_zeroVolOpt ) LOG_MESSAGE( "Zero-volume Optimization" );
		if( s3mHdr.flags & s3mFlagAmigaSlides ) LOG_WARNING( "Amiga slides (not supported)" );
		m_amigaLimits = (s3mHdr.flags & s3mFlagAmigaLimits)!=0;
		if( m_amigaLimits ) LOG_MESSAGE( "Amiga limits" );
		if( s3mHdr.flags & s3mFlagSpecial ) LOG_MESSAGE( "Special data present" );
		if( s3mHdr.defaultPannings == 0xFC ) LOG_MESSAGE( "Default Pannings present" );
		unsigned char schismTest = 0;
		switch( ( s3mHdr.createdWith >> 12 ) & 0x0f ) {
			case s3mTIdScreamTracker:
				setTrackerInfo( "ScreamTracker v" );
				break;
			case s3mTIdImagoOrpheus:
				setTrackerInfo( "Imago Orpheus v" );
				break;
			case s3mTIdImpulseTracker:
				setTrackerInfo( "Impulse Tracker v" );
				break;
				// the following IDs were found in the Schism Tracker sources
			case s3mTIdSchismTracker:
				setTrackerInfo( "Schism Tracker v" );
				schismTest = 1;
				break;  // some versions of Schism Tracker use ID 1
			case s3mTIdOpenMPT:
				setTrackerInfo( "OpenMPT v" );
				break;
			default:
				setTrackerInfo( stringf( "Unknown Tracker (%x) v", s3mHdr.createdWith >> 12 ) );
		}
		setTrackerInfo( getTrackerInfo() + stringf( "%x.%.2x", ( s3mHdr.createdWith >> 8 ) & 0x0f, s3mHdr.createdWith & 0xff ) );
		setTempo( s3mHdr.initialTempo );
		//m_playbackInfo.speed = s3mHdr.initialSpeed;
		setSpeed( s3mHdr.initialSpeed );
		PPP_TEST( getPlaybackInfo().speed == 0 );
		setGlobalVolume( s3mHdr.globalVolume );
		// parse flags
		m_customData = ( s3mHdr.ffv & s3mFlagSpecial ) != 0;
		// now read the orders...
		str.seek( 0x60 );
		for( int i = 0; i < s3mHdr.ordNum; i++ ) {
			if( !str.good() )
				PPP_THROW( "Stream Error: Orders" );
			unsigned char tmpOrd;
			str.read( &tmpOrd );
			getOrder( i )->setIndex( tmpOrd );
		}
		unsigned char defPans[32];
		if( s3mHdr.defaultPannings == 0xFC ) {
			try {
				str.seek( 0x60 + s3mHdr.ordNum + 2 * s3mHdr.smpNum + 2 * s3mHdr.patNum );
				str.read( defPans, 32 );
			}
			catch( ... ) {
				PPP_THROW( "Stream Error: Default Pannings" );
			}
			for( int i = 0; i < 32; i++ ) {
				schismTest |= ( defPans[i] & 0x10 ) != 0;
			}
		}
		// load the samples
		LOG_MESSAGE( "Loading samples..." );
		for( int i = 0; i < s3mHdr.smpNum; i++ ) {
			if( !str.good() )
				PPP_THROW( "Stream Error: Samples" );
			str.seek( s3mHdr.ordNum + 0x60 + 2 * i );
			if( !str.good() ) {
				PPP_THROW( "Stream Operation: Fail" );
			}
			ParaPointer pp;
			str.read( &pp );
			if( pp == 0 )
				continue;
			m_samples[i].reset( new S3mSample() );
			if( !( m_samples[i]->load( str, pp * 16, (( s3mHdr.createdWith >> 12 ) & 0x0f) == s3mTIdImagoOrpheus ) ) )
				PPP_THROW( "Sample Error" );
			if( !str.good() )
				PPP_THROW( "Stream Error: Samples / Load" );
			schismTest |= ( m_samples[i]->isHighQuality() || m_samples[i]->isStereo() );
		}
		if( schismTest != 0 ) {
			LOG_MESSAGE( "Enabling Schism Tracker compatibility mode" );
		}
		// ok, samples loaded, now load the patterns...
		LOG_MESSAGE( "Loading patterns..." );
		for( int i = 0; i < s3mHdr.patNum; i++ ) {
			str.seek( s3mHdr.ordNum + 2 * s3mHdr.smpNum + 0x60 + 2 * i );
			ParaPointer pp;
			str.read( &pp );
			if( pp == 0 )
				continue;
			S3mPattern* pat = new S3mPattern();
			m_patterns[i].reset( pat );
			if( !( pat->load( str, pp * 16 ) ) )
				PPP_THROW( "Pattern Error" );
			if( !str.good() )
				PPP_THROW( "Stream Error: Patterns" );
		}
		//str.close();
		// set pannings...
		LOG_MESSAGE( "Preparing channels..." );
		for( int i = 0; i < 32; i++ ) {
			S3mChannel* s3mChan = new S3mChannel( getPlaybackFrq(), this );
			m_channels[i].reset( s3mChan );
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
						s3mChan->setPanning( 0x03 * 0x80 / 0x0f );
					else // right channel
						s3mChan->setPanning( 0x0c * 0x80 / 0x0f );
				}
				else { // mono
					s3mChan->setPanning( 0x40 );
				}
			}
			else { // panning settings are there...
				if( ( defPans[i] & 0x20 ) == 0 ) { // use defaults
					if( ( s3mHdr.masterVolume & 0x80 ) != 0 ) { // stereo
						if( ( s3mHdr.pannings[i] & 0x08 ) != 0 ) // left channel
							s3mChan->setPanning( 0x03 * 0x80 / 0x0f );
						else // right channel
							s3mChan->setPanning( 0x0c * 0x80 / 0x0f );
					}
					else { // mono
						s3mChan->setPanning( 0x40 );
					}
				}
				else { // use panning settings...
					s3mChan->setPanning( ( defPans[i] & 0x0f ) * 0x80 / 0x0f );
				}
			}
		}
		setTitle( stringncpy( s3mHdr.title, 28 ) );
		getMultiTrack( getCurrentTrack() ).newState()->archive( this ).finishSave();
		// calculate total length...
		LOG_MESSAGE( "Calculating track lengths and preparing seek operations..." );
		do {
			LOG_MESSAGE( "Pre-processing Track %d", getCurrentTrack() );
			std::size_t currTickLen = 0;
			getMultiTrack( getCurrentTrack() ).startOrder = getPlaybackInfo().order;
			do {
				getTickNoMixing( currTickLen );
				getMultiTrack( getCurrentTrack() ).length += currTickLen;
			}
			while( currTickLen != 0 );
			LOG_MESSAGE( "Preprocessed." );
			int nCount = 0;
			for( int i = 0; i < getOrderCount(); i++ ) {
				PPP_TEST( !getOrder( i ) );
				if( ( getOrder( i )->getIndex() != s3mOrderEnd ) && ( getOrder( i )->getIndex() != s3mOrderSkip ) && ( m_orderPlaybackCounts[i] == 0 ) ) {
					if( nCount == 0 )
						setMultiTrack( true );
					nCount++;
				}
			}
			LOG_MESSAGE( "Trying to jump to the next track" );
		}
		while( jumpNextTrack() );
		LOG_MESSAGE( "Lengths calculated, resetting module." );
		if( getTrackCount() > 0 )
			getMultiTrack( 0 ).nextState()->archive( this ).finishLoad();
		LOG_MESSAGE( "Removing empty tracks" );
		removeEmptyTracks();
//		if (aMultiTrack)
//			LOG_MESSAGE("Hmmm... This could be a multi-song module, there are never played orders");
		return true;
	}
	PPP_CATCH_ALL();
}

bool S3mModule::existsSample( int16_t idx ) throw() {
	idx--;
	if( !inRange<int>( idx, 0, m_samples.size() - 1 ) )
		return false;
	return m_samples[idx].get() != NULL;
}

std::string S3mModule::getSampleName( int16_t idx ) throw() {
	if( !existsSample( idx ) )
		return std::string();
	return m_samples[idx - 1]->getTitle();
}

uint8_t S3mModule::channelCount() const {
	return m_usedChannels;
}


void S3mModule::checkGlobalFx() throw( PppException ) {
	try {
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		S3mPattern::Ptr currPat = getPattern( getPlaybackInfo().pattern );
		if( !currPat )
			return;
		std::string data;
		for( unsigned int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mCell::Ptr cell = currPat->getCell( currTrack, getPlaybackInfo().row );
			if( !cell )
				continue;
			if( !cell->isActive() )
				continue;
			if( cell->getEffect() == s3mEmptyCommand )
				continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if( ( fx == s3mFxSpeed ) && ( fxVal != 0 ) )
				setSpeed( fxVal );
			else if( ( fx == s3mFxTempo ) && ( fxVal > 0x20 ) )
				setTempo( fxVal );
			else if( fx == s3mFxGlobalVol ) {
				if( fxVal <= 0x40 )
					setGlobalVolume( fxVal );
			}
		}
		// check for pattern loops
		int patLoopCounter = 0;
		for( unsigned int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mCell::Ptr cell = currPat->getCell( currTrack, getPlaybackInfo().row );
			if( !cell ) continue;
			if( !cell->isActive() ) continue;
			if( cell->getEffect() == s3mEmptyCommand ) continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if( fx != s3mFxSpecial ) continue;
			if( highNibble( fxVal ) != s3mSFxPatLoop ) continue;
			if( lowNibble( fxVal ) == 0x00 ) {  // loop start
				m_patLoopRow = getPlaybackInfo().row;
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
						m_patLoopRow = getPlaybackInfo().row + 1;
					}
					else { // we got an "infinite" loop...
						m_patLoopCount = 127;
						m_breakRow = m_patLoopRow;
						LOG_WARNING( "Infinite pattern loop detected" );
					}
				}
			}
		}
		// check for pattern delays
		int patDelayCounter = 0;
		for( unsigned int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mCell::Ptr cell = currPat->getCell( currTrack, getPlaybackInfo().row );
			if( !cell ) continue;
			if( !cell->isActive() ) continue;
			if( cell->getEffect() == s3mEmptyCommand ) continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if( fx != s3mFxSpecial ) continue;
			if( highNibble( fxVal ) != s3mSFxPatDelay ) continue;
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
		for( unsigned int currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			if( m_patLoopCount != -1 ) break;
			S3mCell::Ptr cell = currPat->getCell( currTrack, getPlaybackInfo().row );
			if( !cell ) continue;
			if( !cell->isActive() ) continue;
			if( cell->getEffect() == s3mEmptyCommand ) continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if( fx == s3mFxJumpOrder ) {
				m_breakOrder = fxVal;
			}
			else if( fx == s3mFxBreakPat ) {
				m_breakRow = highNibble( fxVal ) * 10 + lowNibble( fxVal );
				LOG_MESSAGE( "Row %d: Break pattern to row %d", getPlaybackInfo().row, m_breakRow );
			}
		}
	}
	PPP_RETHROW()
	catch( ... ) {
		PPP_THROW( "Unknown Exception" );
	}
}

bool S3mModule::adjustPosition( bool increaseTick, bool doStore ) throw( PppException ) {
	PPP_TEST( getOrderCount() == 0 );
	bool orderChanged = false;
	if( increaseTick ) {
		nextTick();
		/*		m_playbackInfo.tick++;
				PPP_TEST( getPlaybackInfo().speed == 0 );
				m_playbackInfo.tick %= getPlaybackInfo().speed;*/
	}
	if( ( getPlaybackInfo().tick == 0 ) && increaseTick ) {
		m_patDelayCount = -1;
		if( m_breakOrder != -1 ) {
			m_orderPlaybackCounts[getPlaybackInfo().order]++;
			if( m_breakOrder < getOrderCount() ) {
				setOrder( m_breakOrder );
				orderChanged = true;
			}
			setRow( 0 );
		}
		if( m_breakRow != -1 ) {
			if( m_breakRow <= 63 ) {
				setRow( m_breakRow );
			}
			if( m_breakOrder == -1 ) {
				if( m_patLoopCount == -1 ) {
					m_orderPlaybackCounts[getPlaybackInfo().order]++;
					setOrder( getPlaybackInfo().order + 1 );
					orderChanged = true;
				}
				//else {
				//	LOG_MESSAGE(stringf("oO... aPatLoopCount=%d",aPatLoopCount));
				//}
			}
		}
		if( ( m_breakRow == -1 ) && ( m_breakOrder == -1 ) && ( m_patDelayCount == -1 ) ) {
			setRow( ( getPlaybackInfo().row + 1 ) & 0x3f );
			if( getPlaybackInfo().row == 0 ) {
				m_orderPlaybackCounts[getPlaybackInfo().order]++;
				setOrder( getPlaybackInfo().order + 1 );
				orderChanged = true;
			}
		}
		m_breakRow = m_breakOrder = -1;
	}
	setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
	// skip "--" and "++" marks
	while( getPlaybackInfo().pattern >= 254 ) {
		if( getPlaybackInfo().pattern == s3mOrderEnd ) {
			LOG_TEST_MESSAGE( getPlaybackInfo().pattern == s3mOrderEnd );
			return false;
		}
		if( !mapOrder( getPlaybackInfo().order ) )
			return false;
		m_orderPlaybackCounts[getPlaybackInfo().order]++;
		setOrder( getPlaybackInfo().order + 1 );
		orderChanged = true;
		if( getPlaybackInfo().order >= getOrderCount() ) {
			LOG_MESSAGE( "Song end reached: End of orders" );
			return false;
		}
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
	}
	if( orderChanged ) {
		m_patLoopRow = 0;
		m_patLoopCount = -1;
		try {
			if( doStore )
				getMultiTrack( getCurrentTrack() ).newState()->archive( this ).finishSave();
			else {
				//PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
				getMultiTrack( getCurrentTrack() ).nextState()->archive( this ).finishLoad();
			}
		}
		PPP_CATCH_ALL()
	}
	return true;
}

void S3mModule::getTick( AudioFrameBuffer& buf ) throw( PppException ) {
	try {
		//PPP_TEST(!buf);
		if( !buf )
			buf.reset( new AudioFrameBuffer::element_type );
		if( getPlaybackInfo().tick == 0 )
			checkGlobalFx();
		//buf->resize(getTickBufLen());
		//buf->clear();
		if( !adjustPosition( false, false ) ) {
			LOG_MESSAGE( "Song end reached: adjustPosition() failed" );
			buf.reset();
			return;
		}
		if( m_orderPlaybackCounts[getPlaybackInfo().order] >= getMaxRepeat() ) {
			LOG_MESSAGE( "Song end reached: Maximum repeat count reached" );
			buf.reset();
			return;
		}
		// update channels...
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		S3mPattern::Ptr currPat = getPattern( getPlaybackInfo().pattern );
		if( !currPat )
			return;
		MixerFrameBuffer mixerBuffer( new MixerFrameBuffer::element_type( getTickBufLen(), {0, 0} ) );
		for( unsigned short currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mChannel::Ptr chan = m_channels[currTrack];
			PPP_TEST( !chan );
			S3mCell::Ptr cell = currPat->getCell( currTrack, getPlaybackInfo().row );
			chan->update( cell, m_patDelayCount != -1 );
			chan->mixTick( mixerBuffer );
		}
		buf->resize( mixerBuffer->size() );
		MixerSample* mixerBufferPtr = &mixerBuffer->front().left;
		BasicSample* bufPtr = &buf->front().left;
		for( std::size_t i = 0; i < mixerBuffer->size(); i++ ) {  // postprocess...
			*( bufPtr++ ) = clipSample( *( mixerBufferPtr++ ) >> 2 );
			*( bufPtr++ ) = clipSample( *( mixerBufferPtr++ ) >> 2 );
		}
		adjustPosition( true, false );
		setPosition( getPosition() + mixerBuffer->size() );
	}
	PPP_CATCH_ALL();
// 	PPP_RETHROW()
}

void S3mModule::getTickNoMixing( std::size_t& bufLen ) throw( PppException ) {
	try {
		if( getPlaybackInfo().tick == 0 )
			checkGlobalFx();
		bufLen = 0;
		if( !adjustPosition( false, true ) )
			return;
		PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
		if( m_orderPlaybackCounts[getPlaybackInfo().order] >= getMaxRepeat() )
			return;
		// update channels...
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		S3mPattern::Ptr currPat = getPattern( getPlaybackInfo().pattern );
		if( !currPat )
			return;
		bufLen = getTickBufLen(); // in frames
		for( unsigned short currTrack = 0; currTrack < channelCount(); currTrack++ ) {
			S3mChannel::Ptr chan = m_channels[currTrack];
			PPP_TEST( !chan );
			S3mCell::Ptr cell = currPat->getCell( currTrack, getPlaybackInfo().row );
			chan->update( cell, m_patDelayCount != -1 );
			chan->simTick( bufLen );
		}
		adjustPosition( true, true );
		setPosition( getPosition() + bufLen );
	}
	PPP_CATCH_ALL();
}

bool S3mModule::jumpNextOrder() throw() {
	IArchive* next = getMultiTrack( getCurrentTrack() ).nextState();
	if( next == NULL )
		return false;
	next->archive( this ).finishLoad();
	return true;
}

bool S3mModule::jumpPrevOrder() throw() {
	IArchive* next = getMultiTrack( getCurrentTrack() ).prevState();
	if( next == NULL )
		return false;
	next->archive( this ).finishLoad();
	return true;
}

GenOrder::Ptr S3mModule::mapOrder( int16_t order ) throw() {
	static GenOrder::Ptr xxx( new GenOrder( s3mOrderEnd ) );
	if( !inRange<int16_t>( order, 0, getOrderCount() - 1 ) )
		return xxx;
	return getOrder( order );
}

std::string S3mModule::getChanStatus( int16_t idx ) throw() {
	S3mChannel::Ptr x = m_channels[idx];
	if( !x )
		return "";
	return x->getStatus();
}

std::string S3mModule::getChanCellString( int16_t idx ) throw() {
	S3mChannel::Ptr x = m_channels[idx];
	if( !x )
		return "";
	return x->getCellString();
}

bool S3mModule::jumpNextTrack() throw( PppException ) {
	if( !isMultiTrack() ) {
		LOG_MESSAGE( "This is not a multi-track" );
		return false;
	}
	PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
	m_orderPlaybackCounts[getPlaybackInfo().order]++;
	setCurrentTrack( getCurrentTrack() + 1 );
	if( getCurrentTrack() >= getTrackCount() ) {
		GenMultiTrack nulltrack;
		for( uint16_t i = 0; i < getOrderCount(); i++ ) {
			PPP_TEST( !getOrder( i ) );
			if( ( getOrder( i )->getIndex() != s3mOrderEnd ) && ( getOrder( i )->getIndex() != s3mOrderSkip ) && ( m_orderPlaybackCounts[i] == 0 ) ) {
				PPP_TEST( !mapOrder( i ) );
				setPatternIndex( mapOrder( i )->getIndex() );
				setOrder( i );
				nulltrack.startOrder = i;
				addMultiTrack( nulltrack );
				setPosition( 0 );
				getMultiTrack( getCurrentTrack() ).newState()->archive( this ).finishSave();
				return true;
			}
		}
		nulltrack.startOrder = GenMultiTrack::stopHere;
		addMultiTrack( nulltrack );
		return false;
	}
	else {
		if( getMultiTrack( getCurrentTrack() ).startOrder == GenMultiTrack::stopHere ) {
			LOG_MESSAGE( "No more tracks" );
			return false;
		}
		setOrder( getMultiTrack( getCurrentTrack() ).startOrder );
		PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		setPosition( 0 );
		getMultiTrack( getCurrentTrack() ).nextState()->archive( this ).finishLoad();
		//restoreState( getPlaybackInfo().order, 0 );
		return true;
	}
	LOG_ERROR( "This should definitively NOT have happened..." );
	return false;
}

bool S3mModule::jumpPrevTrack() throw( PppException ) {
	if( !isMultiTrack() ) {
		LOG_MESSAGE( "This is not a multi-track" );
		return false;
	}
	if( getCurrentTrack() == 0 ) {
		LOG_MESSAGE( "Already on first track" );
		return false;
	}
	setCurrentTrack( getCurrentTrack() - 1 );
	setOrder( getMultiTrack( getCurrentTrack() ).startOrder );
	PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
	setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
	setPosition( 0 );
	getMultiTrack( getCurrentTrack() ).nextState()->archive( this ).finishLoad();
	//restoreState( getPlaybackInfo().order, 0 );
	return true;
}

IArchive& S3mModule::serialize( IArchive* data ) {
	GenModule::serialize( data )
	& m_breakRow& m_breakOrder& m_patLoopRow& m_patLoopCount
	& m_patDelayCount& m_customData;
	for( std::size_t i = 0; i < m_channels.size(); i++ ) {
		if( !m_channels[i] )
			continue;
		data->archive( m_channels[i].get() );
	}
	data->array( &m_orderPlaybackCounts.front(), m_orderPlaybackCounts.size() );
	return *data;
}

void S3mModule::setGlobalVolume(int16_t v)
{
	GenModule::setGlobalVolume(v);
	for(std::size_t i=0; i<m_channels.size(); i++)
		m_channels[i]->recalcVolume();
}

uint16_t S3mModule::getTickBufLen() const throw( PppException ) {
	PPP_TEST( getPlaybackInfo().tempo < 0x20 );
	return getPlaybackFrq() * 5 / ( getPlaybackInfo().tempo << 1 );
}
