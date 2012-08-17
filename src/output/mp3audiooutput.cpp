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

#include "mp3audiooutput.h"

#include <lame/lame.h>
#include <boost/format.hpp>

void MP3AudioOutput::encodeThread()
{
	while( AbstractAudioSource::Ptr lockedSrc = source() ) {
		boost::mutex::scoped_lock lock( m_mutex );
		if( m_paused || lockedSrc->paused() ) {
			boost::this_thread::sleep( boost::posix_time::millisec( 10 ) );
			m_encoderThread.yield();
			continue;
		}
		AudioFrameBuffer buffer;
		size_t size = lockedSrc->getAudioData( buffer, lockedSrc->preferredBufferSize() );
		if( size == 0 || !buffer || buffer->empty() ) {
			setErrorCode( InputDry );
			pause();
			return;
		}
		int res = lame_encode_buffer_interleaved( m_lameGlobalFlags, &buffer->front().left, buffer->size(), m_buffer, BufferSize );
		if( res < 0 ) {
			switch(res) {
				case -1:
					logger()->error( L4CXX_LOCATION, "Encoding Buffer too small" );
					break;
				case -2:
					logger()->error( L4CXX_LOCATION, "malloc() problem" );
					break;
				case -3:
					logger()->error( L4CXX_LOCATION, "Missing lame_init_params() call" );
					break;
				case -4:
					logger()->error( L4CXX_LOCATION, "Psycho acoustic problem" );
					break;
				default:
					logger()->error( L4CXX_LOCATION, "Unknown error: %d", res );
			}
			pause();
			setErrorCode(OutputError);
			break;
		}
		m_file.write( reinterpret_cast<char*>( m_buffer ), res );
	}
}


MP3AudioOutput::MP3AudioOutput( const AbstractAudioSource::WeakPtr& src, const std::string& filename ): AbstractAudioOutput( src ),
	m_lameGlobalFlags( nullptr ), m_file(), m_filename( filename ), m_buffer( nullptr ), m_encoderThread(), m_paused( true ), m_mutex()
{
	m_buffer = new uint8_t[BufferSize];
	m_lameGlobalFlags = lame_init();
	logger()->info( L4CXX_LOCATION, "Created output: Filename '%s'", filename );
}

MP3AudioOutput::~MP3AudioOutput()
{
	m_encoderThread.join();
	boost::mutex::scoped_lock lock( m_mutex );
	if( m_lameGlobalFlags != nullptr ) {
		int size = lame_encode_flush(m_lameGlobalFlags, m_buffer, BufferSize);
		m_file.write( reinterpret_cast<char*>(m_buffer), size );
		lame_close( m_lameGlobalFlags );
	}
	delete[] m_buffer;
	m_buffer = nullptr;
	logger()->trace( L4CXX_LOCATION, "Destroyed" );
}

uint16_t MP3AudioOutput::internal_volumeRight() const
{
	return 0;
}

uint16_t MP3AudioOutput::internal_volumeLeft() const
{
	return 0;
}

void MP3AudioOutput::internal_pause()
{
	m_paused = true;
}

void MP3AudioOutput::internal_play()
{
	m_paused = false;
}

bool MP3AudioOutput::internal_paused() const
{
	return m_paused;
}

bool MP3AudioOutput::internal_playing() const
{
	return !m_paused;
}

int MP3AudioOutput::internal_init( int desiredFrq )
{
	AbstractAudioSource::Ptr lockedSrc = source();
	if(!lockedSrc) {
		return 0;
	}
	logger()->trace( L4CXX_LOCATION, "Initializing LAME" );
	logger()->debug( L4CXX_LOCATION, "Using LAME %s, Bitness %s, PSY version %s", get_lame_version(), get_lame_os_bitness(), get_psy_version() );
	m_file.open( m_filename, std::ios::in );
	if( m_file.is_open() ) {
		m_file.close();
		logger()->error( L4CXX_LOCATION, "Output file already exists: '%s'", m_filename );
		setErrorCode( OutputUnavailable );
		return 0;
	}
	m_file.open( m_filename, std::ios::out | std::ios::binary );
	if( !m_file.is_open() ) {
		logger()->error( L4CXX_LOCATION, "Cannot open output file for writing: '%s'", m_filename );
		setErrorCode( OutputUnavailable );
		return 0;
	}
	lame_set_out_samplerate( m_lameGlobalFlags, desiredFrq );
	lame_set_in_samplerate( m_lameGlobalFlags, lockedSrc->frequency() );
	lame_set_num_channels( m_lameGlobalFlags, 2 );
	lame_set_quality( m_lameGlobalFlags, 5 );
	lame_set_mode( m_lameGlobalFlags, STEREO );
	lame_set_VBR( m_lameGlobalFlags, vbr_default );
	if( lame_init_params( m_lameGlobalFlags ) < 0 ) {
		logger()->error( L4CXX_LOCATION, "LAME parameter initialization failed" );
		return 0;
	}
	m_encoderThread = boost::thread( boost::bind( &MP3AudioOutput::encodeThread, this ) );
	logger()->trace( L4CXX_LOCATION, "LAME initialized" );
	return desiredFrq;
}

void MP3AudioOutput::setID3( const std::string& title, const std::string& album, const std::string& artist )
{
	if( m_file.is_open() ) {
		return;
	}
	id3tag_init( m_lameGlobalFlags );
	id3tag_add_v2( m_lameGlobalFlags );
	id3tag_set_title( m_lameGlobalFlags, title.c_str() );
	id3tag_set_artist( m_lameGlobalFlags, artist.c_str() );
	id3tag_set_album( m_lameGlobalFlags, album.c_str() );
}

light4cxx::Logger* MP3AudioOutput::logger()
{
	return light4cxx::Logger::get( AbstractAudioOutput::logger()->name() + ".mp3" );
}
