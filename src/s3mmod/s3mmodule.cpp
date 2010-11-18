/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
} __attribute__((packed));

S3mModule::S3mModule( const uint32_t frq, const uint8_t maxRpt ) throw( PppException ) : GenModule( frq, maxRpt ),
		m_breakRow( -1 ), m_breakOrder( -1 ), m_patLoopRow( -1 ), m_patLoopCount( -1 ), m_patDelayCount( -1 ),
		m_customData( false ) {
	try {
		for ( uint16_t i = 0; i < 256; i++ ) {
			addOrder( GenOrder::Ptr( new GenOrder( s3mOrderEnd ) ) );
			addPattern();
			addSample( S3mSample::Ptr( static_cast<S3mSample*>( 0 ) ) );
		}
		for ( uint8_t i = 0; i < 32; i++ ) {
			addChannel( S3mChannel::Ptr( new S3mChannel( getPlaybackFrq(), getSamples() ) ) );
		}
	}
	PPP_CATCH_ALL();
}

S3mModule::~S3mModule() throw() {
}

bool S3mModule::load( const std::string &fn ) throw( PppException ) {
	try {
		LOG_BEGIN();
		LOG_MESSAGE( "Opening '%s'", fn.c_str() );
		FBinStream str( fn );
		if(!str.isOpen()) {
			LOG_WARNING("%s could not be opened", fn.c_str());
			return false;
		}
		setFilename(fn);
		S3mModuleHeader s3mHdr;
		try {
			str.seek( 0 );
			str.read( reinterpret_cast<char*>(&s3mHdr), sizeof(s3mHdr) );
		}
		catch ( ... ) {
			PPP_THROW( "Header could not be read" );
		}
		if ( str.fail() ) { // header read completely?
			LOG_WARNING_( "Header Error" );
			return false;
		}
		if(!std::equal(s3mHdr.id, s3mHdr.id+4, "SCRM")) {
			LOG_WARNING_( "Header ID Error" );
			return false;
		}
		s3mHdr.ordNum &= 0xff;
		s3mHdr.patNum &= 0xff;
		s3mHdr.smpNum &= 0xff;
		// many modules have 0x00 here, but they_re still correct
		LOG_TEST_WARN( s3mHdr.type[0] != 0x1a );
		if ( s3mHdr.createdWith == 0x1300 )
			s3mHdr.flags |= s3mFlag300Slides;
		if ( s3mHdr.flags&s3mFlagSt2Vibrato ) LOG_WARNING_( "ST2 Vibrato (not supported)" );
		if ( s3mHdr.flags&s3mFlagSt2Tempo ) LOG_MESSAGE_( "ST2 Tempo (not supported)" );
		if ( s3mHdr.flags&s3mFlag300Slides ) LOG_MESSAGE_( "ST v3.00 Volume Slides" );
		if ( s3mHdr.flags&s3mFlag0volOpt ) LOG_MESSAGE_( "Zero-volume Optimization" );
		if ( s3mHdr.flags&s3mFlagAmigaSlides ) LOG_WARNING_( "Amiga slides (not supported)" );
		if ( s3mHdr.flags&s3mFlagAmigaLimits ) LOG_MESSAGE_( "Amiga limits" );
		if ( s3mHdr.flags&s3mFlagSpecial ) LOG_MESSAGE_( "Special data present" );
		if ( s3mHdr.defaultPannings == 0xFC ) LOG_MESSAGE_( "Default Pannings present" );
		switch (( s3mHdr.createdWith >> 12 )&0x0f ) {
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
		setSpeed(s3mHdr.initialSpeed);
		PPP_TEST( getPlaybackInfo().speed == 0 );
		setGlobalVolume( s3mHdr.globalVolume );
		// parse flags
		m_customData = ( s3mHdr.ffv & s3mFlagSpecial ) != 0;
		// now read the orders...
		str.seek( 0x60 );
		for ( int i = 0; i < s3mHdr.ordNum; i++ ) {
			if ( !str.good() )
				PPP_THROW( "Stream Error: Orders" );
			unsigned char tmpOrd;
			str.read( &tmpOrd );
			getOrder(i)->setIndex( tmpOrd );
		}
		unsigned char defPans[32];
		unsigned char schismTest = 0;
		if ( s3mHdr.defaultPannings == 0xFC ) {
			try {
				str.seek( 0x60 + s3mHdr.ordNum + 2*s3mHdr.smpNum + 2*s3mHdr.patNum );
				str.read( defPans, 32 );
			}
			catch ( ... ) {
				PPP_THROW( "Stream Error: Default Pannings" );
			}
			for ( int i = 0; i < 32; i++ ) {
				schismTest |= ( defPans[i] & 0x10 ) != 0;
			}
		}
		// load the samples
		LOG_MESSAGE_( "Loading samples..." );
		for ( int i = 0; i < s3mHdr.smpNum; i++ ) {
			if ( !str.good() )
				PPP_THROW( "Stream Error: Samples" );
			str.seek( s3mHdr.ordNum + 0x60 + 2*i );
			if ( !str.good() ) {
				PPP_THROW( "Stream Operation: Fail" );
			}
			ParaPointer pp;
			str.read( &pp );
			resetSample( i, new S3mSample() );
			if ( !(getSample(i)->load( str, pp*16 ) ) )
				PPP_THROW( "Sample Error" );
			if ( !str.good() )
				PPP_THROW( "Stream Error: Samples / Load" );
			schismTest |= (getSample(i)->isHighQuality() || getSample(i)->isStereo() );
		}
		if (( schismTest != 0 ) && ( s3mHdr.createdWith == 0x1320 ) ) {
			LOG_MESSAGE_( "Enabling Schism Tracker compatibility mode" );
		}
		// ok, samples loaded, now load the patterns...
		LOG_MESSAGE_( "Loading patterns..." );
		for ( int i = 0; i < s3mHdr.patNum; i++ ) {
			str.seek( s3mHdr.ordNum + 2*s3mHdr.smpNum + 0x60 + 2*i );
			ParaPointer pp;
			str.read( &pp );
			S3mPattern* pat = new S3mPattern();
			resetPattern(i,pat);
			if ( !( pat->load( str, pp*16 ) ) )
				PPP_THROW( "Pattern Error" );
			if ( !str.good() )
				PPP_THROW( "Stream Error: Patterns" );
		}
		//str.close();
		// set pannings...
		LOG_MESSAGE_( "Preparing channels..." );
		int chanMapPos = 0;
		setMappedChannelCount(0);
		for ( int i = 0; i < 32; i++ ) {
			S3mChannel *s3mChan = new S3mChannel( getPlaybackFrq(), getSamples() );
			resetChannel( i, s3mChan );
			s3mChan->setGlobalVolume( s3mHdr.globalVolume, true );
			if (( s3mHdr.flags&s3mFlagAmigaLimits ) != 0 )
				s3mChan->enableAmigaLimits();
			if (( s3mHdr.flags&s3mFlag300Slides ) != 0 )
				s3mChan->enable300VolSlides();
			if (( s3mHdr.flags&s3mFlag0volOpt ) != 0 )
				s3mChan->enableZeroVol();
			if ((( s3mHdr.createdWith >> 12 )&0x0f ) != s3mTIdScreamTracker )
				s3mChan->disableGlobalVolDelay();
			if (( schismTest != 0 ) && ( s3mHdr.createdWith == 0x1320 ) )
				s3mChan->maybeSchism();
			if (( s3mHdr.pannings[i]&0x80 ) != 0 ) {
				getChannel(i)->disable();
				continue;
			}
			else {
				getChannel(i)->enable();
				m_channelMappings[chanMapPos++] = i;
				setMappedChannelCount(getMappedChannelCount()+1);
			}
			if ( s3mHdr.defaultPannings != 0xFC ) { // no pannings
				if (( s3mHdr.masterVolume&0x80 ) != 0 ) { // stereo
					if (( s3mHdr.pannings[i]&0x08 ) != 0 ) // left channel
						getChannel(i)->setPanning( 0x03*0x80 / 0x0f );
					else // right channel
						getChannel(i)->setPanning( 0x0c*0x80 / 0x0f );
				}
				else { // mono
					getChannel(i)->setPanning( 0x40 );
				}
			}
			else { // panning settings are there...
				if (( defPans[i]&0x20 ) == 0 ) { // use defaults
					if (( s3mHdr.masterVolume&0x80 ) != 0 ) { // stereo
						if (( s3mHdr.pannings[i]&0x08 ) != 0 ) // left channel
							getChannel(i)->setPanning( 0x03*0x80 / 0x0f );
						else // right channel
							getChannel(i)->setPanning( 0x0c*0x80 / 0x0f );
					}
					else { // mono
						getChannel(i)->setPanning( 0x40 );
					}
				}
				else { // use panning settings...
					getChannel(i)->setPanning(( defPans[i]&0x0f )*0x80 / 0x0f );
				}
			}
		}
		setTitle( stringncpy( s3mHdr.title, 28 ) );
		saveState();
		// calculate total length...
		LOG_MESSAGE_( "Calculating track lengths and preparing seek operations..." );
		do {
			LOG_MESSAGE( "Pre-processing Track %d", getCurrentTrack() );
			std::size_t currTickLen = 0;
			getMultiTrack(getCurrentTrack()).startOrder = getPlaybackInfo().order;
			do {
				getTickNoMixing( currTickLen );
				getMultiTrack(getCurrentTrack()).length += currTickLen;
			} while ( currTickLen != 0 );
			LOG_MESSAGE_( "Preprocessed." );
			int nCount = 0;
			for ( unsigned short i = 0; i < getOrderCount(); i++ ) {
				PPP_TEST( !getOrder(i) );
				if (( getOrder(i)->getIndex() != s3mOrderEnd ) && ( getOrder(i)->getIndex() != s3mOrderSkip ) && ( getOrder(i)->getCount() == 0 ) ) {
					if ( nCount == 0 )
						setMultiTrack(true);
					nCount++;
				}
			}
			LOG_MESSAGE_( "Trying to jump to the next track" );
		} while ( jumpNextTrack() );
		LOG_MESSAGE_( "Lengths calculated, resetting module." );
		if ( getTrackCount() > 0 )
			restoreState( getMultiTrack(0).startOrder, 0 );
		LOG_MESSAGE_( "Removing empty tracks" );
		removeEmptyTracks();
//		if (aMultiTrack)
//			LOG_MESSAGE("Hmmm... This could be a multi-song module, there are never played orders");
		return true;
	}
	PPP_CATCH_ALL();
}

bool S3mModule::existsSample( int16_t idx ) throw() {
	idx--;
	if ( !inRange<int>( idx, 0, getSamples()->size() - 1 ) )
		return false;
	return getSample(idx).get();
}

std::string S3mModule::getSampleName( int16_t idx ) throw() {
	if ( !existsSample( idx ) )
		return "";
	return getSample(idx-1)->getTitle();
}

bool S3mModule::existsInstr( int16_t ) const throw() {
	return false;
}

std::string S3mModule::getInstrName( int16_t ) const throw() {
	return "";
}

void S3mModule::checkGlobalFx() throw( PppException ) {
	try {
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		GenPattern::Ptr currPat = getPattern( getPlaybackInfo().pattern );
		if ( !currPat )
			return;
		std::string data;
		for ( unsigned int currTrack = 0; currTrack < getMappedChannelCount(); currTrack++ ) {
			S3mCell::Ptr cell = std::static_pointer_cast<S3mCell>( currPat->getCell( m_channelMappings[currTrack], getPlaybackInfo().row ) );
			if ( !cell )
				continue;
			if ( !cell->isActive() )
				continue;
			if ( cell->getEffect() == s3mEmptyCommand )
				continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if (( fx == s3mFxSpeed ) && ( fxVal != 0 ) )
				setSpeed( fxVal );
			else if (( fx == s3mFxTempo ) && ( fxVal > 0x20 ) )
				setTempo( fxVal );
			else if ( fx == s3mFxGlobalVol ) {
				if ( fxVal <= 0x40 )
					setGlobalVolume( fxVal );
			}
		}
		// check for pattern loops
		int patLoopCounter = 0;
		for ( unsigned int currTrack = 0; currTrack < getMappedChannelCount(); currTrack++ ) {
			S3mCell::Ptr cell = std::static_pointer_cast<S3mCell>( currPat->getCell( m_channelMappings[currTrack], getPlaybackInfo().row ) );
			if ( !cell ) continue;
			if ( !cell->isActive() ) continue;
			if ( cell->getEffect() == s3mEmptyCommand ) continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if ( fx != s3mFxSpecial ) continue;
			if ( highNibble( fxVal ) != s3mSFxPatLoop ) continue;
			if ( lowNibble( fxVal ) == 0x00 ) { // loop start
				m_patLoopRow = getPlaybackInfo().row;
			}
			else { // loop return
				patLoopCounter++;
				if ( m_patLoopCount == -1 ) { // first loop return -> set loop count
					m_patLoopCount = lowNibble( fxVal );
					m_breakRow = m_patLoopRow;
				}
				else if ( m_patLoopCount > 1 ) { // non-initial return -> decrease loop counter
					m_patLoopCount--;
					m_breakRow = m_patLoopRow;
				}
				else { // loops done...
					if ( patLoopCounter == 1 ) { // one loop, all ok
						m_patLoopCount = -1;
						m_breakRow = -1;
						m_patLoopRow = getPlaybackInfo().row + 1;
					}
					else { // we got an "infinite" loop...
						m_patLoopCount = 127;
						m_breakRow = m_patLoopRow;
						LOG_WARNING_( "Infinite pattern loop detected" );
					}
				}
			}
		}
		// check for pattern delays
		int patDelayCounter = 0;
		for ( unsigned int currTrack = 0; currTrack < getMappedChannelCount(); currTrack++ ) {
			S3mCell::Ptr cell = std::static_pointer_cast<S3mCell>( currPat->getCell( m_channelMappings[currTrack], getPlaybackInfo().row ) );
			if ( !cell ) continue;
			if ( !cell->isActive() ) continue;
			if ( cell->getEffect() == s3mEmptyCommand ) continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if ( fx != s3mFxSpecial ) continue;
			if ( highNibble( fxVal ) != s3mSFxPatDelay ) continue;
			if ( lowNibble( fxVal ) == 0 ) continue;
			if ( ++patDelayCounter != 1 ) continue;
			if ( m_patDelayCount != -1 ) continue;
			m_patDelayCount = lowNibble( fxVal );
		}
		if ( m_patDelayCount > 1 )
			m_patDelayCount--;
		else
			m_patDelayCount = -1;
		// now check for breaking effects
		for ( unsigned int currTrack = 0; currTrack < getMappedChannelCount(); currTrack++ ) {
			if ( m_patLoopCount != -1 ) break;
			S3mCell::Ptr cell = std::static_pointer_cast<S3mCell>( currPat->getCell( m_channelMappings[currTrack], getPlaybackInfo().row ) );
			if ( !cell ) continue;
			if ( !cell->isActive() ) continue;
			if ( cell->getEffect() == s3mEmptyCommand ) continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if ( fx == s3mFxJumpOrder ) {
				m_breakOrder = fxVal;
			}
			else if ( fx == s3mFxBreakPat ) {
				m_breakRow = highNibble( fxVal ) * 10 + lowNibble( fxVal );
				LOG_MESSAGE( "Row %d: Break pattern to row %d", getPlaybackInfo().row, m_breakRow );
			}
		}
	}
	PPP_RETHROW()
	catch ( ... ) {
		PPP_THROW( "Unknown Exception" );
	}
}

bool S3mModule::adjustPosition( const bool increaseTick, const bool doStore ) throw( PppException ) {
	LOG_BEGIN();
	PPP_TEST( getOrderCount() == 0 );
	bool orderChanged = false;
	//! @todo Implement saving of the last order for back-jumping
//	short lastOrder = aPlaybackInfo.order;
	if ( increaseTick ) {
		nextTick();
/*		m_playbackInfo.tick++;
		PPP_TEST( getPlaybackInfo().speed == 0 );
		m_playbackInfo.tick %= getPlaybackInfo().speed;*/
	}
	if (( getPlaybackInfo().tick == 0 ) && increaseTick ) {
		m_patDelayCount = -1;
		if ( m_breakOrder != -1 ) {
			mapOrder( getPlaybackInfo().order )->incCount();
			if ( m_breakOrder < getOrderCount() ) {
				setOrder( m_breakOrder );
				orderChanged = true;
			}
			setRow( 0 );
		}
		if ( m_breakRow != -1 ) {
			if ( m_breakRow <= 63 ) {
				setRow( m_breakRow );
			}
			if ( m_breakOrder == -1 ) {
				if ( m_patLoopCount == -1 ) {
					mapOrder( getPlaybackInfo().order )->incCount();
					setOrder( getPlaybackInfo().order+1 );
					orderChanged = true;
				}
				//else {
				//	LOG_MESSAGE(stringf("oO... aPatLoopCount=%d",aPatLoopCount));
				//}
			}
		}
		if (( m_breakRow == -1 ) && ( m_breakOrder == -1 ) && ( m_patDelayCount == -1 ) ) {
			setRow( (getPlaybackInfo().row+1) & 0x3f );
			if ( getPlaybackInfo().row == 0 ) {
				mapOrder( getPlaybackInfo().order )->incCount();
					setOrder( getPlaybackInfo().order+1 );
					orderChanged = true;
			}
		}
		m_breakRow = m_breakOrder = -1;
	}
	setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
	// skip "--" and "++" marks
	while ( getPlaybackInfo().pattern >= 254 ) {
		if ( getPlaybackInfo().pattern == s3mOrderEnd ) {
			LOG_TEST_MESSAGE( getPlaybackInfo().pattern == s3mOrderEnd );
			return false;
		}
		if ( !mapOrder( getPlaybackInfo().order ) )
			return false;
		mapOrder( getPlaybackInfo().order )->incCount();
		setOrder( getPlaybackInfo().order+1 );
		orderChanged = true;
		if ( getPlaybackInfo().order >= getOrderCount() ) {
			LOG_MESSAGE_( "Song end reached: End of orders" );
			return false;
		}
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
	}
	if ( orderChanged ) {
		m_patLoopRow = 0;
		m_patLoopCount = -1;
		try {
			if ( doStore )
				saveState();
			else {
				PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
				restoreState( getPlaybackInfo().order, mapOrder( getPlaybackInfo().order )->getCount() );
			}
		}
		PPP_CATCH_ALL()
	}
	return true;
}

void S3mModule::getTick( AudioFrameBuffer &buf ) throw( PppException ) {
	LOG_BEGIN();
	try {
		//PPP_TEST(!buf);
		if(!buf)
			buf.reset(new AudioFrameBuffer::element_type);
		if ( getPlaybackInfo().tick == 0 )
			checkGlobalFx();
		//buf->resize(getTickBufLen());
		//buf->clear();
		if ( !adjustPosition( false, false ) ) {
			LOG_MESSAGE_( "Song end reached: adjustPosition() failed" );
			return;
		}
		if ( mapOrder( getPlaybackInfo().order )->getCount() >= getMaxRepeat() ) {
			LOG_MESSAGE_( "Song end reached: Maximum repeat count reached" );
			return;
		}
		// update channels...
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		GenPattern::Ptr currPat = getPattern( getPlaybackInfo().pattern );
		if ( !currPat )
			return;
		MixerFrameBuffer mixerBuffer( new MixerFrameBuffer::element_type( getTickBufLen(), {0,0} ) );
		for ( unsigned short currTrack = 0; currTrack < getMappedChannelCount(); currTrack++ ) {
			GenChannel::Ptr chan = getMappedChannel( currTrack );
			PPP_TEST( !chan );
			GenCell::Ptr cell = currPat->getCell( m_channelMappings[currTrack], getPlaybackInfo().row );
			chan->update( cell, getPlaybackInfo().tick, m_patDelayCount != -1 );
			chan->mixTick( mixerBuffer, getPlaybackInfo().globalVolume );
		}
		buf->resize(mixerBuffer->size());
		MixerSample *mixerBufferPtr = &mixerBuffer->front().left;
		BasicSample *bufPtr = &buf->front().left;
		for ( std::size_t i = 0; i < mixerBuffer->size(); i++ ) { // postprocess...
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
		if ( getPlaybackInfo().tick == 0 )
			checkGlobalFx();
		bufLen = 0;
		if ( !adjustPosition( false, true ) )
			return;
		PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
		if ( mapOrder( getPlaybackInfo().order )->getCount() >= getMaxRepeat() )
			return;
		// update channels...
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		GenPattern::Ptr currPat = getPattern( getPlaybackInfo().pattern );
		if ( !currPat )
			return;
		bufLen = getTickBufLen(); // in frames
		for ( unsigned short currTrack = 0; currTrack < getMappedChannelCount(); currTrack++ ) {
			GenChannel::Ptr chan = getMappedChannel( currTrack );
			PPP_TEST( !chan );
			GenCell::Ptr cell = currPat->getCell( m_channelMappings[currTrack], getPlaybackInfo().row );
			chan->update( cell, getPlaybackInfo().tick, m_patDelayCount != -1 );
			chan->simTick( bufLen, getPlaybackInfo().globalVolume );
		}
		adjustPosition( true, true );
		setPosition( getPosition() + bufLen );
	}
	PPP_CATCH_ALL();
}

bool S3mModule::jumpNextOrder() throw() {
	int currOrder = getPlaybackInfo().order;
	while ( currOrder == getPlaybackInfo().order ) {
		if ( !adjustPosition( true, false ) )
			return false;
	}
	return true;
}

GenOrder::Ptr S3mModule::mapOrder( int16_t order ) throw() {
	static GenOrder::Ptr xxx( new GenOrder( s3mOrderEnd ) );
	xxx->setCount( 0xff );
	if ( !inRange<int16_t>( order, 0, getOrderCount() - 1 ) )
		return xxx;
	return getOrder(order);
}

// GenPattern::Ptr S3mModule::getPattern( int16_t n ) throw() {
// 	if ( !inRange<int16_t>( n, 0, getPatternCount() - 1 ) )
// 		return GenPattern::Ptr();
// 	return getPattern(n);
// }

GenChannel::Ptr S3mModule::getMappedChannel( int16_t n ) throw() {
	if ( !inRange<int16_t>( n, 0, getMappedChannelCount() - 1 ) )
		return GenChannel::Ptr();
	return getChannel(m_channelMappings[n]);
}

// GenSample::Ptr S3mModule::getSmp( int16_t n ) throw() {
// 	if ( !existsSample( n ) )
// 		return GenSample::Ptr();
// 	return getSample(n);
// }

std::string S3mModule::getChanStatus( int16_t idx ) throw() {
	GenChannel::Ptr x = getMappedChannel( idx );
	if ( !x )
		return "";
	return x->getStatus();
}

std::string S3mModule::getChanCellString( int16_t idx ) throw() {
	GenChannel::Ptr x = getMappedChannel( idx );
	if ( !x )
		return "";
	return x->getCellString();
}

bool S3mModule::jumpNextTrack() throw( PppException ) {
	LOG_BEGIN();
	if ( !isMultiTrack() ) {
		LOG_MESSAGE_( "This is not a multi-track" );
		return false;
	}
	PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
	mapOrder( getPlaybackInfo().order )->incCount();
	setCurrentTrack( getCurrentTrack()+1 );
	if ( getCurrentTrack() >= getTrackCount() ) {
		GenMultiTrack nulltrack;
		for ( uint16_t i = 0; i < getOrderCount(); i++ ) {
			PPP_TEST( !getOrder(i) );
			if (( getOrder(i)->getIndex() != s3mOrderEnd ) && ( getOrder(i)->getIndex() != s3mOrderSkip ) && ( getOrder(i)->getCount() == 0 ) ) {
				PPP_TEST( !mapOrder( i ) );
				setPatternIndex( mapOrder( i )->getIndex() );
				setOrder( i );
				nulltrack.startOrder = i;
				addMultiTrack( nulltrack );
				setPosition( 0 );
				saveState();
				return true;
			}
		}
		nulltrack.startOrder = GenMultiTrack::stopHere;
		addMultiTrack( nulltrack );
		return false;
	}
	else {
		if ( getMultiTrack(getCurrentTrack()).startOrder == GenMultiTrack::stopHere ) {
			LOG_MESSAGE_( "No more tracks" );
			return false;
		}
		setOrder( getMultiTrack(getCurrentTrack()).startOrder );
		PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
		setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
		setPosition( 0 );
		restoreState( getPlaybackInfo().order, 0 );
		return true;
	}
	LOG_ERROR_( "This should definitively NOT have happened..." );
	return false;
}

bool S3mModule::jumpPrevTrack() throw( PppException ) {
	LOG_BEGIN();
	if ( !isMultiTrack() ) {
		LOG_MESSAGE_( "This is not a multi-track" );
		return false;
	}
	if ( getCurrentTrack() == 0 ) {
		LOG_MESSAGE_( "Already on first track" );
		return false;
	}
	setCurrentTrack( getCurrentTrack()-1 );
	setOrder( getMultiTrack(getCurrentTrack()).startOrder );
	PPP_TEST( !mapOrder( getPlaybackInfo().order ) );
	setPatternIndex( mapOrder( getPlaybackInfo().order )->getIndex() );
	setPosition( 0 );
	restoreState( getPlaybackInfo().order, 0 );
	return true;
}

BinStream &S3mModule::saveState() throw( PppException ) {
	try {
		BinStream &str = GenModule::saveState();
		str.write( &m_breakRow )
		.write( &m_breakOrder )
		.write( &m_patLoopRow )
		.write( &m_patLoopCount )
		.write( &m_patDelayCount )
		.write( &m_customData )
		.write( m_channelMappings, 32 );
		return str;
	}
	PPP_CATCH_ALL();
}

BinStream &S3mModule::restoreState( unsigned short ordindex, unsigned char cnt ) throw( PppException ) {
	try {
		BinStream &str = GenModule::restoreState( ordindex, cnt );
		str.read( &m_breakRow )
		.read( &m_breakOrder )
		.read( &m_patLoopRow )
		.read( &m_patLoopCount )
		.read( &m_patDelayCount )
		.read( &m_customData )
		.read( m_channelMappings, 32 );
		return str;
	}
	PPP_CATCH_ALL();
}
