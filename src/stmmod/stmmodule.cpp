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

#include "stmmodule.h"

#include <iostream>
#include <algorithm>

using namespace ppp;
using namespace ppp::stm;

/**
* @file
* @ingroup StmMod
* @brief Module definitions for ScreamTracker 3 Modules
*/

/**
* @brief Parapointer, 16-byte aligned
* @ingroup StmMod
*/
typedef unsigned short ParaPointer;

/**
* @brief STM Module Header
* @ingroup StmMod
*/
typedef struct __attribute__((packed)) {
	char title[20]; //!< @brief The module's title
	char trackerName[8]; //!< @brief Tracker name
	unsigned char id; //!< @brief Tracker ID
	unsigned char fileType; //!< @brief File type
	unsigned char majorVersion; //!< @brief Major version
	unsigned char minorVersion; //!< @brief Minor version
	unsigned char initTempo; //!< @brief Initial tempo
	unsigned char patNum; //!< @brief Number of patterns in the module
	unsigned char globalVolume; //!< @brief Global volume
	unsigned char reserved[13]; //!< @brief Reserved data
} stmModuleHeader;

StmModule::StmModule( const unsigned int frq, const unsigned short maxRpt ) throw( PppException ) : GenModule( frq, maxRpt ),
		m_breakRow( -1 ), m_breakOrder( -1 ) {
	try {
		for ( int i = 0; i < 256; i++ ) {
			m_orders.push_back( GenOrder::Ptr( new GenOrder( stmOrderEnd ) ) );
			m_patterns.push_back( GenPattern::Ptr() );
			m_samples->push_back( GenSample::Ptr() );
		}
		for ( int i = 0; i < 4; i++ ) {
			StmChannel::CP cleanChannel( new StmChannel( m_playbackFrequency, m_samples ) );
			m_channels.push_back( std::static_pointer_cast<GenChannel>( cleanChannel ) );
			m_channels[i]->setPanning( 0x40 );
			m_channels[i]->enable();
		}
		m_channelCount = 4;
	}
	PPP_CATCH_ALL();
}

StmModule::~StmModule() throw() {
}

bool StmModule::load( const std::string &fn ) throw( PppException ) {
	try {
		LOG_BEGIN();
		m_fileName = fn;
		stmModuleHeader stmHdr;
		LOG_MESSAGE( "Opening '" + fn + "'" );
		FBinStream str( fn );
		if ( !str.good() ) {
			PPP_THROW( "File Open Error" );
		}
		str.seek( 0 );
		str.read( stmHdr );
		if ( str.fail() )
			PPP_THROW( "Header could not be read" );
		if ( stmHdr.id != 0x1a ) {
			LOG_ERROR( stringf( "ID not 0x1a: 0x%.2x", stmHdr.id ) );
			return false;
		}
		m_trackerInfo = "";
		for ( unsigned char i = 0; i < 8; i++ ) {
			if ( stmHdr.trackerName[i] == 0x00 )
				break;
			m_trackerInfo += stmHdr.trackerName[i];
		}
		m_trackerInfo += stringf( " v%x.%.2x", stmHdr.majorVersion, stmHdr.minorVersion );
		for ( int i = 0; ( i < 20 ) && ( stmHdr.title[i] != 0x00 ); i++ )
			m_title += stmHdr.title[i];
		setTempo(125)
		setSpeed( highNibble( stmHdr.initTempo ) );
		m_playbackInfo.globalVolume = stmHdr.globalVolume;
		// load the samples
		for ( int i = 0; i < 31; i++ ) {
			if ( !str.good() )
				PPP_THROW( "Stream Error: Samples" );
			StmSample::CP smp( new StmSample() );
			( *m_samples )[i] = smp;
			if ( !( smp->load( str, str.pos() ) ) ) {
				return false;
			}
			if ( !str.good() )
				PPP_THROW( "Stream Error: Samples / Load" );
		}
		// load orders...
		for ( int i = 0; i < 128; i++ ) {
			unsigned char buf;
			str.read( buf );
			m_orders[i]->setIndex( buf );
		}
		// ok, now load the patterns...
		for ( int i = 0; i < stmHdr.patNum; i++ ) {
			StmPattern::CP pat( new StmPattern() );
			m_patterns[i] = pat;
			if ( !( pat->load( str, str.pos() ) ) ) {
				return false;
			}
			if ( !str.good() )
				PPP_THROW( "Stream Error: Patterns" );
		}
		//str.close();
		// calc length
		do {
			std::size_t currTickLen = 0;
			m_tracks[m_currentTrack].startOrder = m_playbackInfo.order;
			do {
				try {
					getTickNoMixing( currTickLen );
					m_tracks[m_currentTrack].length += currTickLen;
				}
				PPP_RETHROW()
				catch ( ... ) {
					PPP_THROW( "Unknown Exception" );
				}
			}
			while ( currTickLen != 0 );
		}
		while ( jumpNextTrack() );
		m_currentTrack = 0;
		// reset
		m_playbackInfo.speed = highNibble( stmHdr.initTempo );
		m_playbackInfo.tick = m_playbackInfo.order = m_playbackInfo.pattern = m_playbackInfo.row = 0;
		for ( unsigned short i = 0; i < m_orders.size(); i++ )
			m_orders[i]->resetCount();
		for ( unsigned char i = 0; i < 4; i++ ) {
			m_channels[i].reset( new StmChannel( m_playbackFrequency, m_samples ) );
			m_channels[i]->enable();
		}
	}
	PPP_CATCH_ALL();
	return true;
}

