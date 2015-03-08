/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * amd.cpp - AMD Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstring>
#include "stream/filestream.h"

#include "amd.h"
#include "debug.h"

CPlayer *CamdLoader::factory() { return new CamdLoader(); }

bool CamdLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
#pragma pack(push,1)
    struct {
        char id[9];
        uint8_t version;
    } header;
#pragma pack(pop)
    int k, t, maxi = 0;
    const unsigned char convfx[10] = { 0, 1, 2, 9, 17, 11, 13, 18, 3, 14 };
    const unsigned char convvol[64] = {
        0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 0xa, 0xa, 0xb,
        0xc, 0xc, 0xd, 0xe, 0xe, 0xf, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x14,
        0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x21,
        0x22, 0x23, 0x25, 0x26, 0x28, 0x29, 0x2b, 0x2d, 0x2e, 0x30, 0x32, 0x35,
        0x37, 0x3a, 0x3c, 0x3f
    };

    // file validation section
    if (f.size() < 1072) {
        return false;
    }
    f.seek(1062);
    f.read(&header);
    if (strncmp(header.id, "<o\xefQU\xeeRoR", 9) && strncmp(header.id, "MaDoKaN96", 9)) {
        return false;
    }

    // load section
    f.seek(0);
    f.read(songname, sizeof(songname)/sizeof(songname[0]));
    f.read(author, sizeof(author)/sizeof(author[0]));
    for (int i = 0; i < 26; i++) {
        f.read(m_instrumentNames[i], 23);
        f.read(instrument(i,true).data, 11);
    }
    uint8_t tmp8;
    f.read(&tmp8);
    m_length = tmp8;
    f.read(&tmp8);
    numberOfPatterns = tmp8 + 1;
    m_order.resize(128);
    f.read(m_order.data(), 128);
    f.seekrel(10);
    if (header.version == 0x10) { // unpacked module
        maxi = numberOfPatterns * 9;
        for (int i = 0; i < 64; i++)
            for(int j=0; j<9; ++j)
                trackord.at(i,j) = i*9 + j + 1;
        t = 0;
        while (f.pos() != f.size()) {
            for (int j = 0; j < 64; j++)
                for (int i = t; i < t + 9; i++) {
                    uint8_t buf;
                    f.read(&buf);
                    m_tracks.at(i,j).param2 = (buf & 127) % 10;
                    m_tracks.at(i,j).param1 = (buf & 127) / 10;
                    f.read(&buf);
                    m_tracks.at(i,j).inst = buf >> 4;
                    m_tracks.at(i,j).command = buf & 0x0f;
                    f.read(&buf);
                    if (buf >> 4) // fix bug in AMD save routine
                        m_tracks.at(i,j).note = ((buf & 14) >> 1) * 12 + (buf >> 4);
                    else
                        m_tracks.at(i,j).note = 0;
                    m_tracks.at(i,j).inst += (buf & 1) << 4;
                }
            t += 9;
        }
    } else { // packed module
        for (int i = 0; i < numberOfPatterns; i++) {
            for (int j = 0; j < 9; j++) {
                uint16_t tmp16;
                f.read(&tmp16);
                trackord.at(i,j) = tmp16 + 1;
            }
        }
        uint16_t numtrax;
        f.read(&numtrax);
        for (k = 0; k < numtrax; k++) {
            uint16_t i;
            f.read(&i);
            if (i > 575)
                i = 575; // fix corrupted modules
            maxi = (i + 1 > maxi ? i + 1 : maxi);
            int j = 0;
            do {
                uint8_t buf;
                f.read(&buf);
                if (buf & 128) {
                    for (t = j; t < j + (buf & 127) && t < 64; t++) {
                        m_tracks.at(i,t).command = 0;
                        m_tracks.at(i,t).inst = 0;
                        m_tracks.at(i,t).note = 0;
                        m_tracks.at(i,t).param1 = 0;
                        m_tracks.at(i,t).param2 = 0;
                    }
                    j += buf & 127;
                    continue;
                }
                m_tracks.at(i,j).param2 = buf % 10;
                m_tracks.at(i,j).param1 = buf / 10;
                f.read(&buf);
                m_tracks.at(i,j).inst = buf >> 4;
                m_tracks.at(i,j).command = buf & 0x0f;
                f.read(&buf);
                if (buf >> 4) // fix bug in AMD save routine
                    m_tracks.at(i,j).note = ((buf & 14) >> 1) * 12 + (buf >> 4);
                else
                    m_tracks.at(i,j).note = 0;
                m_tracks.at(i,j).inst += (buf & 1) << 4;
                j++;
            } while (j < 64);
        }
    }

    // convert to protracker replay data
    m_bpm = 50;
    m_restartpos = 0;
    m_flags = Decimal;
    for (int i = 0; i < 26; i++) { // convert instruments
        CmodPlayer::Instrument& inst = instrument(i);
        auto buf = inst.data[0];
        auto buf2 = inst.data[1];
        inst.data[0] = inst.data[10];
        inst.data[1] = buf;
        buf = inst.data[2];
        inst.data[2] = inst.data[5];
        auto buf3 = inst.data[3];
        inst.data[3] = buf;
        buf = inst.data[4];
        inst.data[4] = inst.data[7];
        inst.data[5] = buf3;
        buf3 = inst.data[6];
        inst.data[6] = inst.data[8];
        inst.data[7] = buf;
        inst.data[8] = inst.data[9];
        inst.data[9] = buf2;
        inst.data[10] = buf3;
        for (int j = 0; j < 23; j++) // convert names
            if (m_instrumentNames[i][j] == '\xff')
                m_instrumentNames[i][j] = '\x20';
    }
    for (int i = 0; i < maxi; i++) // convert patterns
        for (int j = 0; j < 64; j++) {
            m_tracks.at(i,j).command = convfx[m_tracks.at(i,j).command];
            // extended command
            if (m_tracks.at(i,j).command == 14) {
                if (m_tracks.at(i,j).param1 == 2) {
                    m_tracks.at(i,j).command = 10;
                    m_tracks.at(i,j).param1 = m_tracks.at(i,j).param2;
                    m_tracks.at(i,j).param2 = 0;
                }

                if (m_tracks.at(i,j).param1 == 3) {
                    m_tracks.at(i,j).command = 10;
                    m_tracks.at(i,j).param1 = 0;
                }
            }

            // fix volume
            if (m_tracks.at(i,j).command == 17) {
                int vol = convvol[m_tracks.at(i,j).param1 * 10 + m_tracks.at(i,j).param2];

                if (vol > 63)
                    vol = 63;
                m_tracks.at(i,j).param1 = vol / 10;
                m_tracks.at(i,j).param2 = vol % 10;
            }
        }

    rewind(0);
    return true;
}

size_t CamdLoader::framesUntilUpdate() {
    if (m_tempo)
        return SampleRate / m_tempo;
    else
        return SampleRate / 18.2;
}
