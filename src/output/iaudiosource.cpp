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

IAudioSource::IAudioSource() : m_initialized( false ), m_frequency( 0 ), m_lockMutex(), m_isBusy( false )
{
}

IAudioSource::~IAudioSource() = default;

bool IAudioSource::initialized() const
{
	return m_initialized;
}

bool IAudioSource::fail()
{
	m_initialized = false;
	m_frequency = 0;
	return false;
}

bool IAudioSource::initialize( uint32_t frequency )
{
	m_frequency = frequency;
	m_initialized = true;
	return true;
}

uint32_t IAudioSource::frequency() const
{
	return m_frequency;
}

uint16_t IAudioSource::volumeLeft() const
{
	return 0;
}

uint16_t IAudioSource::volumeRight() const
{
	return 0;
}

size_t IAudioSource::preferredBufferSize() const
{
	return 0;
}

void IAudioSource::setBusy( bool value )
{
	m_isBusy = value;
}

bool IAudioSource::isBusy() const
{
	return m_isBusy;
}

light4cxx::Logger::Ptr IAudioSource::logger()
{
	return light4cxx::Logger::get( "audio.source" );
}