bool StmModule::existsSample( int16_t idx ) throw() {
	idx--;
	if ( !inRange<int>( idx, 0, m_samples->size() - 1 ) )
		return false;
	return ( *m_samples )[idx];
}

std::string StmModule::getSampleName( int16_t idx ) throw() {
	if ( !existsSample( idx ) )
		return "";
	return ( *m_samples )[idx-1]->getTitle();
}

bool StmModule::existsInstr( int16_t /*idx*/ ) const throw() {
	return false;
}

std::string StmModule::getInstrName( int16_t /*idx*/ ) const throw() {
	return "";
}

void StmModule::checkGlobalFx() throw( PppException ) {
	try {
		m_playbackInfo.pattern = mapOrder( m_playbackInfo.order )->getIndex();
		GenPattern::Ptr currPat = getPattern( m_playbackInfo.pattern );
		if ( !currPat )
			return;
		std::string data = "";
		for ( unsigned int currTrack = 0; currTrack < m_channelCount; currTrack++ ) {
			StmCell::Ptr cell = std::static_pointer_cast<StmCell>( currPat->getCell( currTrack, m_playbackInfo.row ) );
			if ( !cell )
				continue;
			if ( !cell->isActive() )
				continue;
			if ( cell->getEffect() == stmEmptyEffect )
				continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if (( fx == stmFxSetTempo ) && ( fxVal != 0 ) ) {
				m_playbackInfo.speed = highNibble( fxVal );
			}
		}
		// now check for breaking effects
		for ( unsigned int currTrack = 0; currTrack < m_channelCount; currTrack++ ) {
			StmCell::Ptr cell = std::static_pointer_cast<StmCell>( currPat->getCell( currTrack, m_playbackInfo.row ) );
			if ( !cell )
				continue;
			if ( !cell->isActive() )
				continue;
			if ( cell->getEffect() == stmEmptyEffect )
				continue;
			unsigned char fx = cell->getEffect();
			unsigned char fxVal = cell->getEffectValue();
			if ( fx == stmFxPatJump )
				m_breakOrder = fxVal;
			else if ( fx == stmFxPatBreak )
				m_breakRow = highNibble( fxVal ) * 10 + lowNibble( fxVal );
		}
	}
	PPP_RETHROW()
	catch ( ... ) {
		PPP_THROW( "Unknown Exception" );
	}
}

bool StmModule::adjustPosition( const bool increaseTick ) throw( PppException ) {
	PPP_TEST( m_orders.size() == 0 );
	if ( increaseTick ) {
		m_playbackInfo.tick++;
		PPP_TEST( m_playbackInfo.speed == 0 );
		m_playbackInfo.tick %= m_playbackInfo.speed;
	}
	if (( m_playbackInfo.tick == 0 ) && increaseTick ) {
		if ( m_breakOrder != -1 ) {
			mapOrder( m_playbackInfo.order )->incCount();
			if ( m_breakOrder < static_cast<signed>( m_orders.size() ) ) {
				m_playbackInfo.order = m_breakOrder;
			}
			m_playbackInfo.row = 0;
		}
		if ( m_breakRow != -1 ) {
			if ( m_breakRow <= 63 ) {
				m_playbackInfo.row = m_breakRow;
			}
			if ( m_breakOrder == -1 ) {
				mapOrder( m_playbackInfo.order )->incCount();
				m_playbackInfo.order++;
			}
		}
		if (( m_breakRow == -1 ) && ( m_breakOrder == -1 ) ) {
			m_playbackInfo.row++;
			m_playbackInfo.row %= 64;
			if ( m_playbackInfo.row == 0 ) {
				mapOrder( m_playbackInfo.order )->incCount();
				m_playbackInfo.order++;
				m_playbackInfo.order %= m_orders.size();
			}
		}
		m_breakRow = m_breakOrder = -1;
	}
	m_playbackInfo.pattern = mapOrder( m_playbackInfo.order )->getIndex();
	// skip "--" and "++" marks
	if ( m_playbackInfo.pattern == stmOrderEnd )
		return false;
	if ( m_playbackInfo.order >= static_cast<signed>( m_orders.size() ) )
		return false;
	m_playbackInfo.pattern = mapOrder( m_playbackInfo.order )->getIndex();
	return true;
}

