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

class PPPLAY_OUTPUT_WAV_EXPORT WavAudioOutput
    : public AbstractAudioOutput
{
private:
    std::ofstream m_file;
    std::string m_filename;
    std::thread m_encoderThread;
    //! @brief Whether the output is paused
    bool m_paused;
    mutable std::mutex m_mutex;

    void encodeThread();

    uint16_t internal_volumeRight() const override;

    uint16_t internal_volumeLeft() const override;

    void internal_pause() override;

    void internal_play() override;

    bool internal_paused() const override;

    bool internal_playing() const override;

    int internal_init(int desiredFrq) override;

public:
    DISABLE_COPY(WavAudioOutput)

    WavAudioOutput() = delete;

    explicit WavAudioOutput(const AbstractAudioSource::WeakPtr& src, const std::string& filename);

    ~WavAudioOutput() override;

protected:
    static light4cxx::Logger* logger();
};

/**
 * @}
 */

#endif
