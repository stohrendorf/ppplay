/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  mad.cpp - MAD loader by Riven the Mage <riven@ok.ru>
*/

#include <cstring>

#include "stream/filestream.h"

#include "mad.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CmadLoader::factory() { return new CmadLoader(); }

bool CmadLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
    const unsigned char conv_inst[10] = { 2, 1, 10, 9, 4, 3, 6, 5, 8, 7 };
    // 'MAD+' - signed ?
    char id[4];
    f.read(id, 4);
    if (strncmp(id, "MAD+", 4)) {
        return false;
    }

    // load instruments
    for (int i = 0; i < 9; i++) {
        f.read(instruments[i].name, 8);
        f.read(instruments[i].data, 12);
    }

    f.seekrel(1);

    // data for Protracker
    uint8_t tmp;
    f >> tmp;
    m_length = tmp;
    f >> tmp;
    numberOfPatterns = tmp;
    f >> tmp;
    timer = tmp;

    // init CmodPlayer
    m_order.resize(m_length);
    realloc_patterns(numberOfPatterns, 32, 9);
    init_trackord();

    // load tracks
    for (auto i = 0; i < numberOfPatterns; i++) {
        for (auto k = 0; k < 32; k++) {
            for (auto j = 0; j < 9; j++) {
                auto t = i * 9 + j;

                // read event
                uint8_t event;
                f >> event;

                // convert event
                if (event < 0x61)
                    m_tracks.at(t,k).note = event;
                if (event == 0xFF) // 0xFF: Release note
                    m_tracks.at(t,k).command = 8;
                if (event == 0xFE) // 0xFE: Pattern Break
                    m_tracks.at(t,k).command = 13;
            }
        }
    }

    // load order
    for (auto i = 0; i < m_length; i++) {
        uint8_t tmp;
        f >> tmp;
        m_order[i] = tmp - 1;
    }

    // convert instruments
    for (auto i = 0; i < 9; i++)
        for (auto j = 0; j < 10; j++)
            instrument(i,true).data[conv_inst[j]] = instruments[i].data[j];

    // data for Protracker
    m_restartpos = 0;
    m_initspeed = 1;

    rewind(0);
    return true;
}

void CmadLoader::rewind(int subsong) {
    CmodPlayer::rewind(subsong);

    // default instruments
    for (int i = 0; i < 9; i++) {
        channel[i].inst = i;

        const CmodPlayer::Instrument& inst = instrument(i);
        channel[i].vol1 = 63 - (inst.data[10] & 63);
        channel[i].vol2 = 63 - (inst.data[9] & 63);
    }
}

size_t CmadLoader::framesUntilUpdate() { return SampleRate / timer; }

std::string CmadLoader::gettype() { return std::string("Mlat Adlib Tracker"); }

std::string CmadLoader::getinstrument(unsigned int n) {
    return std::string(instruments[n].name, 8);
}

unsigned int CmadLoader::getinstruments() { return 9; }
