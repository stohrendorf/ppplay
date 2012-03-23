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

#include "iaudiooutput.h"

IAudioOutput::~IAudioOutput() = default;

IAudioOutput::ErrorCode IAudioOutput::errorCode() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_errorCode;
}

void IAudioOutput::setErrorCode( IAudioOutput::ErrorCode ec )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_errorCode = ec;
}

IAudioSource::WeakPtr IAudioOutput::source() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_source;
}

uint16_t IAudioOutput::volumeLeft() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if( !m_source.expired() ) {
		return m_source.lock()->volumeLeft();
	}
	return 0;
}

uint16_t IAudioOutput::volumeRight() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	IAudioSource::Ptr source(m_source.lock());
	if( !m_source.expired() ) {
		return source->volumeRight();
	}
	return 0;
}

light4cxx::Logger::Ptr IAudioOutput::logger()
{
	return light4cxx::Logger::get( "audio.output" );
}

/**
 * @}
 */
