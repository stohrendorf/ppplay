/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "genchannel.h"

/**
* @file
* @ingroup GenMod
* @brief General channel definitions
*/

using namespace ppp;

GenChannel::GenChannel( const uint16_t frq ) throw( PppException ) :
		m_active( false ), m_disabled( true ), m_vibrato(), m_tremolo(), m_panning( 0x40 ),
		m_volume( 0 ), m_tick( 0 ), m_position( 0 ),
		m_statusString(), m_playbackFrequency( frq ),
		m_statusStringMutex()
{
}

GenChannel::~GenChannel() throw() {
}

IArchive& GenChannel::serialize(IArchive* data) {
	*data & m_active & m_disabled & m_volume & m_panning & m_position & m_tick;
	data->archive(&m_tremolo).archive(&m_vibrato);
	return *data;
}

std::string GenChannel::getStatus() throw() {
	std::lock_guard<std::mutex> lock(m_statusStringMutex);
	return m_statusString;
}

void GenChannel::setStatusString(const std::string& s) {
	std::lock_guard<std::mutex> lock(m_statusStringMutex);
	m_statusString = s;
}
