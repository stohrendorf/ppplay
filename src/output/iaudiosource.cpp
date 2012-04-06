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

#include "iaudiosource.h"

IAudioSource::IAudioSource() : m_initialized( false ), m_frequency( 0 ), m_paused(false), m_mutex()
{
}

IAudioSource::~IAudioSource() = default;

bool IAudioSource::initialized() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_initialized;
}

bool IAudioSource::fail()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_initialized = false;
	m_frequency = 0;
	return false;
}

uint32_t IAudioSource::frequency() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_frequency;
}

bool IAudioSource::initialize( uint32_t frequency )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_frequency = frequency;
	bool res = internal_initialize(frequency);
	if( res ) {
		m_frequency = frequency;
		m_initialized = true;
	}
	else {
		m_frequency = 0;
		m_initialized = false;
	}
	return res;
}


uint16_t IAudioSource::internal_volumeLeft() const
{
	return 0;
}

uint16_t IAudioSource::volumeLeft() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_volumeLeft();
}

uint16_t IAudioSource::internal_volumeRight() const
{
	return 0;
}

uint16_t IAudioSource::volumeRight() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_volumeRight();
}

size_t IAudioSource::internal_preferredBufferSize() const
{
	return 0;
}

size_t IAudioSource::preferredBufferSize() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_preferredBufferSize();
}

size_t IAudioSource::getAudioData( AudioFrameBuffer& buffer, size_t requestedFrames )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_getAudioData(buffer,requestedFrames);
}

light4cxx::Logger::Ptr IAudioSource::logger()
{
	return light4cxx::Logger::get( "audio.source" );
}
