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

GenChannel::GenChannel( const ppp::Frequency frq ) throw( PppException ) :
		m_active( false ), m_disabled( true ), m_vibrato( ProtrackerLookup, 256, 256 ), m_tremolo( ProtrackerLookup, 256, 256 ), m_panning( 0x40 ),
		m_volume( 0 ), m_tick( 0 ), m_position( 0 ),
		m_statusString( "" ), m_currSmpIndex( -1 ), m_currCell(), m_playbackFrequency( frq )
{
}

GenChannel::GenChannel( const GenChannel &src ) throw() :
		m_active( src.m_active ), m_disabled( src.m_disabled ), m_vibrato( src.m_vibrato ), m_tremolo( src.m_tremolo ),
		m_panning( src.m_panning ), m_volume( src.m_volume ), m_tick( src.m_tick ),
		m_position( src.m_position ), m_statusString( src.m_statusString ),
		m_currSmpIndex( src.m_currSmpIndex ), m_currCell( src.m_currCell ), m_playbackFrequency( src.m_playbackFrequency )
{
}

GenChannel &GenChannel::operator=( const GenChannel & src ) throw() {
	m_active = src.m_active;
	m_disabled = src.m_disabled;
	m_panning = src.m_panning;
	m_volume = src.m_volume;
	m_tick = src.m_tick;
	m_position = src.m_position;
	m_statusString = src.m_statusString;
	m_currSmpIndex = src.m_currSmpIndex;
	*m_currCell = *src.m_currCell;
	m_playbackFrequency = src.m_playbackFrequency;
	return *this;
}

GenChannel::~GenChannel() throw() {
}

BinStream &GenChannel::saveState( BinStream &str ) const throw( PppException ) {
	try {
		str.write( &m_active )
		.write( &m_disabled )
		.write( &m_volume )
		.write( &m_panning )
		.write( &m_currSmpIndex )
		.writeSerialisable( m_currCell.get() )
		.write( &m_position )
		.write( &m_tick )
		.writeSerialisable( &m_tremolo )
		.writeSerialisable( &m_vibrato );
	}
	PPP_CATCH_ALL();
	return str;
}

BinStream &GenChannel::restoreState( BinStream &str ) throw( PppException ) {
	try {
		str.read( &m_active )
		.read( &m_disabled )
		.read( &m_volume )
		.read( &m_panning )
		.read( &m_currSmpIndex )
		//! @warning THIS IS VERY DANGEROUS! Needs dynamic update...
		.readSerialisable( m_currCell.get() )
		.read( &m_position )
		.read( &m_tick )
		.readSerialisable( &m_tremolo )
		.readSerialisable( &m_vibrato );
	}
	PPP_CATCH_ALL();
	return str;
}
