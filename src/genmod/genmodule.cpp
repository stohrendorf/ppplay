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

#include "genbase.h"
#include "genmodule.h"
//#include <cmath>

/**
* @file
* @ingroup GenMod
* @brief General/common module definitions
*/

using namespace ppp;

GenModule::GenModule( const uint32_t frq, const uint8_t maxRpt ) throw( PppException ) :
		m_fileName( "" ), m_title( "" ), m_trackerInfo( "" ), m_orders(), m_maxRepeat( maxRpt ),
		m_playbackFrequency( clip<unsigned int>( frq, 11025, 44800 ) ), m_playedFrames( 0 ), m_tracks(),
		m_currentTrack( 0 ), m_playbackInfo(), m_multiTrack( false ), playbackFifo( 2048 ) {
	PPP_TEST( maxRpt==0 );
	m_playbackInfo.tick = m_playbackInfo.order = m_playbackInfo.pattern = 0;
	m_playbackInfo.row = m_playbackInfo.speed = m_playbackInfo.tempo = 0;
	m_playbackInfo.globalVolume = 0x40;
	GenMultiTrack nulltrack;
	m_tracks.push_back( nulltrack );
}

GenModule::GenModule( const GenModule &src ) throw( PppException ) :
		m_fileName( src.m_fileName ), m_title( src.m_title ), m_trackerInfo( src.m_trackerInfo ), m_orders( src.m_orders ),
		m_maxRepeat( src.m_maxRepeat ), m_playbackFrequency( src.m_playbackFrequency ),
		m_playedFrames( src.m_playedFrames ), m_tracks( src.m_tracks ), m_currentTrack( src.m_currentTrack ),
		m_playbackInfo( src.m_playbackInfo ), m_multiTrack( src.m_multiTrack ), playbackFifo( 2048 ) {
	try {
		playbackFifo.setMinFrameCount(src.playbackFifo.getMinFrameCount());
		GenMultiTrack nulltrack;
		m_tracks.push_back( nulltrack );
	}
	PPP_CATCH_ALL();
}

GenModule &GenModule::operator=( const GenModule & src ) throw( PppException ) {
	try {
		m_fileName = src.m_fileName;
		m_title = src.m_title;
		m_trackerInfo = src.m_trackerInfo;
		m_orders = src.m_orders;
		m_maxRepeat = src.m_maxRepeat;
		m_playbackFrequency = src.m_playbackFrequency;
		m_playedFrames = src.m_playedFrames;
		m_tracks = src.m_tracks;
		m_currentTrack = src.m_currentTrack;
		m_playbackInfo = src.m_playbackInfo;
		m_multiTrack = src.m_multiTrack;
		return *this;
	}
	PPP_CATCH_ALL();
}

GenModule::~GenModule() {
}

BinStream &GenModule::saveState() throw( PppException ) {
	LOG_BEGIN();
	GenOrder::Ptr ord = m_orders[m_playbackInfo.order];
	PPP_TEST( !ord );
	LOG_MESSAGE( "Saving state for order %d, loop count %d", m_playbackInfo.order, ord->getCount() );
	std::cout << std::flush;
	BinStream::SpBinStream str = ord->getCurrentState();
	PPP_TEST( !str );
	str->seek( 0 );
	str->clear();
	// save playback info
	str->write( reinterpret_cast<const char*>(&m_playbackInfo), sizeof(m_playbackInfo) ).write( &m_playedFrames ).write( &m_currentTrack );
	// save order counts
	for ( uint_fast16_t i = 0; i < m_orders.size(); i++ ) {
		if ( !m_orders[i] )
			continue;
		unsigned char tmp = m_orders[i]->getCount();
		str->write( &tmp );
	}
	return *str;
}

