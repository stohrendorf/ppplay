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

/**
 * @ingroup XmModule
 * @{
 */

#include "xmcell.h"

#include "genmod/genbase.h"
#include "stream/iarchive.h"

namespace ppp
{
namespace xm
{

XmCell::XmCell() : IPatternCell(), m_note( 0 ), m_instr( 0 ), m_volume( 0 ), m_effect( Effect::None ), m_effectValue( 0 )
{
}

XmCell::~XmCell() = default;

bool XmCell::load( BinStream& str )
{
	uint8_t data;
	str.read( &data );
	if( ( data & 0x80 ) == 0 ) {
		m_note = data;
		str.read( &m_instr ).read( &m_volume ).read( reinterpret_cast<uint8_t*>( &m_effect ) ).read( &m_effectValue );
		return !str.fail();
	}
	if( data & 0x01 )
		str.read( &m_note );
	if( data & 0x02 )
		str.read( &m_instr );
	if( m_instr > 0x80 )
		m_instr = 0;
	if( data & 0x04 )
		str.read( &m_volume );
	if( data & 0x08 )
		str.read( reinterpret_cast<uint8_t*>( &m_effect ) );
	if( data & 0x10 )
		str.read( &m_effectValue );
	return !str.fail();
}

void XmCell::clear()
{
	m_note = 0;
	m_instr = 0xff;
	m_volume = 0;
	m_effect = Effect::None;
	m_effectValue = 0;
}

std::string XmCell::fxString() const
{
	if( m_effect == Effect::None ) {
		return "...";
	}
	else if( static_cast<uint8_t>( m_effect ) <= 0x0f ) {
		return stringFmt( "%1X%02X", int(m_effect), int(m_effectValue) );
	}
	else if( static_cast<uint8_t>( m_effect ) <= 0x21 ) {
		return stringFmt( "%c%02X", int( m_effect ) - 0x0f + 'F', int(m_effectValue) );
	}
	else {
		return stringFmt( "?%02X", int(m_effectValue) );
	}
}

std::string XmCell::noteString() const
{
	if( m_note == 0 ) {
		return "...";
	}
	else if( m_note == KeyOffNote ) {
		return "===";
	}
	else if( m_note < KeyOffNote ) {
		return stringFmt( "%s%d", NoteNames[ ( m_note - 1 ) % 12 ], ( m_note - 1 ) / 12 );
	}
	else {
		return "???";
	}
}

std::string XmCell::trackerString() const
{
	/*    if(!isActive())
	        return "...       ...";*/
	std::string xmsg = noteString();
	if( m_instr == 0 ) {
		xmsg += "    ";
	}
	else {
		xmsg += stringFmt( " %2X ", int(m_instr) );
	}
	/*
	VfxVolSlideDown = 6,
	VfxVolSlideUp = 7,
	VfxFineVolSlideDown = 8,
	VfxFineVolSlideUp = 9,
	VfxSetVibSpeed = 0xa,
	VfxVibrato = 0xb,
	VfxSetPanning = 0xc,
	VfxPanSlideLeft = 0xd,
	VfxPanSlideRight = 0xe,
	VfxPorta = 0xf
	 */
	static const char vfxChars[] = "-+DUSVPLRM";
	switch( highNibble( m_volume ) ) {
		case 0:
			xmsg += "   ";
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			xmsg += stringFmt( "%2d ", m_volume - 0x10 );
			break;
		default:
			xmsg += stringFmt( "%c%X ", vfxChars[highNibble( m_volume ) - 6], int(lowNibble( m_volume )) );
			break;
	}
	return xmsg + fxString();
}

uint8_t XmCell::note() const
{
	return m_note;
}

uint8_t XmCell::instrument() const
{
	return m_instr;
}

uint8_t XmCell::volume() const
{
	return m_volume;
}

Effect XmCell::effect() const
{
	return m_effect;
}

uint8_t XmCell::effectValue() const
{
	return m_effectValue;
}

IArchive& XmCell::serialize( IArchive* data )
{
	*data
	% m_note
	% m_instr
	% m_volume
	% *reinterpret_cast<uint8_t*>( &m_effect )
	% m_effectValue;
	return *data;
}

}
}

/**
 * @}
 */
