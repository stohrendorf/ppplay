/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ksm.cpp - KSM Player for AdPlug by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"

#include <boost/filesystem.hpp>

#include "ksm.h"

namespace
{
const uint16_t adlibfreq[63] = {
    0, 2390, 2411, 2434, 2456, 2480, 2506, 2533, 2562, 2592, 2625, 2659, 2695,
    3414, 3435, 3458, 3480, 3504, 3530, 3557, 3586, 3616, 3649, 3683, 3719, 4438,
    4459, 4482, 4504, 4528, 4554, 4581, 4610, 4640, 4673, 4707, 4743, 5462, 5483,
    5506, 5528, 5552, 5578, 5605, 5634, 5664, 5697, 5731, 5767, 6486, 6507, 6530,
    6552, 6576, 6602, 6629, 6658, 6688, 6721, 6755, 6791, 7510
};
}

/*** public methods **************************************/

Player* KsmPlayer::factory()
{
    return new KsmPlayer();
}

bool KsmPlayer::load(const std::string& filename)
{
    FileStream f(filename);

    // file validation section
    if( !f || f.extension() != ".ksm" )
    {
        return false;
    }

    // Load instruments from 'insts.dat'
    boost::filesystem::path fn(filename);
    fn.remove_filename() /= "insts.dat";

    FileStream insts(fn.string());
    if( !f )
    {
        return false;
    }
    loadInstruments(insts);

    f.read(m_channelInstruments, 16);
    f.read(m_quanters, 16);
    f.read(m_channelFlags, 16);
    f.seekrel(16);
    f.read(m_channelVolumes, 16);
    uint16_t numnotes;
    f >> numnotes;
    m_notes.resize(numnotes);
    f.read(m_notes.data(), m_notes.size());

    if( !m_channelFlags[11] )
    {
        m_bdRegister = 0;
        m_channelCount = 9;
    }
    else
    {
        m_bdRegister = 0x20;
        m_channelCount = 6;
    }

    rewind(size_t(0));
    return true;
}

bool KsmPlayer::update()
{
    m_updateCounter++;
    if( m_updateCounter < m_nextUpdate )
    {
        return !m_songEnd;
    }

    while( m_updateCounter >= m_nextUpdate )
    {
        auto templong = m_notes[m_notesOffset];
        const auto track = ((templong >> 8) & 0x0f);
        if( (templong & 192) == 0 )
        {
            size_t i = 0;

            while( (i < m_channelCount) && ((m_channelNotes[i] != (templong & 63)) || (m_slotToTrack[i] != ((templong >> 8) & 15))) )
            {
                i++;
            }
            if( i < m_channelCount )
            {
                getOpl()->writeReg(0xb0 + i, (adlibfreq[templong & 63] >> 8) & 223);
                m_channelNotes[i] = 0;
                m_slotAges[i] = 0;
            }
        }
        else
        {
            int volevel = m_channelVolumes[track];
            if( (templong & 192) == 128 )
            {
                volevel -= 4;
                if( volevel < 0 )
                {
                    volevel = 0;
                }
            }
            if( (templong & 192) == 192 )
            {
                volevel += 4;
                if( volevel > 63 )
                {
                    volevel = 63;
                }
            }
            if( track < 11 )
            {
                size_t temp = 0;
                size_t i = m_channelCount;
                for( size_t j = 0; j < m_channelCount; j++ )
                {
                    if( (m_nextUpdate - m_slotAges[j] >= temp) && (m_slotToTrack[j] == track) )
                    {
                        temp = m_nextUpdate - m_slotAges[j];
                        i = j;
                    }
                }
                if( i < m_channelCount )
                {
                    getOpl()->writeReg(0xb0 + i, 0);
                    const auto volval = (m_instruments[m_channelInstruments[track]][1] & 192) + (volevel ^ 63);
                    getOpl()->writeReg(0x40 + s_opTable[i] + 3, volval);
                    getOpl()->writeReg(0xa0 + i, adlibfreq[templong & 0x3f] & 0xff);
                    getOpl()->writeReg(0xb0 + i, (adlibfreq[templong & 0x3f] >> 8) | 0x20);
                    m_channelNotes[i] = templong & 0x3f;
                    m_slotAges[i] = m_nextUpdate;
                }
            }
            else if( (m_bdRegister & 0x20) > 0 )
            {
                auto freq = adlibfreq[templong & 63];
                uint8_t slotIdx;
                uint8_t drumnum;
                switch( track )
                {
                    case 11:
                        drumnum = 0x10;
                        slotIdx = 6;
                        freq -= 2048;
                        break;
                    case 12:
                        drumnum = 0x08;
                        slotIdx = 7;
                        freq -= 2048;
                        break;
                    case 13:
                        drumnum = 0x04;
                        slotIdx = 8;
                        break;
                    case 14:
                        drumnum = 0x02;
                        slotIdx = 8;
                        break;
                    case 15:
                        drumnum = 0x01;
                        slotIdx = 7;
                        freq -= 2048;
                        break;
                    default:
                        BOOST_THROW_EXCEPTION(std::runtime_error("Unexpected track number"));
                }
                getOpl()->writeReg(0xa0 + slotIdx, freq & 0xff);
                getOpl()->writeReg(0xb0 + slotIdx, (freq >> 8) & 223);
                getOpl()->writeReg(0xbd, m_bdRegister & (0xff - drumnum));
                m_bdRegister |= drumnum;
                if( track == 11 || track == 12 || track == 14 )
                {
                    const auto volval = (m_instruments[m_channelInstruments[track]][1] & 192) + (volevel ^ 63);
                    getOpl()->writeReg(0x40 + s_opTable[slotIdx] + 3, volval);
                }
                else
                {
                    const auto volval = (m_instruments[m_channelInstruments[track]][6] & 192) + (volevel ^ 63);
                    getOpl()->writeReg(0x40 + s_opTable[slotIdx], volval);
                }
                getOpl()->writeReg(0xbd, m_bdRegister);
            }
        }
        m_notesOffset++;
        if( m_notesOffset >= m_notes.size() )
        {
            m_notesOffset = 0;
            m_songEnd = true;
        }
        templong = m_notes[m_notesOffset];
        if( m_notesOffset == 0 )
        {
            m_updateCounter = (templong >> 12) - 1;
        }
        const auto quanter = (240 / m_quanters[(templong >> 8) & 15]);
        m_nextUpdate = (((templong >> 12) + (quanter >> 1)) / quanter) * quanter;
    }

    return !m_songEnd;
}

