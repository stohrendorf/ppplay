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

#include "s3mpattern.h"
#include "genpattern.h"

/**
* @file
* @brief S3M Pattern class
* @ingroup S3mMod
*/

using namespace ppp;
using namespace ppp::s3m;

S3mCell::S3mCell() throw() : GenCell(), m_note( s3mEmptyNote ), m_instr( s3mEmptyInstr ), m_volume( s3mEmptyVolume ),
		m_effect( s3mEmptyCommand ), m_effectValue( 0x00 ) {
}

S3mCell::~S3mCell() throw() {
}

bool S3mCell::load( BinStream &str ) throw( PppException ) {
	try {
		reset();
		uint8_t master = 0;
		uint8_t buf;
		str.read( &master );
		setActive( true );
		if ( master&0x20 ) {
			str.read( &buf );
			m_note = buf;
			if (( m_note >= 0x9b ) && ( m_note != s3mEmptyNote ) && ( m_note != s3mKeyOffNote ) ) {
				LOG_WARNING( "File Position %.8x: Note out of range: %.2x", str.pos(), m_note );
				m_note = s3mEmptyNote;
			}
			str.read( &buf );
			m_instr = buf;
		}
		if ( master&0x40 ) {
			str.read( &buf );
			m_volume = buf;
			if ( buf > 0x40 ) {
				LOG_WARNING( "File Position %.8x: Volume out of range: %d", str.pos(), m_volume );
				m_volume = s3mEmptyVolume;
			}
		}
		if ( master&0x80 ) {
			str.read( &buf );
			m_effect = buf;
			str.read( &buf );
			m_effectValue = buf;
		}
	}
	catch ( ... ) {
		LOG_ERROR_( "EXCEPTION" );
		setActive( false );
		return false;
	}
	return true;
}

void S3mCell::reset() throw() {
	GenCell::reset();
	m_note = s3mEmptyNote;
	m_instr = s3mEmptyInstr;
	m_volume = s3mEmptyVolume;
	m_effect = s3mEmptyCommand;
	m_effectValue = 0x00;
}

std::string S3mCell::trackerString() const throw() {
	if ( !isActive() )
		return "... .. .. ...";
	std::string xmsg = "";
	if ( m_note == s3mEmptyNote )
		xmsg += "... ";
	else if ( m_note == s3mKeyOffNote )
		xmsg += "^^  ";
	else
		xmsg += stringf( "%s%d ", NoteNames[m_note&0x0f], m_note >> 4 );
	if ( m_instr != s3mEmptyInstr )
		xmsg += stringf( "%.2d ", m_instr );
	else
		xmsg += ".. ";
	if ( m_volume != s3mEmptyVolume )
		xmsg += stringf( "%.2d ", m_volume );
	else
		xmsg += ".. ";
	if ( m_effect != s3mEmptyCommand )
		xmsg += stringf( "%c%.2x", 'A' -1 + m_effect, m_effectValue );
	else
		xmsg += "...";
	return xmsg;
}

S3mCell &S3mCell::operator=( const S3mCell & src ) throw() {
	GenCell::operator=( src );
	m_note = src.m_note;
	m_instr = src.m_instr;
	m_volume = src.m_volume;
	m_effect = src.m_effect;
	m_effectValue = src.m_effectValue;
	return *this;
}

uint8_t S3mCell::getNote() const throw() {
	return m_note;
}

uint8_t S3mCell::getInstr() const throw() {
	return m_instr;
}

uint8_t S3mCell::getVolume() const throw() {
	return m_volume;
}

uint8_t S3mCell::getEffect() const throw() {
	return m_effect;
}

uint8_t S3mCell::getEffectValue() const throw() {
	return m_effectValue;
}


S3mPattern::S3mPattern() throw( PppException ) : GenPattern() {
	try {
		for ( uint8_t i = 0; i < 32; i++ ) {
			GenCell::Vector track;
			for ( uint8_t i = 0; i < 64; i++ )
				track.push_back( GenCell::Ptr() );
			addTrack( track );
		}
	}
	catch ( ... ) {
		PPP_THROW( "Unknown Exception" );
	}
}

S3mPattern::~S3mPattern() throw() {
}

GenCell::Ptr S3mPattern::createCell( int16_t trackIndex, int16_t row ) throw( PppException ) {
	PPP_TEST(( row < 0 ) || ( row > 63 ) );
	GenCell::Vector* track = getTrack( trackIndex );
	PPP_TEST( !track );
	GenCell::Ptr cell = track->at(row);
	if ( cell )
		return cell;
	track->at(row).reset( new S3mCell() );
	return track->at(row);
}

bool S3mPattern::load( BinStream& str, std::size_t pos ) throw( PppException ) {
	LOG_BEGIN();
	try {
		uint16_t patSize;
		str.seek( pos );
		str.read( &patSize );
		uint16_t currRow = 0, currTrack = 0;
		while ( currRow < 64 ) {
			uint8_t master;
			str.read( &master );
			if ( master == 0 ) {
				currRow++;
				continue;
			}
			currTrack = master & 31;
			str.seekrel( -1 );
			if ( str.fail() ) {
				LOG_ERROR_( "str.fail()..." );
				return false;
			}
			GenCell::Ptr cell = createCell( currTrack, currRow );
			if ( !cell->load( str ) ) {
				LOG_ERROR_( "Cell loading: ERROR" );
				return false;
			}
		}
		return true;
	}
	PPP_RETHROW()
	catch ( ... ) {
		PPP_THROW( "Unknown Exception" );
	}
}
