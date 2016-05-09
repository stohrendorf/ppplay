/*
    PPPlay - an old-fashioned module player
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

#ifndef WAVAUDIOOUTPUT_H
#define WAVAUDIOOUTPUT_H

#include "abstractaudiooutput.h"

#include "ppplay_output_wav_export.h"

#include <fstream>

/**
 * @ingroup Output
 * @{
 */

class PPPLAY_OUTPUT_WAV_EXPORT WavAudioOutput : public AbstractAudioOutput
{
    DISABLE_COPY( WavAudioOutput )
    WavAudioOutput() = delete;
private:
    std::ofstream m_file;
    std::string m_filename;
    std::thread m_encoderThread;
    //! @brief Whether the output is paused
    bool m_paused;
    mutable std::mutex m_mutex;
    void encodeThread();
    virtual uint16_t internal_volumeRight() const;
    virtual uint16_t internal_volumeLeft() const;
    virtual void internal_pause();
    virtual void internal_play();
    virtual bool internal_paused() const;
    virtual bool internal_playing() const;
    virtual int internal_init( int desiredFrq );
public:
    explicit WavAudioOutput( const AbstractAudioSource::WeakPtr& src, const std::string& filename );
    virtual ~WavAudioOutput();
protected:
    static light4cxx::Logger* logger();
};

/**
 * @}
 */

#endif
