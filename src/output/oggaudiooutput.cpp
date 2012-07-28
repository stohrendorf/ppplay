/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "oggaudiooutput.h"
#include "stream/filestream.h"

#include <vorbis/vorbisenc.h>
#include <boost/format.hpp>


void OggAudioOutput::encodeThread()
{
	bool endOfStream = false;
	while( AbstractAudioSource::Ptr srcLock = source().lock() ) {
		if(endOfStream) {
			vorbis_analysis_wrote(m_ds, 0);
			setErrorCode( InputDry );
			pause();
			break;
		}
		boost::mutex::scoped_lock lock( m_mutex );
		if( m_paused || srcLock->paused() ) {
			boost::this_thread::sleep( boost::posix_time::millisec( 10 ) );
			m_thread.yield();
			continue;
		}
		AudioFrameBuffer buffer;
		size_t size = srcLock->getAudioData( buffer, srcLock->preferredBufferSize() );
		if( size == 0 || !buffer || buffer->empty() ) {
			vorbis_analysis_wrote(m_ds, 0);
			setErrorCode( InputDry );
			pause();
			return;
		}
		float** analysis = vorbis_analysis_buffer(m_ds, size);
		for(size_t i=0; i<size; i++) {
			analysis[0][i] = (*buffer)[i].left /32768.0;
			analysis[1][i] = (*buffer)[i].right/32768.0;
		}
		vorbis_analysis_wrote(m_ds, size);
		while(vorbis_analysis_blockout(m_ds, m_vb)) {
			vorbis_analysis(m_vb, nullptr);
			vorbis_bitrate_addblock(m_vb);
			ogg_packet op;
			while(vorbis_bitrate_flushpacket(m_ds, &op)) {
				ogg_stream_packetin(m_os, &op);
				while(ogg_stream_pageout(m_os, m_op)) {
					m_stream->write(m_op->header, m_op->header_len).write(m_op->body, m_op->body_len);
					if(ogg_page_eos(m_op)) {
						endOfStream = true;
						break;
					}
				}
			}
		}
	}
}

OggAudioOutput::OggAudioOutput( const AbstractAudioSource::WeakPtr& src, const std::string& filename ): AbstractAudioOutput( src ),
	m_filename( filename ), m_paused( true ), m_mutex(), m_vi(nullptr), m_ds(nullptr), m_vb(nullptr),
	m_os(nullptr), m_op(nullptr), m_stream(nullptr), m_title(), m_artist(), m_album(), m_thread()
{
	logger()->info( L4CXX_LOCATION, "Created output: Filename '%s'", filename );
}

OggAudioOutput::~OggAudioOutput()
{
	boost::mutex::scoped_lock lock( m_mutex );
	if(m_os) {
		ogg_stream_clear(m_os);
		delete m_os;
	}
	if(m_vb) {
		vorbis_block_clear(m_vb);
		delete m_vb;
	}
	if(m_ds) {
		vorbis_dsp_clear(m_ds);
		delete m_ds;
	}
	if(m_vi) {
		vorbis_info_clear(m_vi);
		delete m_vi;
	}
	delete m_op;
	delete m_stream;
	logger()->trace( L4CXX_LOCATION, "Destroyed" );
}

uint16_t OggAudioOutput::internal_volumeRight() const
{
	return 0;
}

uint16_t OggAudioOutput::internal_volumeLeft() const
{
	return 0;
}

void OggAudioOutput::internal_pause()
{
	m_paused = true;
}

void OggAudioOutput::internal_play()
{
	m_paused = false;
}

bool OggAudioOutput::internal_paused() const
{
	return m_paused;
}

bool OggAudioOutput::internal_playing() const
{
	return !m_paused;
}

int OggAudioOutput::internal_init( int desiredFrq )
{
	logger()->trace( L4CXX_LOCATION, "Initializing OGG Vorbis" );
	m_stream = new FileStream(m_filename, FileStream::Mode::Write);
	if(!static_cast<FileStream*>(m_stream)->isOpen()) {
		logger()->error(L4CXX_LOCATION, "Cannot open output file");
		setErrorCode(OutputUnavailable);
		return 0;
	}
	m_vi = new vorbis_info;
	vorbis_info_init(m_vi);
	if(vorbis_encode_init_vbr(m_vi, 2, desiredFrq, 0.5)) {
		logger()->error(L4CXX_LOCATION, "Failed to init encoder");
		setErrorCode(OutputError);
		return 0;
	}
	m_ds = new vorbis_dsp_state;
	vorbis_analysis_init(m_ds, m_vi);
	m_vb = new vorbis_block;
	vorbis_block_init(m_ds,m_vb);
	m_os = new ogg_stream_state;
	ogg_stream_init(m_os, rand());
	m_op = new ogg_page;
	{
		vorbis_comment comments;
		vorbis_comment_init(&comments);
		vorbis_comment_add_tag( &comments, "title", m_title.c_str() );
		vorbis_comment_add_tag( &comments, "artist", m_artist.c_str() );
		vorbis_comment_add_tag( &comments, "album", m_album.c_str() );
		vorbis_comment_add_tag( &comments, "encoder", "PeePeePlayer" );
		
		ogg_packet hdrMain, hdrComments, hdrCodebooks;
		vorbis_analysis_headerout(m_ds, &comments, &hdrMain, &hdrComments, &hdrCodebooks);
		ogg_stream_packetin(m_os, &hdrMain);
		ogg_stream_packetin(m_os, &hdrComments);
		ogg_stream_packetin(m_os, &hdrCodebooks);
		while(ogg_stream_flush(m_os, m_op)) {
			m_stream->write(m_op->header, m_op->header_len).write(m_op->body, m_op->body_len);
		}
		
		vorbis_comment_clear(&comments);
	}
	
	m_thread = boost::thread(&OggAudioOutput::encodeThread, this);
	
	logger()->trace( L4CXX_LOCATION, "OGG initialized" );
	return desiredFrq;
}

void OggAudioOutput::setMeta( const std::string& title, const std::string& album, const std::string& artist )
{
	m_title = title;
	m_album = album;
	m_artist = artist;
}

light4cxx::Logger* OggAudioOutput::logger()
{
	return light4cxx::Logger::get( AbstractAudioOutput::logger()->name() + ".ogg" );
}
