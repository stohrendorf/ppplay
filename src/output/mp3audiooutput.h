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

#ifndef MP3AUDIOOUTPUT_H
#define MP3AUDIOOUTPUT_H

#include "iaudiooutput.h"

#include <thread>
#include <fstream>

class MP3AudioOutput : public IAudioOutput {
		DISABLE_COPY( MP3AudioOutput )
		MP3AudioOutput() = delete;
	private:
		struct lame_global_struct* m_lameGlobalFlags;
		std::ofstream m_file;
		std::string m_filename;
		uint8_t* m_buffer;
		std::thread m_encoderThread;
		std::mutex m_bufferMutex;
		bool m_paused;
		static const std::size_t BufferSize = 4096;
		static void encodeThread( MP3AudioOutput* src );
	public:
		explicit MP3AudioOutput( const IAudioSource::WeakPtr& src, const std::string& filename );
		virtual ~MP3AudioOutput();
		virtual uint16_t volumeRight() const;
		virtual uint16_t volumeLeft() const;
		virtual void pause();
		virtual void play();
		virtual bool paused();
		virtual bool playing();
		virtual int init( int desiredFrq );
		void setID3( const std::string& title, const std::string& album, const std::string& artist );
};

#endif // MP3AUDIOOUTPUT_H
