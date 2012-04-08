/*
    PeePeePlayer - an old-fashioned module player
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

#include "songinfocontainer.h"
#include <stdexcept>

namespace ppp
{
	
SongInfoContainer::SongInfoContainer() :
	m_songs(),
	m_currentSongIndex( 0 ),
	m_initialState( new MemArchive() )
{
}

void SongInfoContainer::setIndex( uint16_t idx )
{
	if( idx > m_songs.size() ) {
		throw std::out_of_range("Invalid index passed to SongInfoContainer::setIndex()");
	}
	m_currentSongIndex = idx;
}

int16_t SongInfoContainer::index() const
{
	return m_currentSongIndex;
}

const SongInfo& SongInfoContainer::at( size_t idx ) const
{
	return m_songs.at( idx );
}

SongInfo& SongInfoContainer::at( size_t idx )
{
	return m_songs.at( idx );
}

const SongInfo& SongInfoContainer::current() const
{
	return m_songs.at( m_currentSongIndex );
}

SongInfo& SongInfoContainer::current()
{
	return m_songs.at( m_currentSongIndex );
}

void SongInfoContainer::append( const SongInfo& info )
{
	m_songs.push_back( info );
}

size_t SongInfoContainer::size() const
{
	return m_songs.size();
}

IArchive::Ptr SongInfoContainer::initialState() const
{
	return m_initialState;
}

void SongInfoContainer::removeEmptySongs()
{
	std::remove_if(
		m_songs.begin(),
		m_songs.end(),
	[]( const SongInfo & info ) {
		return info.length == 0;
	}
	);
	m_currentSongIndex = 0;
}

}
