/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * xsm.cpp - eXtra Simple Music Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"

#include "xsm.h"

bool XsmPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if(!f)
        return false;

    // check if header matches
    char id[6];
    f.read(id, 6);
    uint16_t songlen;
    f >> songlen;
    if(strncmp(id, "ofTAZ!", 6) || songlen > 3200)
    {
        return false;
    }

    // read and set instruments
    for(int i = 0; i < 9; i++)
    {
        uint8_t insdata[16];
        f.read(insdata, 16);
        getOpl()->writeReg(0x20 + s_opTable[i], insdata[0]);
        getOpl()->writeReg(0x23 + s_opTable[i], insdata[1]);
        getOpl()->writeReg(0x40 + s_opTable[i], insdata[2]);
        getOpl()->writeReg(0x43 + s_opTable[i], insdata[3]);
        getOpl()->writeReg(0x60 + s_opTable[i], insdata[4]);
        getOpl()->writeReg(0x63 + s_opTable[i], insdata[5]);
        getOpl()->writeReg(0x80 + s_opTable[i], insdata[6]);
        getOpl()->writeReg(0x83 + s_opTable[i], insdata[7]);
        getOpl()->writeReg(0xe0 + s_opTable[i], insdata[8]);
        getOpl()->writeReg(0xe3 + s_opTable[i], insdata[9]);
        getOpl()->writeReg(0xc0 + s_opTable[i], insdata[10]);
    }

    // read song data
    m_music.clear();
    m_music.resize(songlen);
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < songlen; j++)
        {
            f >> m_music[j].data[i];
        }
    }

    // success
    rewind(0);
    return true;
}

bool XsmPlayer::update()
{
    int c;

    if(m_currentRow >= m_music.size())
    {
        m_songEnd = true;
        m_currentRow = m_lastRow = 0;
    }

    for(c = 0; c < 9; c++)
        if(m_music[m_currentRow].data[c] != m_music[m_lastRow].data[c])
            getOpl()->writeReg(0xb0 + c, 0);

    for(c = 0; c < 9; c++)
    {
        if(m_music[m_currentRow].data[c])
            playNote(c, m_music[m_currentRow].data[c] % 12, m_music[m_currentRow].data[c] / 12);
        else
            playNote(c, 0, 0);
    }

    m_lastRow = m_currentRow;
    m_currentRow++;
    return !m_songEnd;
}

void XsmPlayer::rewind(int)
{
    m_currentRow = m_lastRow = 0;
    m_songEnd = false;
}

size_t XsmPlayer::framesUntilUpdate() const
{
    return SampleRate / 5;
}

void XsmPlayer::playNote(int c, int note, int octv)
{
    int freq = s_noteTable[note];

    if(!note && !octv)
        freq = 0;
    getOpl()->writeReg(0xa0 + c, freq & 0xff);
    getOpl()->writeReg(0xb0 + c, (freq / 0xff) | 32 | (octv * 4));
}