/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * sa2.cpp - SAdT2 Loader by Simon Peter <dn.tlp@gmx.net>
 *           SAdT Loader by Mamiya <mamiya@users.sourceforge.net>
 */

#include <cstring>
#include <cstdio>
#include <cstring>

#include "stream/filestream.h"

#include "sa2.h"
#include "debug.h"

CPlayer *Csa2Loader::factory() { return new Csa2Loader(); }

bool Csa2Loader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;

#pragma pack(push,1)
    struct InstrumentData {
        uint8_t data[11], arpstart, arpspeed, arppos, arpspdcnt;
    };
#pragma pack(pop)

    const unsigned char convfx[16] = { 0, 1, 2, 3, 4, 5, 6, 255, 8, 255, 10, 11,
                                       12, 13, 255, 15 };
    unsigned char sat_type;
    enum SAT_TYPE {
        HAS_ARPEGIOLIST = (1 << 7),
        HAS_V7PATTERNS = (1 << 6),
        HAS_ACTIVECHANNELS = (1 << 5),
        HAS_TRACKORDER = (1 << 4),
        HAS_ARPEGIO = (1 << 3),
        HAS_OLDBPM = (1 << 2),
        HAS_OLDPATTERNS = (1 << 1),
        HAS_UNKNOWN127 = (1 << 0)
    };

    // read header
    f.read(header.sadt, 4);
    f >> header.version;

    // file validation section
    if (strncmp(header.sadt, "SAdT", 4)) {
        return false;
    }
    int notedis = 0;
    switch (header.version) {
    case 1:
        notedis = +0x18;
        sat_type = HAS_UNKNOWN127 | HAS_OLDPATTERNS | HAS_OLDBPM;
        break;
    case 2:
        notedis = +0x18;
        sat_type = HAS_OLDPATTERNS | HAS_OLDBPM;
        break;
    case 3:
        notedis = +0x0c;
        sat_type = HAS_OLDPATTERNS | HAS_OLDBPM;
        break;
    case 4:
        notedis = +0x0c;
        sat_type = HAS_ARPEGIO | HAS_OLDPATTERNS | HAS_OLDBPM;
        break;
    case 5:
        notedis = +0x0c;
        sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_OLDPATTERNS | HAS_OLDBPM;
        break;
    case 6:
        sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_OLDPATTERNS | HAS_OLDBPM;
        break;
    case 7:
        sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_V7PATTERNS;
        break;
    case 8:
        sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_TRACKORDER;
        break;
    case 9:
        sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_TRACKORDER | HAS_ACTIVECHANNELS;
        break;
    default: /* unknown */
        return false;
    }

    // load section
    // instruments
    for (int i = 0; i < 31; i++) {
        InstrumentData insts;
        CmodPlayer::Instrument& inst = instrument(i,true);
        if (sat_type & HAS_ARPEGIO) {
            f >> insts;

            inst.arpstart = insts.arpstart;
            inst.arpspeed = insts.arpspeed;
            inst.arppos = insts.arppos;
            inst.arpspdcnt = insts.arpspdcnt;
        }
        else {
            f.read(insts.data, 11);
            inst.arpstart = 0;
            inst.arpspeed = 0;
            inst.arppos = 0;
            inst.arpspdcnt = 0;
        }
        std::copy_n(insts.data, 11, inst.data);
        inst.misc = 0;
        inst.slide = 0;
    }

    // instrument names
    for (int i = 0; i < 29; i++)
        f.read(instname[i], 17);

    f.seekrel(3); // dummy bytes
    f.read(m_order.data(), 128);
    if (sat_type & HAS_UNKNOWN127)
        f.seekrel(127);

    // infos
    f >> numberOfPatterns;
    {
        uint8_t tmp;
        f >> tmp;
        m_length = tmp;
        f >> tmp;
        m_restartpos = tmp;
    }

    // bpm
    f >> m_bpm;
    if (sat_type & HAS_OLDBPM) {
        m_bpm = m_bpm * 125 / 50; // cps -> bpm
    }

    if (sat_type & HAS_ARPEGIOLIST) {
        arplist = std::array<uint8_t,256>();
        f.read(arplist->data(), 256);
        arpcmd = std::array<uint8_t,256>();
        f.read(arpcmd->data(), 256);
    }

    for (int i = 0; i < 64; i++) { // track orders
        for (int j = 0; j < 9; j++) {
            if (sat_type & HAS_TRACKORDER) {
                uint8_t tmp;
                f >> tmp;
                trackord.at(i,j) = tmp;
            }
            else {
                trackord.at(i,j) = i * 9 + j;
            }
        }
    }

    if (sat_type & HAS_ACTIVECHANNELS) {
        uint16_t tmp;
        f >> tmp;
        activechan = uint32_t(tmp) << 16; // active channels
    }

    AdPlug_LogWrite("Csa2Loader::load(\"%s\"): sat_type = %x, nop = %d, "
                    "length = %d, restartpos = %d, activechan = %x, bpm = %d\n",
                    filename.c_str(), sat_type, numberOfPatterns, m_length, m_restartpos,
                    activechan, m_bpm);

    // track data
    if (sat_type & HAS_OLDPATTERNS) {
        int i = 0;
        while(f.pos() < f.size()) {
            for (int j = 0; j < 64; j++) {
                for (int k = 0; k < 9; k++) {
                    uint8_t buf;
                    f >> buf;
                    m_tracks.at(i + k, j).note = buf ? (buf + notedis) : 0;
                    f >> buf;
                    m_tracks.at(i + k, j).inst = buf;
                    f >> buf;
                    m_tracks.at(i + k, j).command = convfx[buf & 0xf];
                    f >> buf;
                    m_tracks.at(i + k, j).param1 = buf;
                    f >> buf;
                    m_tracks.at(i + k, j).param2 = buf;
                }
            }
            i += 9;
        }
    }
    else if (sat_type & HAS_V7PATTERNS) {
        int i = 0;
        while (f.pos() < f.size()) {
            for (int j = 0; j < 64; j++) {
                for (int k = 0; k < 9; k++) {
                    uint8_t buf;
                    f >> buf;
                    m_tracks.at(i + k, j).note = buf >> 1;
                    m_tracks.at(i + k, j).inst = (buf & 1) << 4;
                    f >> buf;
                    m_tracks.at(i + k, j).inst += buf >> 4;
                    m_tracks.at(i + k, j).command = convfx[buf & 0x0f];
                    f >> buf;
                    m_tracks.at(i + k, j).param1 = buf >> 4;
                    m_tracks.at(i + k, j).param2 = buf & 0x0f;
                }
            }
            i += 9;
        }
    } else {
        int i = 0;
        while (f.pos() < f.size()) {
            for (int j = 0; j < 64; j++) {
                uint8_t buf;
                f >> buf;
                m_tracks.at(i,j).note = buf >> 1;
                m_tracks.at(i,j).inst = (buf & 1) << 4;
                f >> buf;
                m_tracks.at(i,j).inst += buf >> 4;
                m_tracks.at(i,j).command = convfx[buf & 0x0f];
                f >> buf;
                m_tracks.at(i,j).param1 = buf >> 4;
                m_tracks.at(i,j).param2 = buf & 0x0f;
            }
            i++;
        }
    }

    // fix instrument names
    for (int i = 0; i < 29; i++)
        for (int j = 0; j < 17; j++)
            if (!instname[i][j])
                instname[i][j] = ' ';

    rewind(0); // rewind module
    return true;
}

std::string Csa2Loader::gettype() {
    char tmpstr[40];

    sprintf(tmpstr, "Surprise! Adlib Tracker 2 (version %d)", header.version);
    return std::string(tmpstr);
}

std::string Csa2Loader::gettitle() {
    char bufinst[29 * 17], buf[18];
    int i, ptr;

    // parse instrument names for song name
    memset(bufinst, '\0', 29 * 17);
    for (i = 0; i < 29; i++) {
        buf[16] = ' ';
        buf[17] = '\0';
        memcpy(buf, instname[i] + 1, 16);
        for (ptr = 16; ptr > 0; ptr--)
            if (buf[ptr] == ' ')
                buf[ptr] = '\0';
            else {
                if (ptr < 16)
                    buf[ptr + 1] = ' ';
                break;
            }
        strcat(bufinst, buf);
    }

    if (strchr(bufinst, '"'))
        return std::string(bufinst, strchr(bufinst, '"') - bufinst + 1,
                           strrchr(bufinst, '"') - strchr(bufinst, '"') - 1);
    else
        return std::string();
}
