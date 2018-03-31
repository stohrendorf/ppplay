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

#include "wavaudiooutput.h"

#include <boost/format.hpp>
#include <chrono>

void WavAudioOutput::encodeThread()
{
    while(AbstractAudioSource::Ptr lockedSrc = source())
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!m_file.is_open() || !m_file)
        {
            break;
        }
        if(m_paused || lockedSrc->paused())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::this_thread::yield();
            continue;
        }
        AudioFrameBufferPtr buffer;
        size_t size = lockedSrc->getAudioData(buffer, lockedSrc->preferredBufferSize());
        if(size == 0 || !buffer || buffer->empty())
        {
            setErrorCode(InputDry);
            pause();
            return;
        }
        m_file.write(reinterpret_cast<const char*>(&buffer->front()), buffer->size() * sizeof(BasicSampleFrame));
    }
}

WavAudioOutput::WavAudioOutput(const AbstractAudioSource::WeakPtr& src, const std::string& filename) : AbstractAudioOutput(src),
m_file(), m_filename(filename), m_encoderThread(), m_paused(true), m_mutex()
{
    logger()->info(L4CXX_LOCATION, "Created output: Filename '%s'", filename);
}

WavAudioOutput::~WavAudioOutput()
{
    m_encoderThread.join();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t filesize = m_file.tellp();

    // RIFF chunk size
    uint32_t tmp = filesize - 8;
    m_file.seekp(4);
    m_file.write(reinterpret_cast<const char*>(&tmp), 4);

    // data chunk size
    tmp = filesize - 44;
    m_file.seekp(40);
    m_file.write(reinterpret_cast<const char*>(&tmp), 4);

    logger()->trace(L4CXX_LOCATION, "Destroyed");
}

uint16_t WavAudioOutput::internal_volumeRight() const
{
    return 0;
}

uint16_t WavAudioOutput::internal_volumeLeft() const
{
    return 0;
}

void WavAudioOutput::internal_pause()
{
    m_paused = true;
}

void WavAudioOutput::internal_play()
{
    m_paused = false;
}

bool WavAudioOutput::internal_paused() const
{
    return m_paused;
}

bool WavAudioOutput::internal_playing() const
{
    return !m_paused;
}

int WavAudioOutput::internal_init(int desiredFrq)
{
    m_file.open(m_filename, std::ios::in);
    if(m_file.is_open())
    {
        m_file.close();
        logger()->error(L4CXX_LOCATION, "Output file already exists: '%s'", m_filename);
        setErrorCode(OutputUnavailable);
        return 0;
    }
    m_file.open(m_filename, std::ios::out | std::ios::binary);
    if(!m_file.is_open())
    {
        logger()->error(L4CXX_LOCATION, "Cannot open output file for writing: '%s'", m_filename);
        setErrorCode(OutputUnavailable);
        return 0;
    }

    struct
    {
        char id1[4];
        int32_t size1;
        char id2[4];
        char id3[4];
        int32_t subsize1;
        int16_t format, numChans;
        uint32_t sampleRate, byteRate;
        int16_t blockAlign, bitsPerSample;
        char id4[4];
        int32_t subsize2;
    } header = {
        {'R', 'I', 'F', 'F'}, 0,
        {'W', 'A', 'V', 'E'}, {'f', 'm', 't', ' '}, 16, 1, 2, static_cast<uint32_t>(desiredFrq), uint32_t(desiredFrq * sizeof(BasicSampleFrame)), sizeof(BasicSampleFrame), 16,
        {'d', 'a', 't', 'a'}, 0
    };
    m_file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    /*
    // ChunkID
    m_file.write("RIFF", 4);
    // ChunkSize (placeholder)
    m_file.write(&int32_t(0), 4);
    // Format, SubChunkID
    m_file.write("WAVEfmt ", 8);
    m_file
    << int32_t(16) // SubChunk size
    << int16_t(1) // Audio Format = PCM
    << int16_t(2) // NumChannels
    << int32_t(desiredFrq) // SampleRate
    << int32_t(desiredFrq*sizeof(BasicSampleFrame)) // ByteRate
    << int16_t(sizeof(BasicSampleFrame)) // BlockAlign
    << int16_t(16); // BitsPerSample
    m_file.write("data", 4); // SubChunk ID
    m_file << int32_t(0); // SubChunk size (placeholder)
    */

    m_encoderThread = std::thread(&WavAudioOutput::encodeThread, this);
    return desiredFrq;
}

light4cxx::Logger* WavAudioOutput::logger()
{
    return light4cxx::Logger::get(AbstractAudioOutput::logger()->name() + ".wav");
}