void KsmPlayer::rewind(const boost::optional<size_t>&)
{
    m_songEnd = false;
    getOpl()->writeReg(1, 32);
    getOpl()->writeReg(4, 0);
    getOpl()->writeReg(8, 0);
    getOpl()->writeReg(0xbd, m_bdRegister);

    if( m_channelFlags[11] == 1 )
    {
        auto instbuf = m_instruments[m_channelInstruments[11]];
        instbuf[1] = ((instbuf[1] & 192) | (m_channelVolumes[11] ^ 63));
        storeInstrument(6, instbuf);
        for( int i = 0; i < 5; i++ )
        {
            instbuf[i] = m_instruments[m_channelInstruments[12]][i];
        }
        for( int i = 5; i < 11; i++ )
        {
            instbuf[i] = m_instruments[m_channelInstruments[15]][i];
        }
        instbuf[1] = ((instbuf[1] & 192) | (m_channelVolumes[12] ^ 63));
        instbuf[6] = ((instbuf[6] & 192) | (m_channelVolumes[15] ^ 63));
        storeInstrument(7, instbuf);
        for( int i = 0; i < 5; i++ )
        {
            instbuf[i] = m_instruments[m_channelInstruments[14]][i];
        }
        for( int i = 5; i < 11; i++ )
        {
            instbuf[i] = m_instruments[m_channelInstruments[13]][i];
        }
        instbuf[1] = ((instbuf[1] & 192) | (m_channelVolumes[14] ^ 63));
        instbuf[6] = ((instbuf[6] & 192) | (m_channelVolumes[13] ^ 63));
        storeInstrument(8, instbuf);
    }

    for( size_t i = 0; i < m_channelCount; i++ )
    {
        m_slotToTrack[i] = 0;
        m_slotAges[i] = 0;
    }
    size_t j = 0;
    for( int i = 0; i < 16; i++ )
    {
        if( (m_channelFlags[i] > 0) && (j < m_channelCount) )
        {
            auto k = m_channelFlags[i];
            while( (j < m_channelCount) && (k > 0) )
            {
                m_slotToTrack[j] = i;
                k--;
                j++;
            }
        }
    }
    for( size_t i = 0; i < m_channelCount; i++ )
    {
        auto instbuf = m_instruments[m_channelInstruments[m_slotToTrack[i]]];
        instbuf[1] = ((instbuf[1] & 192) | (63 - m_channelVolumes[m_slotToTrack[i]]));
        storeInstrument(i, instbuf);
        m_channelNotes[i] = 0;
    }
    auto templong = m_notes.front();
    m_updateCounter = (templong >> 12) - 1;
    m_nextUpdate = (templong >> 12) - 1;
    m_notesOffset = 0;
}

std::string KsmPlayer::instrumentTitle(size_t n) const
{
    if( m_channelFlags[n] )
    {
        return m_instrumentNames[m_channelInstruments[n]];
    }
    else
    {
        return std::string();
    }
}

/*** private methods *************************************/

void KsmPlayer::loadInstruments(FileStream& f)
{
    for( int i = 0; i < 256; i++ )
    {
        f.read(m_instrumentNames[i], 20);
        f.read(m_instruments[i].data(), 11);
        f.seekrel(2);
    }
}

void KsmPlayer::storeInstrument(int chan, const std::array<uint8_t, 11>& data)
{
    int offs;

    getOpl()->writeReg(0xa0 + chan, 0);
    getOpl()->writeReg(0xb0 + chan, 0);
    getOpl()->writeReg(0xc0 + chan, data[10]);
    offs = s_opTable[chan];
    getOpl()->writeReg(0x20 + offs, data[5]);
    getOpl()->writeReg(0x40 + offs, data[6]);
    getOpl()->writeReg(0x60 + offs, data[7]);
    getOpl()->writeReg(0x80 + offs, data[8]);
    getOpl()->writeReg(0xe0 + offs, data[9]);
    offs += 3;
    getOpl()->writeReg(0x20 + offs, data[0]);
    getOpl()->writeReg(0x40 + offs, data[1]);
    getOpl()->writeReg(0x60 + offs, data[2]);
    getOpl()->writeReg(0x80 + offs, data[3]);
    getOpl()->writeReg(0xe0 + offs, data[4]);
}