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

#include "abstractaudiosource.h"

AbstractAudioSource::AbstractAudioSource() noexcept :
m_initialized(false), m_frequency(0), m_paused(false), m_mutex()
{
}

bool AbstractAudioSource::initialized() const noexcept
{
    return m_initialized;
}

bool AbstractAudioSource::fail()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_initialized = false;
    m_frequency = 0;
    return false;
}

uint32_t AbstractAudioSource::frequency() const noexcept
{
    return m_frequency;
}

bool AbstractAudioSource::initialize(uint32_t frequency)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_frequency = frequency;
    bool res = internal_initialize(frequency);
    if(res)
    {
        m_frequency = frequency;
        m_initialized = true;
    }
    else
    {
        m_frequency = 0;
        m_initialized = false;
    }
    return res;
}

uint16_t AbstractAudioSource::internal_volumeLeft() const
{
    return 0;
}

uint16_t AbstractAudioSource::volumeLeft() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return internal_volumeLeft();
}

uint16_t AbstractAudioSource::internal_volumeRight() const
{
    return 0;
}

uint16_t AbstractAudioSource::volumeRight() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return internal_volumeRight();
}

size_t AbstractAudioSource::internal_preferredBufferSize() const
{
    return 0;
}

size_t AbstractAudioSource::preferredBufferSize() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return internal_preferredBufferSize();
}

size_t AbstractAudioSource::getAudioData(AudioFrameBuffer& buffer, size_t requestedFrames)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return internal_getAudioData(buffer, requestedFrames);
}

light4cxx::Logger* AbstractAudioSource::logger()
{
    return light4cxx::Logger::get("audio.source");
}