/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "modcell.h"

#include "modbase.h"

#include "genmod/genbase.h"
#include "stream/abstractarchive.h"

#include <boost/format.hpp>

namespace ppp
{
namespace mod
{

ModCell::ModCell() : m_sampleNumber( 0 ), m_period( 0 ), m_effect( 0 ), m_effectValue( 0 ), m_note( "---" )
{
}

void ModCell::reset()
{
	m_sampleNumber = 0;
	m_period = 0;
	m_effect = 0;
	m_effectValue = 0;
	m_note = "---";
}

bool ModCell::load( Stream* str )
{
	clear();
	uint8_t tmp;
	*str >> tmp;
	m_sampleNumber = tmp & 0xf0;
	if( m_sampleNumber > 32 ) {
		// logger()->error( L4CXX_LOCATION, "Sample out of range: %d", int(m_sampleNumber) );
		return false;
	}
	//m_sampleNumber &= 0x1f;
	m_period = ( tmp & 0x0f ) << 8;
	*str >> tmp;
	m_period |= tmp;
	m_period &= 0xfff;
	*str >> tmp;
	m_sampleNumber |= tmp >> 4;
	m_effect = tmp & 0x0f;
	*str >> tmp;
	m_effectValue = tmp;
	if( m_period != 0 ) {
		uint8_t idx = periodToNoteIndex( m_period );
		if( idx != 255 ) {
			m_note = stringFmt( "%s%u", NoteNames[idx % 12], idx / 12 );
		}
		else {
			logger()->warn( L4CXX_LOCATION, "Period %d too low: Cannot find matching note name.", m_period );
			m_note = "^^^";
		}
	}
	return str->good();
}

void ModCell::clear()
{
	m_sampleNumber = 0;
	m_period = 0;
	m_effect = 0;
	m_effectValue = 0;
	m_note.assign( "---" );
}

uint8_t ModCell::sampleNumber() const
{
	return m_sampleNumber;
}

uint16_t ModCell::period() const
{
	return m_period;
}

uint8_t ModCell::effect() const
{
	return m_effect;
}

uint8_t ModCell::effectValue() const
{
	return m_effectValue;
}

std::string ModCell::trackerString() const
{
	std::string res( m_note );
	if( m_sampleNumber != 0 ) {
		res.append( stringFmt( " %02u", int(m_sampleNumber) ) );
	}
	else {
		res.append( " --" );
	}
	if( m_effect == 0 && m_effectValue == 0 ) {
		res.append( " ---" );
	}
	else {
		res.append( stringFmt( " %01X%02X", int(m_effect), int(m_effectValue) ) );
	}
	return res;
}

AbstractArchive& ModCell::serialize( AbstractArchive* data )
{
	*data % m_sampleNumber % m_period % m_effect % m_effectValue;
	if( data->isLoading() ) {
		if( m_period != 0 ) {
			uint8_t idx = periodToNoteIndex( m_period );
			if( idx != 255 ) {
				m_note = stringFmt( "%s%u", NoteNames[idx % 12] ,idx / 12 );
			}
			else {
				m_note = "^^^";
			}
		}
		else {
			m_note = "---";
		}
	}
	return *data;
}

light4cxx::Logger* ModCell::logger()
{
	return light4cxx::Logger::get( IPatternCell::logger()->name() + ".mod" );
}


}
}
