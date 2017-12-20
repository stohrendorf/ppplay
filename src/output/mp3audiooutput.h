/*
    PPPlay - an old-fashioned module player
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

#include "abstractaudiooutput.h"
#include "ppplay_output_mp3_export.h"

#include <fstream>

/**
 * @ingroup Output
 * @{
 */

/**
 * @class MP3AudioOutput
 * @brief MP3 writing IAudioOutput
 */
class PPPLAY_OUTPUT_MP3_EXPORT MP3AudioOutput : public AbstractAudioOutput
{
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
    //! @brief Whether the output is paused
    bool m_paused;
    mutable std::mutex m_mutex;
    //! @brief Default size of m_buffer
    static constexpr size_t BufferSize = 32768;
    /**
     * @brief Encoder thread handler
     */
    void encodeThread();
    virtual uint16_t internal_volumeRight() const;
    virtual uint16_t internal_volumeLeft() const;
    virtual void internal_pause();
    virtual void internal_play();
    virtual bool internal_paused() const;
    virtual bool internal_playing() const;
    virtual int internal_init( int desiredFrq );
public:
    DISABLE_COPY( MP3AudioOutput )
    MP3AudioOutput() = delete;
    /**
     * @brief Constructor
     * @param[in] src Source of audio data
     * @param[in] filename Output filename of the MP3 data
     */
    explicit MP3AudioOutput( const AbstractAudioSource::WeakPtr& src, const std::string& filename );
    virtual ~MP3AudioOutput();
    /**
     * @brief Set the ID3 tags of the output file
     * @param[in] title Title tag
     * @param[in] album Album tag
     * @param[in] artist Artist tag
     * @pre Should be called before init(int).
     */
    void setID3( const std::string& title, const std::string& album, const std::string& artist );
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
