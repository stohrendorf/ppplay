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

/**
 * @ingroup Output
 * @{
 */

#include "abstractaudiooutput.h"

AbstractAudioOutput::~AbstractAudioOutput() = default;

AbstractAudioOutput::ErrorCode AbstractAudioOutput::errorCode() const
{
	boost::mutex::scoped_lock lock( m_mutex );
	return m_errorCode;
}

void AbstractAudioOutput::setErrorCode( AbstractAudioOutput::ErrorCode ec )
{
// 	boost::mutex::scoped_lock lock( m_mutex );
	m_errorCode = ec;
}

AbstractAudioSource::WeakPtr AbstractAudioOutput::source() const
{
// 	boost::mutex::scoped_lock lock( m_mutex );
	return m_source;
}

uint16_t AbstractAudioOutput::internal_volumeLeft() const
{
	if( AbstractAudioSource::Ptr src = m_source.lock() ) {
		return src->volumeLeft();
	}
	return 0;
}

uint16_t AbstractAudioOutput::internal_volumeRight() const
{
	AbstractAudioSource::Ptr source( m_source.lock() );
	if( !m_source.expired() ) {
		return source->volumeRight();
	}
	return 0;
}

int AbstractAudioOutput::init( int desiredFrq )
{
	boost::mutex::scoped_lock lock( m_mutex );
	return internal_init( desiredFrq );
}

void AbstractAudioOutput::pause()
{
	boost::mutex::scoped_lock lock( m_mutex );
	internal_pause();
}

bool AbstractAudioOutput::paused() const
{
	boost::mutex::scoped_lock lock( m_mutex );
	return internal_paused();
}

void AbstractAudioOutput::play()
{
	boost::mutex::scoped_lock lock( m_mutex );
	internal_play();
}

bool AbstractAudioOutput::playing() const
{
	boost::mutex::scoped_lock lock( m_mutex );
	return internal_playing();
}

uint16_t AbstractAudioOutput::volumeLeft() const
{
	boost::mutex::scoped_lock lock( m_mutex );
	return internal_volumeLeft();
}

uint16_t AbstractAudioOutput::volumeRight() const
{
	boost::mutex::scoped_lock lock( m_mutex );
	return internal_volumeRight();
}

light4cxx::Logger* AbstractAudioOutput::logger()
{
	return light4cxx::Logger::get( "audio.output" );
}

/**
 * @}
 */
