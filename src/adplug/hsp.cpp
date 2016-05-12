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
 * hsp.cpp - HSP Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"

#include "hsp.h"

Player* HspPlayer::factory()
{
    return new HspPlayer();
}

bool HspPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if(!f || f.extension() != ".hsp")
        return false;

    // file validation section
    auto filesize = f.size();
    uint16_t orgsize;
    f >> orgsize;
    if(orgsize > 59187)
    {
        return false;
    }

    // load section
    std::vector<uint8_t> cmp(filesize);
    f.seek(0);
    f.read(cmp.data(), cmp.size());

    std::vector<uint8_t> org(orgsize);
    for(int i = 0, j = 0; i < filesize; j += cmp[i], i += 2)
    { // RLE decompress
        if(j >= orgsize)
            break; // memory boundary check
        std::fill_n(org.begin() + j, j + cmp[i] < orgsize ? cmp[i] : orgsize - j - 1, cmp[i + 1]);
    }

    memcpy(instrumentData(), org.data(), 128 * 12); // instruments
    for(int i = 0; i < 128; i++)
    { // correct instruments
        instrumentData()[i][2] ^= (instrumentData()[i][2] & 0x40) << 1;
        instrumentData()[i][3] ^= (instrumentData()[i][3] & 0x40) << 1;
        instrumentData()[i][11] >>= 4; // slide
    }
    for(int i = 0; i < 51; ++i)
        addOrder(org[128 * 12 + i]);
    memcpy(patternData(), org.data() + 128 * 12 + 51, orgsize - 128 * 12 - 51); // patterns

    rewind(0);
    return true;
}