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
 * @ingroup GenMod
 * @{
 */

#include "abstractchannel.h"
#include "stream/abstractarchive.h"

namespace ppp
{

AbstractChannel::AbstractChannel() :
	m_active( false ), m_disabled( true ),
	m_statusString(),
	m_mutex()
{
}

AbstractChannel::~AbstractChannel() = default;

AbstractArchive& AbstractChannel::serialize( AbstractArchive* data )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	*data % m_active % m_disabled;
	return *data;
}

std::string AbstractChannel::statusString() const
{
	boost::recursive_mutex::scoped_lock lock( m_mutex );
	return m_statusString;
}

void AbstractChannel::setStatusString( const std::string& s )
{
	boost::recursive_mutex::scoped_lock lock( m_mutex );
	m_statusString = s;
}

void AbstractChannel::setActive( bool a )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_active = a;
}

void AbstractChannel::enable()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_disabled = false;
}

void AbstractChannel::disable()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_disabled = true;
}

bool AbstractChannel::isDisabled() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_disabled;
}

bool AbstractChannel::isActive() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_active;
}

std::string AbstractChannel::cellString() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_cellString();
}

std::string AbstractChannel::effectDescription() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_effectDescription();
}

std::string AbstractChannel::effectName() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_effectName();
}

void AbstractChannel::mixTick( MixerFrameBuffer* mixBuffer )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	internal_mixTick(mixBuffer);
}

std::string AbstractChannel::noteName() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_noteName();
}

void AbstractChannel::updateStatus()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	internal_updateStatus();
}

light4cxx::Logger* AbstractChannel::logger()
{
	return light4cxx::Logger::get( "channel" );
}

}
