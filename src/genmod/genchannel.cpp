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

#include "genchannel.h"
#include "stream/iarchive.h"

namespace ppp
{

GenChannel::GenChannel() :
	m_active( false ), m_disabled( true ),
	m_position( GenSample::EndOfSample ),
	m_statusString(),
	m_mutex()
{
}

GenChannel::~GenChannel() = default;

IArchive& GenChannel::serialize( IArchive* data )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	*data % m_active % m_disabled % m_position;
	return *data;
}

std::string GenChannel::statusString()
{
	boost::recursive_mutex::scoped_lock lock( m_mutex );
	return m_statusString;
}

void GenChannel::setStatusString( const std::string& s )
{
	boost::recursive_mutex::scoped_lock lock( m_mutex );
	m_statusString = s;
}

void GenChannel::setActive( bool a )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_active = a;
}

void GenChannel::setPosition( GenSample::PositionType p )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_position = p;
}

void GenChannel::enable()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_disabled = false;
}

void GenChannel::disable()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_disabled = true;
}

GenSample::PositionType GenChannel::position() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_position;
}

bool GenChannel::isDisabled() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_disabled;
}

bool GenChannel::isActive() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_active;
}

std::string GenChannel::cellString()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_cellString();
}

std::string GenChannel::effectDescription() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_effectDescription();
}

std::string GenChannel::effectName() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_effectName();
}

void GenChannel::mixTick( MixerFrameBuffer* mixBuffer )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	internal_mixTick(mixBuffer);
}

std::string GenChannel::noteName()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return internal_noteName();
}

void GenChannel::updateStatus()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	internal_updateStatus();
}

light4cxx::Logger::Ptr GenChannel::logger()
{
	return light4cxx::Logger::get( "channel" );
}

}