void StmModule::getTick( AudioFifo::AudioBuffer &buf, std::size_t &bufLen ) throw( PppException ) {
	try {
		if ( m_playbackInfo.tick == 0 )
			checkGlobalFx();
		bufLen = 0;
		buf.reset();
		if ( !adjustPosition( false ) )
			return;
		if ( mapOrder( m_playbackInfo.order )->getCount() >= m_maxRepeat )
			return;
		// update channels...
		m_playbackInfo.pattern = mapOrder( m_playbackInfo.order )->getIndex();
		GenPattern::Ptr currPat = getPattern( m_playbackInfo.pattern );
		if ( !currPat )
			return;
		bufLen = getTickBufLen(); // in frames
		//const unsigned short SAMPLESIZE = sizeof( int );
		//const unsigned short FRAMESIZE = SAMPLESIZE * 2;
		AudioFifo::MixerBuffer mixerBuffer( new int32_t[bufLen*2] );
		std::fill_n(mixerBuffer.get(), bufLen*2, 0);
		//memset( mixerBuffer.get(), 0, bufLen * FRAMESIZE );
		for ( unsigned short currTrack = 0; currTrack < m_channelCount; currTrack++ ) {
			GenChannel::Ptr chan = getChannel( currTrack );
			PPP_TEST( !chan );
			GenCell::Ptr cell = currPat->getCell( currTrack, m_playbackInfo.row );
			chan->update( cell, m_playbackInfo.tick, false );
			chan->mixTick( mixerBuffer, bufLen, m_playbackInfo.globalVolume );
		}
		buf.reset( new short[bufLen*2] );
		for ( unsigned int i = 0; i < bufLen*2; i++ ) // postprocess...
			buf[i] = clipSample( mixerBuffer[i] >> 2 );
		adjustPosition( true );
	}
	PPP_RETHROW()
}

void StmModule::getTickNoMixing( std::size_t& bufLen ) throw( PppException ) {
	try {
		if ( m_playbackInfo.tick == 0 )
			checkGlobalFx();
		bufLen = 0;
		if ( !adjustPosition( false ) )
			return;
		if ( mapOrder( m_playbackInfo.order )->getCount() >= m_maxRepeat )
			return;
		// update channels...
		m_playbackInfo.pattern = mapOrder( m_playbackInfo.order )->getIndex();
		GenPattern::Ptr currPat = getPattern( m_playbackInfo.pattern );
		if ( !currPat )
			return;
		bufLen = getTickBufLen();
		adjustPosition( true );
	}
	PPP_RETHROW()
	catch ( ... ) {
		PPP_THROW( "Unknown Exception" );
	}
}

GenOrder::Ptr StmModule::mapOrder( int16_t order ) throw() {
	static GenOrder::Ptr xxx( new GenOrder( stmOrderEnd ) );
	xxx->setCount( 0xff );
	if ( !inRange<int>( order, 0, m_orders.size() - 1 ) )
		return xxx;
	return m_orders[order];
}

GenPattern::Ptr StmModule::getPattern( int n ) throw() {
	if ( !inRange<int>( n, 0, m_patterns.size() - 1 ) )
		return GenPattern::Ptr();
	return m_patterns[n];
}

GenChannel::Ptr StmModule::getChannel( int n ) throw() {
	if ( !inRange( n, 0, m_channelCount - 1 ) )
		return GenChannel::Ptr();
	return m_channels[n];
}

GenSample::Ptr StmModule::getSmp( int n ) throw() {
	if ( !existsSample( n ) )
		return GenSample::Ptr();
	return ( *m_samples )[n];
}

std::string StmModule::getChanStatus( int16_t idx ) throw() {
	GenChannel::Ptr x = getChannel( idx );
	if ( x )
		return "";
	return x->getStatus();
}

bool StmModule::jumpNextTrack() throw() {
	return false;
}
bool StmModule::jumpPrevTrack() throw() {
	return false;
}
