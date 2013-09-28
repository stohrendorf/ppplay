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

#ifndef OGGAUDIOOUTPUT_H
#define OGGAUDIOOUTPUT_H

#include "abstractaudiooutput.h"

#include "stream/stream.h"
#include <vorbis/codec.h>

/**
 * @ingroup Output
 * @{
 */

class OggAudioOutput : public AbstractAudioOutput
{
	DISABLE_COPY( OggAudioOutput )
	OggAudioOutput() = delete;
private:
	std::string m_filename;
	//! @brief Whether the output is paused
	bool m_paused;
	mutable std::mutex m_mutex;

	vorbis_info* m_vi;
	vorbis_dsp_state* m_ds;
	vorbis_block* m_vb;
	ogg_stream_state* m_os;
	ogg_page* m_op;
	
	Stream* m_stream;
	std::string m_title;
	std::string m_artist;
	std::string m_album;
	std::thread m_thread;
	
	void encodeThread();
	
	virtual uint16_t internal_volumeRight() const;
	virtual uint16_t internal_volumeLeft() const;
	virtual void internal_pause();
	virtual void internal_play();
	virtual bool internal_paused() const;
	virtual bool internal_playing() const;
	virtual int internal_init( int desiredFrq );
public:
	/**
	 * @brief Constructor
	 * @param[in] src Source of audio data
	 * @param[in] filename Output filename of the MP3 data
	 */
	explicit OggAudioOutput( const AbstractAudioSource::WeakPtr& src, const std::string& filename );
	virtual ~OggAudioOutput();
	/**
	 * @brief Set the meta tags of the output file
	 * @param[in] title Title tag
	 * @param[in] album Album tag
	 * @param[in] artist Artist tag
	 * @pre Should be called before init(int).
	 */
	void setMeta( const std::string& title, const std::string& album, const std::string& artist );
protected:
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".mp3"
	 */
	static light4cxx::Logger* logger();
};

/**
 * @}
 */

#endif // MP3AUDIOOUTPUT_H
