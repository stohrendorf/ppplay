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

#ifndef MP3AUDIOOUTPUT_H
#define MP3AUDIOOUTPUT_H

/**
 * @ingroup Output
 * @{
 */

#include "iaudiooutput.h"

#define _GLIBCXX_USE_NANOSLEEP
#warning "Defining _GLIBCXX_USE_NANOSLEEP for enabling usage of std::sleep_for"
#include <thread>
#undef _GLIBCXX_USE_NANOSLEEP

#include <fstream>

/**
 * @class MP3AudioOutput
 * @brief MP3 writing IAudioOutput
 */
class MP3AudioOutput : public IAudioOutput {
		DISABLE_COPY(MP3AudioOutput)
		MP3AudioOutput() = delete;
	private:
		//! @brief Internal lame flags struct
		struct lame_global_struct* m_lameGlobalFlags;
		//! @brief Output file stream for the MP3 data
		std::ofstream m_file;
		//! @brief MP3 filename
		std::string m_filename;
		//! @brief Internally used buffer for encoding
		uint8_t* m_buffer;
		//! @brief Encoder thread holder
		std::thread m_encoderThread;
		//! @brief Mutex that locks m_buffer
		std::mutex m_bufferMutex;
		//! @brief Whether the output is paused
		bool m_paused;
		//! @brief Default size of m_buffer
		static const std::size_t BufferSize = 4096;
		/**
		 * @brief Encoder thread handler
		 * @param[in] src Audio output source
		 * @note Declared here to get access to private data members without need to declare it as a @c friend
		 */
		static void encodeThread(MP3AudioOutput* src);
	public:
		/**
		 * @brief Constructor
		 * @param[in] src Source of audio data
		 * @param[in] filename Output filename of the MP3 data
		 */
		explicit MP3AudioOutput(const IAudioSource::WeakPtr& src, const std::string& filename);
		virtual ~MP3AudioOutput();
		virtual uint16_t volumeRight() const;
		virtual uint16_t volumeLeft() const;
		virtual void pause();
		virtual void play();
		virtual bool paused();
		virtual bool playing();
		virtual int init(int desiredFrq);
		/**
		 * @brief Set the ID3 tags of the output file
		 * @param[in] title Title tag
		 * @param[in] album Album tag
		 * @param[in] artist Artist tag
		 * @pre Should be called before init(int).
		 */
		void setID3(const std::string& title, const std::string& album, const std::string& artist);
};

/**
 * @}
 */

#endif // MP3AUDIOOUTPUT_H
