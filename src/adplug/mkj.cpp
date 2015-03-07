/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2004 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * mkj.cpp - MKJamz Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstring>
#include <cassert>

#include "stream/filestream.h"

#include "mkj.h"
#include "debug.h"

CPlayer *CmkjPlayer::factory() { return new CmkjPlayer(); }

bool CmkjPlayer::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
    // file validation
    char id[6];
    f.read(id, 6);
    if (strncmp(id, "MKJamz", 6)) {
        return false;
    }
    float ver;
    f >> ver;
    if (ver > 1.12) {
        return false;
    }

    // load
    f >> m_maxChannel;
    getOpl()->writeReg(1, 32);
    for(auto i = 0; i < m_maxChannel; i++) {
        int16_t inst[8];
        f.read(inst, 8);
        getOpl()->writeReg(0x20 + s_opTable[i], inst[4]);
        getOpl()->writeReg(0x23 + s_opTable[i], inst[0]);
        getOpl()->writeReg(0x40 + s_opTable[i], inst[5]);
        getOpl()->writeReg(0x43 + s_opTable[i], inst[1]);
        getOpl()->writeReg(0x60 + s_opTable[i], inst[6]);
        getOpl()->writeReg(0x63 + s_opTable[i], inst[2]);
        getOpl()->writeReg(0x80 + s_opTable[i], inst[7]);
        getOpl()->writeReg(0x83 + s_opTable[i], inst[3]);
    }
    f >> m_maxNotes;
    for (auto i = 0; i < m_maxChannel; i++)
        f >> m_channels[i].defined;
    m_songBuf.resize((m_maxChannel + 1) * m_maxNotes);
    f.read(m_songBuf.data(), m_songBuf.size());

    AdPlug_LogWrite("CmkjPlayer::load(\"%s\"): loaded file ver %.2f, %d channels,"
                    " %d notes/channel.\n",
                    filename.c_str(), ver, m_maxChannel, m_maxNotes);
    rewind(0);
    return true;
}

bool CmkjPlayer::update() {
    int c, i;
    short note;

    for (c = 0; c < m_maxChannel; c++) {
        if (!m_channels[c].defined) // skip if channel is disabled
            continue;

        if (m_channels[c].pstat) {
            m_channels[c].pstat--;
            continue;
        }

        getOpl()->writeReg(0xb0 + c, 0); // key off
        do {
            assert(m_channels[c].songptr < (m_maxChannel + 1) * m_maxNotes);
            note = m_songBuf[m_channels[c].songptr];
            if (m_channels[c].songptr - c > m_maxChannel)
                if (note && note < 250)
                    m_channels[c].pstat = m_channels[c].speed;
            switch (note) {
            // normal notes
            case 68:
                getOpl()->writeReg(0xa0 + c, 0x81);
                getOpl()->writeReg(0xb0 + c, 0x21 + 4 * m_channels[c].octave);
                break;
            case 69:
                getOpl()->writeReg(0xa0 + c, 0xb0);
                getOpl()->writeReg(0xb0 + c, 0x21 + 4 * m_channels[c].octave);
                break;
            case 70:
                getOpl()->writeReg(0xa0 + c, 0xca);
                getOpl()->writeReg(0xb0 + c, 0x21 + 4 * m_channels[c].octave);
                break;
            case 71:
                getOpl()->writeReg(0xa0 + c, 0x2);
                getOpl()->writeReg(0xb0 + c, 0x22 + 4 * m_channels[c].octave);
                break;
            case 65:
                getOpl()->writeReg(0xa0 + c, 0x41);
                getOpl()->writeReg(0xb0 + c, 0x22 + 4 * m_channels[c].octave);
                break;
            case 66:
                getOpl()->writeReg(0xa0 + c, 0x87);
                getOpl()->writeReg(0xb0 + c, 0x22 + 4 * m_channels[c].octave);
                break;
            case 67:
                getOpl()->writeReg(0xa0 + c, 0xae);
                getOpl()->writeReg(0xb0 + c, 0x22 + 4 * m_channels[c].octave);
                break;
            case 17:
                getOpl()->writeReg(0xa0 + c, 0x6b);
                getOpl()->writeReg(0xb0 + c, 0x21 + 4 * m_channels[c].octave);
                break;
            case 18:
                getOpl()->writeReg(0xa0 + c, 0x98);
                getOpl()->writeReg(0xb0 + c, 0x21 + 4 * m_channels[c].octave);
                break;
            case 20:
                getOpl()->writeReg(0xa0 + c, 0xe5);
                getOpl()->writeReg(0xb0 + c, 0x21 + 4 * m_channels[c].octave);
                break;
            case 21:
                getOpl()->writeReg(0xa0 + c, 0x20);
                getOpl()->writeReg(0xb0 + c, 0x22 + 4 * m_channels[c].octave);
                break;
            case 15:
                getOpl()->writeReg(0xa0 + c, 0x63);
                getOpl()->writeReg(0xb0 + c, 0x22 + 4 * m_channels[c].octave);
                break;
            case 255: // delay
                m_channels[c].songptr += m_maxChannel;
                m_channels[c].pstat = m_songBuf[m_channels[c].songptr];
                break;
            case 254: // set octave
                m_channels[c].songptr += m_maxChannel;
                m_channels[c].octave = m_songBuf[m_channels[c].songptr];
                break;
            case 253: // set speed
                m_channels[c].songptr += m_maxChannel;
                m_channels[c].speed = m_songBuf[m_channels[c].songptr];
                break;
            case 252: // set waveform
                m_channels[c].songptr += m_maxChannel;
                m_channels[c].waveform = m_songBuf[m_channels[c].songptr] - 300;
                if (c > 2)
                    getOpl()->writeReg(0xe0 + c + (c + 6), m_channels[c].waveform);
                else
                    getOpl()->writeReg(0xe0 + c, m_channels[c].waveform);
                break;
            case 251: // song end
                for (i = 0; i < m_maxChannel; i++)
                    m_channels[i].songptr = i;
                songend = true;
                return false;
            }

            if (m_channels[c].songptr - c < m_maxNotes)
                m_channels[c].songptr += m_maxChannel;
            else
                m_channels[c].songptr = c;
        } while (!m_channels[c].pstat);
    }

    return !songend;
}

void CmkjPlayer::rewind(int) {
    int i;

    for (i = 0; i < m_maxChannel; i++) {
        m_channels[i].pstat = 0;
        m_channels[i].speed = 0;
        m_channels[i].waveform = 0;
        m_channels[i].songptr = i;
        m_channels[i].octave = 4;
    }

    songend = false;
}

size_t CmkjPlayer::framesUntilUpdate() { return SampleRate / 100; }
