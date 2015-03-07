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
 * mtk.cpp - MPU-401 Trakker Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include <cstring>

#include "stream/filestream.h"

#include "mtk.h"

/*** public methods **************************************/

CPlayer *CmtkLoader::factory() { return new CmtkLoader(); }

bool CmtkLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
    struct {
        char id[18];
        uint16_t crc, size;
    } header;
    unsigned int i;
    unsigned long cmpsize, cmpptr = 0, orgptr = 0;
    unsigned short ctrlbits = 0, ctrlmask = 0, cmd, cnt, offs;

    // read header
    f.read(header.id, 18);
    f >> header.crc >> header.size;

    // file validation section
    if (strncmp(header.id, "mpu401tr\x92kk\xeer@data", 18)) {
        return false;
    }

    // load section
    cmpsize = f.size() - 22;
    std::vector<uint8_t> cmp(cmpsize);
    std::vector<uint8_t> org(header.size);
    f.read(cmp.data(), cmpsize);

    while (cmpptr < cmpsize) { // decompress
        ctrlmask >>= 1;
        if (!ctrlmask) {
            ctrlbits = cmp[cmpptr] + (cmp[cmpptr + 1] << 8);
            cmpptr += 2;
            ctrlmask = 0x8000;
        }
        if (!(ctrlbits & ctrlmask)) { // uncompressed data
            if (orgptr >= header.size)
                return false;

            org[orgptr] = cmp[cmpptr];
            orgptr++;
            cmpptr++;
            continue;
        }

        // compressed data
        cmd = (cmp[cmpptr] >> 4) & 0x0f;
        cnt = cmp[cmpptr] & 0x0f;
        cmpptr++;
        switch (cmd) {
        case 0:
            if (orgptr + cnt > header.size)
                return false;
            cnt += 3;
            memset(&org[orgptr], cmp[cmpptr], cnt);
            cmpptr++;
            orgptr += cnt;
            break;

        case 1:
            if (orgptr + cnt > header.size)
                return false;
            cnt += (cmp[cmpptr] << 4) + 19;
            memset(&org[orgptr], cmp[++cmpptr], cnt);
            cmpptr++;
            orgptr += cnt;
            break;

        case 2:
            if (orgptr + cnt > header.size)
                return false;
            offs = (cnt + 3) + (cmp[cmpptr] << 4);
            cnt = cmp[++cmpptr] + 16;
            cmpptr++;
            memcpy(&org[orgptr], &org[orgptr - offs], cnt);
            orgptr += cnt;
            break;

        default:
            if (orgptr + cmd > header.size)
                return false;
            offs = (cnt + 3) + (cmp[cmpptr++] << 4);
            memcpy(&org[orgptr], &org[orgptr - offs], cmd);
            orgptr += cmd;
            break;
        }
    }
    struct mtkdata {
        char songname[34], composername[34], instname[0x80][34];
        unsigned char insts[0x80][12], order[0x80], dummy, patterns[0x32][0x40][9];
    };
    mtkdata* data = (struct mtkdata *)org.data();

    // convert to HSC replay data
    memset(title, 0, 34);
    strncpy(title, data->songname + 1, 33);
    memset(composer, 0, 34);
    strncpy(composer, data->composername + 1, 33);
    memset(instname, 0, 0x80 * 34);
    for (i = 0; i < 0x80; i++)
        strncpy(instname[i], data->instname[i] + 1, 33);
    memcpy(m_instr, data->insts, 0x80 * 12);
    memcpy(m_song, data->order, 0x80);
    memcpy(m_patterns, data->patterns, header.size - 6084);
    for (i = 0; i < 128; i++) { // correct instruments
        m_instr[i][2] ^= (m_instr[i][2] & 0x40) << 1;
        m_instr[i][3] ^= (m_instr[i][3] & 0x40) << 1;
        m_instr[i][11] >>= 4; // make unsigned
    }

    rewind(0);
    return true;
}