BinStream &GenModule::restoreState( uint16_t ordindex, uint8_t cnt ) throw( PppException ) {
	LOG_BEGIN();
	GenOrder::Ptr ord = m_orders[ordindex];
	PPP_TEST( !ord );
	LOG_MESSAGE( "Loading state for order %d, loop count %d", ordindex, cnt );
	std::cout << std::flush;
	BinStream::SpBinStream str = ord->getState( cnt );
	PPP_TEST( !str );
	str->clear();
	str->seek( 0 );
	// load playback info
	str->read( reinterpret_cast<char*>(&m_playbackInfo), sizeof(m_playbackInfo) ).read( &m_playedFrames ).read( &m_currentTrack );
	// load order counts
	for ( uint_fast16_t i = 0; i < m_orders.size(); i++ ) {
		if ( !m_orders[i] )
			continue;
		unsigned char tmp;
		str->read( &tmp );
		m_orders[i]->setCount( tmp );
	}
	return *str;
}

void GenModule::initFifo( std::size_t nFrames ) throw( PppException ) {
	PPP_TEST( nFrames <= 0 );
	playbackFifo.setMinFrameCount( nFrames );
	LOG_MESSAGE( "Set FIFO length to %ld frames", nFrames );
	/*	aPlaybackBuffer.reset(new short[nFrames*2]);
		aPlaybackBufferSize = nFrames;*/
}

bool GenModule::getFifo( AudioFrameBuffer& buffer, std::size_t count ) throw( PppException ) {
	if(!fillFifo())
		return false;
	playbackFifo.get( buffer, count );
	return true;
}

bool GenModule::fillFifo() throw(PppException) {
	AudioFrameBuffer td(new AudioFrameBuffer::element_type);
	while ( playbackFifo.needsData() ) {
		getTick( td );
		if ( !td || td->empty() )
			return false;
		playbackFifo.feedChunk( td );
	}
	return true;
}

std::string GenModule::getFileName() throw( PppException ) {
	try {
		std::size_t lastPos = m_fileName.find_last_of("/\\");
		if(lastPos != std::string::npos)
			return m_fileName.substr(lastPos+1);
		return m_fileName;
	}
	PPP_CATCH_ALL();
}

std::string GenModule::getTitle() const throw() {
	return m_title;
}

std::string GenModule::getTrimTitle() const throw() {
	std::string res = m_title;
	std::size_t startpos = res.find_first_not_of( " \t" );
	std::size_t endpos = res.find_last_not_of( " \t" );
	if (( std::string::npos == startpos ) || ( std::string::npos == endpos ) )
		return std::string();
	return res.substr( startpos, endpos - startpos + 1 );
}

std::size_t GenModule::timeElapsed() const throw( PppException ) {
	PPP_TEST( m_playbackFrequency == 0 );
	return static_cast<uint32_t>( m_playedFrames / m_playbackFrequency );
}

std::size_t GenModule::getLength() const throw() {
	return m_tracks[m_currentTrack].length;
}

std::string GenModule::getTrackerInfo() const throw() {
	return m_trackerInfo;
}

GenPlaybackInfo GenModule::getPlaybackInfo() const throw() {
	return m_playbackInfo;
}

bool GenModule::isMultiTrack() const throw() {
	return m_multiTrack;
}

void GenModule::removeEmptyTracks() {
	std::vector<GenMultiTrack> nTr;
	std::for_each(
		m_tracks.begin(), m_tracks.end(),
		[&nTr](GenMultiTrack mt){ if(mt.length!=0 && mt.startOrder!=GenMultiTrack::stopHere) nTr.push_back(mt); }
	);
/*	for(std::vector<GenMultiTrack>::iterator it = m_tracks.begin(); it!=m_tracks.end(); it++) {
		if (( it->length != 0 ) && ( it->startOrder != GenMultiTrack::stopHere ) )
			nTr.push_back( *it );
	}*/
	m_tracks = nTr;
	m_multiTrack = getTrackCount() > 1;
	for ( uint_fast16_t i = 0; i < m_orders.size(); i++ )
		m_orders[i]->resetCount();
}
