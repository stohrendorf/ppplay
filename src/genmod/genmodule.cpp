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

#include "genbase.h"
#include "genmodule.h"
//#include <cmath>

/**
* @file
* @ingroup GenMod
* @brief General/common module definitions
*/

using namespace ppp;

IArchive* GenMultiTrack::newState() {
	IArchive::Ptr p( new MemArchive() );
	LOG_MESSAGE( "Creating new state %u", m_states.size() );
	m_states.push_back( p );
	return p.get();
}

IArchive* GenMultiTrack::nextState() {
	if( m_stateIndex >= m_states.size() ) {
		LOG_ERROR( "%d >= %d", m_stateIndex, m_states.size() );
		return NULL;
	}
	m_stateIndex++;
	LOG_MESSAGE( "Loading state %u", m_stateIndex );
	IArchive* result = m_states[m_stateIndex].get();
	return result;
}

IArchive* GenMultiTrack::prevState() {
	if( m_stateIndex <= 1 ) {
		LOG_ERROR( "m_stateIndex <= 1" );
		return NULL;
	}
	m_stateIndex--;
	LOG_MESSAGE( "Loading state %u", m_stateIndex );
	IArchive* result = m_states[m_stateIndex].get();
	return result;
}

GenModule::GenModule( uint32_t frq, uint8_t maxRpt ) throw( PppException ) :
	m_fileName(), m_title(), m_trackerInfo(), m_orders(), m_maxRepeat( maxRpt ),
	m_playbackFrequency( clip<unsigned int>( frq, 11025, 44800 ) ), m_playedFrames( 0 ), m_tracks(),
	m_currentTrack( 0 ), m_multiTrack( false ), m_playbackInfo() {
	PPP_TEST( maxRpt == 0 );
	m_playbackInfo.tick = m_playbackInfo.order = m_playbackInfo.pattern = 0;
	m_playbackInfo.row = m_playbackInfo.speed = m_playbackInfo.tempo = 0;
	m_playbackInfo.globalVolume = 0x40;
	GenMultiTrack nulltrack;
	m_tracks.push_back( nulltrack );
}

GenModule::~GenModule() {
}

IArchive& GenModule::serialize( IArchive* data ) {
	data->array( reinterpret_cast<char*>( &m_playbackInfo ), sizeof( m_playbackInfo ) ) & m_playedFrames & m_currentTrack;
	/*	for ( uint_fast16_t i = 0; i < m_orders.size(); i++ ) {
			if ( !m_orders[i] )
				continue;
			data->archive(m_orders[i].get());
		}*/
	return *data;
}

std::string GenModule::getFileName() throw( PppException ) {
	try {
		std::size_t lastPos = m_fileName.find_last_of( "/\\" );
		if( lastPos != std::string::npos )
			return m_fileName.substr( lastPos + 1 );
		return m_fileName;
	}
	PPP_CATCH_ALL();
}

std::string GenModule::getTitle() const throw() {
	return m_title;
}

std::string GenModule::getTrimTitle() const throw() {
	return trimString(m_title);
}

uint32_t GenModule::timeElapsed() const throw( PppException ) {
	PPP_TEST( m_playbackFrequency == 0 );
	return static_cast<uint32_t>( m_playedFrames / m_playbackFrequency );
}

uint32_t GenModule::getLength() const throw() {
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
	[&nTr]( const GenMultiTrack & mt ) {
		if( mt.length != 0 && mt.startOrder != GenMultiTrack::stopHere ) nTr.push_back( mt );
	}
	);
	/*	for(std::vector<GenMultiTrack>::iterator it = m_tracks.begin(); it!=m_tracks.end(); it++) {
			if (( it->length != 0 ) && ( it->startOrder != GenMultiTrack::stopHere ) )
				nTr.push_back( *it );
		}*/
	m_tracks = nTr;
	m_multiTrack = getTrackCount() > 1;
}

std::size_t GenModule::getAudioData( AudioFrameBuffer& buffer, std::size_t size ) {
	if( !buffer )
		buffer.reset( new AudioFrameBuffer::element_type );
	while( buffer->size() < size ) {
		AudioFrameBuffer tmpBuf;
		getTick( tmpBuf );
		if( !tmpBuf || tmpBuf->size() == 0 )
			return 0;
		buffer->insert( buffer->end(), tmpBuf->begin(), tmpBuf->end() );
	}
	return buffer->size();
}
