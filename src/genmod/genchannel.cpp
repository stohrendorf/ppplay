/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#include "genchannel.h"

/**
* @file
* @ingroup GenMod
* @brief General channel definitions
*/

using namespace ppp;

GenChannel::GenChannel( const uint16_t frq ) throw( PppException ) :
	m_active( false ), m_disabled( true ), m_panning( 0x40 ),
	m_position( 0 ),
	m_statusString(), m_playbackFrequency( frq ),
	m_statusStringMutex() {
}

GenChannel::~GenChannel() throw() {
}

IArchive& GenChannel::serialize( IArchive* data ) {
	*data& m_active& m_disabled& m_panning& m_position;
	return *data;
}

std::string GenChannel::getStatus() throw() {
	std::lock_guard<std::mutex> lock( m_statusStringMutex );
	return m_statusString;
}

void GenChannel::setStatusString( const std::string& s ) {
	std::lock_guard<std::mutex> lock( m_statusStringMutex );
	m_statusString = s;
}

