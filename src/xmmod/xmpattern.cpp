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

#include "xmpattern.h"

using namespace ppp;
using namespace ppp::xm;

XmCell::XmCell() throw() : GenCell(), m_note(0), m_instr(0), m_volume(0), m_effect(Effect::None), m_effectValue(0)
{}

XmCell::~XmCell() throw()
{}

bool XmCell::load(BinStream &str) throw(PppException)
{
    uint8_t data;
    str.read(&data);
    if((data & 0x80) == 0) {
        m_note = data;
        str.read(&m_instr).read(&m_volume).read(reinterpret_cast<uint8_t *>(&m_effect)).read(&m_effectValue);
        return !str.fail();
    }
    if(data & 0x01)
        str.read(&m_note);
    if(data & 0x02)
        str.read(&m_instr);
    if(data & 0x04)
        str.read(&m_volume);
    if(data & 0x08)
        str.read(reinterpret_cast<uint8_t *>(&m_effect));
    if(data & 0x10)
        str.read(&m_effectValue);
    return !str.fail();
}

void XmCell::reset() throw()
{
    m_note = 0;
    m_instr = 0xff;
    m_volume = 0;
    m_effect = Effect::None;
    m_effectValue = 0;
}

std::string XmCell::trackerString() const throw()
{
    if(!isActive())
        return "... .. .. ...";
    std::string xmsg = "";
    // TODO
    /*	if( m_note == 0 )
    		xmsg += "... ";
    	else if( m_note == 97 )
    		xmsg += "===";
    	else
    		xmsg += stringf( "%s%d ", NoteNames[m_note & 0x0f], m_note >> 4 );
    	if( m_instr != 0 )
    		xmsg += stringf( "%.2d ", m_instr );
    	else
    		xmsg += ".. ";
    	if( m_volume != 0 )
    		xmsg += stringf( "%.2x ", m_volume );
    	else
    		xmsg += ".. ";
    	if( m_effect != 0 )
    		xmsg += stringf( "%c%.2x", 'A' - 1 + m_effect, m_effectValue );
    	else
    		xmsg += "...";*/
    return xmsg;
}

uint8_t XmCell::getNote() const throw()
{
    return m_note;
}

uint8_t XmCell::getInstr() const throw()
{
    return m_instr;
}

uint8_t XmCell::getVolume() const throw()
{
    return m_volume;
}

Effect XmCell::getEffect() const throw()
{
    return m_effect;
}

uint8_t XmCell::getEffectValue() const throw()
{
    return m_effectValue;
}

XmCell::Ptr XmPattern::createCell(uint16_t trackIndex, uint16_t row) throw(PppException)
{
    PPP_TEST(row >= numRows());
    PPP_TEST(trackIndex >= numChannels());
    XmCell::Vector *track = &m_tracks[trackIndex];
    XmCell::Ptr &cell = track->at(row);
    if(cell)
        return cell;
    cell.reset(new XmCell());
    return cell;
}

XmPattern::XmPattern(int16_t chans) throw(PppException) : m_tracks(chans)
{
}

XmPattern::~XmPattern() throw()
{
}

bool XmPattern::load(BinStream &str) throw(PppException)
{
    uint32_t hdrLen;
    str.read(&hdrLen);
    uint8_t packType;
    str.read(&packType);
    if(packType != 0) {
        LOG_WARNING("Unsupported Pattern pack type: %u", packType);
        return false;
    }
    uint16_t rows;
    str.read(&rows);
    if(rows < 1 || rows > 256) {
        LOG_WARNING("Number of rows out of range: %u", rows);
        return false;
    }
    for(std::size_t chan = 0; chan < m_tracks.size(); chan++) {
        m_tracks[chan].resize(rows, XmCell::Ptr());
    }
    uint16_t packedSize;
    str.read(&packedSize);
    str.seekrel(hdrLen - 9);   // copied from schismtracker
    if(packedSize == 0) {
        return true;
    }
    for(uint16_t row = 0; row < rows; row++) {
        for(std::size_t chan = 0; chan < m_tracks.size(); chan++) {
            XmCell *cell = new XmCell();
            if(!cell->load(str))
                return false;
            m_tracks[chan][row].reset(cell);
        }
    }
    return !str.fail();
}

XmCell::Ptr XmPattern::getCell(uint16_t trackIndex, uint16_t row) throw()
{
    if(trackIndex >= numChannels() || row >= numRows())
        return XmCell::Ptr();
    const XmCell::Vector &track = m_tracks.at(trackIndex);
    return track.at(row);
}

std::size_t XmPattern::numRows() const
{
    if(numChannels() == 0)
        return 0;
    return m_tracks[0].size();
}

std::size_t XmPattern::numChannels() const
{
    return m_tracks.size();
}
