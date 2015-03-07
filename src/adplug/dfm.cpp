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
 * dfm.cpp - Digital-FM Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstdio>
#include <cstring>

#include "stream/filestream.h"

#include "dfm.h"
#include "debug.h"

CPlayer *CdfmLoader::factory() { return new CdfmLoader(); }

bool CdfmLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
    const unsigned char convfx[8] = { 255, 255, 17, 19, 23, 24, 255, 13 };

    // file validation
    f.read(header.id, 4);
    f >> header.hiver >> header.lover;
    if (strncmp(header.id, "DFM\x1a", 4) || header.hiver > 1) {
        return false;
    }

    // load
    m_restartpos = 0;
    m_flags = Standard;
    m_bpm = 0;
    init_trackord();
    f.read(songinfo, 33);
    f >> m_initspeed;
    for (auto i = 0; i < 32; i++)
        f.read(instname[i], 12);
    for (auto i = 0; i < 32; i++) {
        f >> m_instruments[i].data[1];
        f >> m_instruments[i].data[2];
        f >> m_instruments[i].data[9];
        f >> m_instruments[i].data[10];
        f >> m_instruments[i].data[3];
        f >> m_instruments[i].data[4];
        f >> m_instruments[i].data[5];
        f >> m_instruments[i].data[6];
        f >> m_instruments[i].data[7];
        f >> m_instruments[i].data[8];
        f >> m_instruments[i].data[0];
    }
    f.read(m_order.data(), 128);
    int i;
    for (i = 0; i < 128 && m_order[i] != 128; i++)
        /* nothing */;
    m_length = i;
    uint8_t npats;
    f >> npats;
    for (auto i = 0; i < npats; i++) {
        uint8_t n;
        f >> n;
        for (auto r = 0; r < 64; r++) {
            for (auto c = 0; c < 9; c++) {
                uint8_t note;
                f >> note;
                if ((note & 15) == 15)
                    m_tracks.at(n * 9 + c, r).note = 127; // key off
                else
                    m_tracks.at(n * 9 + c, r).note = ((note & 127) >> 4) * 12 + (note & 15);
                if (note & 128) { // additional effect byte
                    uint8_t fx;
                    f >> fx;
                    if (fx >> 5 == 1)
                        m_tracks.at(n * 9 + c, r).inst = (fx & 31) + 1;
                    else {
                        m_tracks.at(n * 9 + c, r).command = convfx[fx >> 5];
                        if (m_tracks.at(n * 9 + c, r).command == 17) { // set volume
                            auto param = fx & 31;
                            param = 63 - param * 2;
                            m_tracks.at(n * 9 + c, r).param1 = param >> 4;
                            m_tracks.at(n * 9 + c, r).param2 = param & 15;
                        } else {
                            m_tracks.at(n * 9 + c, r).param1 = (fx & 31) >> 4;
                            m_tracks.at(n * 9 + c, r).param2 = fx & 15;
                        }
                    }
                }
            }
        }
    }

    rewind(0);
    return true;
}

std::string CdfmLoader::gettype() {
    char tmpstr[20];

    sprintf(tmpstr, "Digital-FM %d.%d", header.hiver, header.lover);
    return std::string(tmpstr);
}

size_t CdfmLoader::framesUntilUpdate() { return SampleRate / 125; }
