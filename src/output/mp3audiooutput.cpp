/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Steffen Ohrendorf <email>

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


#include "mp3audiooutput.h"

#include <lame/lame.h>
#include <ctime>
#include "logger/logger.h"

void MP3AudioOutput::encodeThread( MP3AudioOutput* src ) {
	while( true ) {
		while( src->m_paused )
			usleep( 1000 );
		AudioFrameBuffer buffer;
		if( src->source()->getAudioData( buffer, 1024 ) == 0 ) {
			//! @bug This shows "terminate called without an active exception"
			std::terminate();
			return;
		}
		src->m_bufferMutex.lock();
		int res = lame_encode_buffer_interleaved( src->m_lameGlobalFlags, &buffer->front().left, buffer->size(), src->m_buffer, BufferSize );
		if( res < 0 ) {
			if( res == -1 )
				LOG_ERROR_( "Lame Encoding Buffer too small!" );
			else
				LOG_ERROR_( "Unknown Lame Error." );
		}
		else {
			src->m_file.write( reinterpret_cast<char*>( src->m_buffer ), res );
		}
		src->m_bufferMutex.unlock();
	}
}


MP3AudioOutput::MP3AudioOutput( IAudioSource* src, const std::string& filename ): IAudioOutput( src ),
	m_lameGlobalFlags( NULL ), m_file(), m_filename( filename ), m_buffer( NULL ), m_encoderThread(), m_bufferMutex(), m_paused( true ) {
	m_buffer = new uint8_t[BufferSize];
	m_lameGlobalFlags = lame_init();
}

MP3AudioOutput::~MP3AudioOutput() {
	delete[] m_buffer;
	lame_close( m_lameGlobalFlags );
}

uint16_t MP3AudioOutput::volumeRight() const {
	return 0;
}

uint16_t MP3AudioOutput::volumeLeft() const {
	return 0;
}

void MP3AudioOutput::pause() {
	m_paused = true;
}

void MP3AudioOutput::play() {
	m_paused = false;
}

bool MP3AudioOutput::paused() {
	return m_paused;
}

bool MP3AudioOutput::playing() {
	return !m_paused;
}

int MP3AudioOutput::init( int desiredFrq ) {
	m_file.open( m_filename, std::ios::in );
	if( m_file.is_open() ) {
		m_file.close();
		LOG_ERROR_( "Output file already exists" );
		setErrorCode( OutputUnavailable );
		return 0;
	}
	m_file.open( m_filename, std::ios::out | std::ios::binary );
	if( !m_file.is_open() ) {
		LOG_ERROR_( "Cannot open output file for writing" );
		setErrorCode( OutputUnavailable );
		return 0;
	}
	lame_set_in_samplerate( m_lameGlobalFlags, desiredFrq );
	lame_set_num_channels( m_lameGlobalFlags, 2 );
	lame_set_quality( m_lameGlobalFlags, 5 );
	lame_set_mode( m_lameGlobalFlags, STEREO );
	lame_set_VBR( m_lameGlobalFlags, vbr_off );
	if( lame_init_params( m_lameGlobalFlags ) < 0 )
		return 0;
	m_encoderThread = std::thread( encodeThread, this );
	m_encoderThread.detach();
	return desiredFrq;
}

void MP3AudioOutput::setID3( const std::string& title, const std::string& album, const std::string& artist ) {
	id3tag_init( m_lameGlobalFlags );
	id3tag_add_v2( m_lameGlobalFlags );
	id3tag_set_title( m_lameGlobalFlags, title.c_str() );
	id3tag_set_artist( m_lameGlobalFlags, artist.c_str() );
	id3tag_set_album( m_lameGlobalFlags, album.c_str() );
}