/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2007 Simon Peter <dn.tlp@gmx.net>, et al.

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

  fmc.cpp - FMC Loader by Riven the Mage <riven@ok.ru>
*/

#include <cstring>

#include "stream/filestream.h"

#include "fmc.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CfmcLoader::factory() { return new CfmcLoader(); }

bool CfmcLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
    const unsigned char conv_fx[16] = { 0, 1, 2, 3, 4, 8, 255, 255, 255, 255, 26,
                                        11, 12, 13, 14, 15 };

    // read header
    f.read(header.id, 4);
    f.read(header.title, 21);
    f >> header.numchan;

    // 'FMC!' - signed ?
    if (strncmp(header.id, "FMC!", 4)) {
        return false;
    }

    // init CmodPlayer
    m_order.resize(256);
    realloc_patterns(64, 64, header.numchan);
    init_trackord();

    // load order
    f.read(m_order.data(), 256);

    f.seekrel(2);

    // load instruments
    for (int i = 0; i < 32; i++) {
        f >> instruments[i];
    }

    // load tracks
    int t = 0;
    for (int i = 0; i < 64 && f.pos() < f.size(); i++) {
        for (int j = 0; j < header.numchan; j++) {
            for (int k = 0; k < 64; k++) {
                fmc_event event;

                // read event
                f >> event;

                // convert event
                m_tracks.at(t,k).note = event.byte0 & 0x7F;
                m_tracks.at(t,k).inst =
                        ((event.byte0 & 0x80) >> 3) + (event.byte1 >> 4) + 1;
                m_tracks.at(t,k).command = conv_fx[event.byte1 & 0x0F];
                m_tracks.at(t,k).param1 = event.byte2 >> 4;
                m_tracks.at(t,k).param2 = event.byte2 & 0x0F;

                // fix effects
                if (m_tracks.at(t,k).command == 0x0E) // 0x0E (14): Retrig
                    m_tracks.at(t,k).param1 = 3;
                if (m_tracks.at(t,k).command == 0x1A) { // 0x1A (26): Volume Slide
                    if (m_tracks.at(t,k).param1 > m_tracks.at(t,k).param2) {
                        m_tracks.at(t,k).param1 -= m_tracks.at(t,k).param2;
                        m_tracks.at(t,k).param2 = 0;
                    } else {
                        m_tracks.at(t,k).param2 -= m_tracks.at(t,k).param1;
                        m_tracks.at(t,k).param1 = 0;
                    }
                }
            }

            t++;
        }
    }

    // convert instruments
    for (int i = 0; i < 31; i++)
        buildinst(i);

    // order length
    for (int i = 0; i < 256; i++) {
        if (m_order[i] >= 0xFE) {
            m_length = i;
            break;
        }
    }

    // data for Protracker
    activechan = (0xffffffff >> (32 - header.numchan)) << (32 - header.numchan);
    numberOfPatterns = t / header.numchan;
    m_restartpos = 0;

    // flags
    m_flags = Faust;

    rewind(0);

    return true;
}

size_t CfmcLoader::framesUntilUpdate() { return SampleRate / 50; }

std::string CfmcLoader::gettype() { return std::string("Faust Music Creator"); }

std::string CfmcLoader::gettitle() { return std::string(header.title); }

std::string CfmcLoader::getinstrument(unsigned int n) {
    return std::string(instruments[n].name);
}

unsigned int CfmcLoader::getinstruments() { return 32; }

/* -------- Private Methods ------------------------------- */

void CfmcLoader::buildinst(unsigned char i) {
    CmodPlayer::Instrument& inst = instrument(i,true);
    inst.data[0] = ((instruments[i].synthesis & 1) ^ 1);
    inst.data[0] |= ((instruments[i].feedback & 7) << 1);

    inst.data[3] = ((instruments[i].mod_attack & 15) << 4);
    inst.data[3] |= (instruments[i].mod_decay & 15);
    inst.data[5] = ((15 - (instruments[i].mod_sustain & 15)) << 4);
    inst.data[5] |= (instruments[i].mod_release & 15);
    inst.data[9] = (63 - (instruments[i].mod_volume & 63));
    inst.data[9] |= ((instruments[i].mod_ksl & 3) << 6);
    inst.data[1] = (instruments[i].mod_freq_multi & 15);
    inst.data[7] = (instruments[i].mod_waveform & 3);
    inst.data[1] |= ((instruments[i].mod_sustain_sound & 1) << 5);
    inst.data[1] |= ((instruments[i].mod_ksr & 1) << 4);
    inst.data[1] |= ((instruments[i].mod_vibrato & 1) << 6);
    inst.data[1] |= ((instruments[i].mod_tremolo & 1) << 7);

    inst.data[4] = ((instruments[i].car_attack & 15) << 4);
    inst.data[4] |= (instruments[i].car_decay & 15);
    inst.data[6] = ((15 - (instruments[i].car_sustain & 15)) << 4);
    inst.data[6] |= (instruments[i].car_release & 15);
    inst.data[10] = (63 - (instruments[i].car_volume & 63));
    inst.data[10] |= ((instruments[i].car_ksl & 3) << 6);
    inst.data[2] = (instruments[i].car_freq_multi & 15);
    inst.data[8] = (instruments[i].car_waveform & 3);
    inst.data[2] |= ((instruments[i].car_sustain_sound & 1) << 5);
    inst.data[2] |= ((instruments[i].car_ksr & 1) << 4);
    inst.data[2] |= ((instruments[i].car_vibrato & 1) << 6);
    inst.data[2] |= ((instruments[i].car_tremolo & 1) << 7);

    inst.slide = instruments[i].pitch_shift;
}
