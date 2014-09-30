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
#include "fmc.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CfmcLoader::factory() { return new CfmcLoader(); }

bool CfmcLoader::load(const std::string &filename, const CFileProvider &fp) {
  binistream *f = fp.open(filename);
  if (!f)
    return false;
  const unsigned char conv_fx[16] = { 0, 1, 2, 3, 4, 8, 255, 255, 255, 255, 26,
                                      11, 12, 13, 14, 15 };

  int i, j, k, t = 0;

  // read header
  f->readString(header.id, 4);
  f->readString(header.title, 21);
  header.numchan = f->readInt(1);

  // 'FMC!' - signed ?
  if (strncmp(header.id, "FMC!", 4)) {
    fp.close(f);
    return false;
  }

  // init CmodPlayer
  realloc_instruments(32);
  realloc_order(256);
  realloc_patterns(64, 64, header.numchan);
  init_trackord();

  // load order
  for (i = 0; i < 256; i++)
    m_order[i] = f->readInt(1);

  f->ignore(2);

  // load instruments
  for (i = 0; i < 32; i++) {
    instruments[i].synthesis = f->readInt(1);
    instruments[i].feedback = f->readInt(1);

    instruments[i].mod_attack = f->readInt(1);
    instruments[i].mod_decay = f->readInt(1);
    instruments[i].mod_sustain = f->readInt(1);
    instruments[i].mod_release = f->readInt(1);
    instruments[i].mod_volume = f->readInt(1);
    instruments[i].mod_ksl = f->readInt(1);
    instruments[i].mod_freq_multi = f->readInt(1);
    instruments[i].mod_waveform = f->readInt(1);
    instruments[i].mod_sustain_sound = f->readInt(1);
    instruments[i].mod_ksr = f->readInt(1);
    instruments[i].mod_vibrato = f->readInt(1);
    instruments[i].mod_tremolo = f->readInt(1);

    instruments[i].car_attack = f->readInt(1);
    instruments[i].car_decay = f->readInt(1);
    instruments[i].car_sustain = f->readInt(1);
    instruments[i].car_release = f->readInt(1);
    instruments[i].car_volume = f->readInt(1);
    instruments[i].car_ksl = f->readInt(1);
    instruments[i].car_freq_multi = f->readInt(1);
    instruments[i].car_waveform = f->readInt(1);
    instruments[i].car_sustain_sound = f->readInt(1);
    instruments[i].car_ksr = f->readInt(1);
    instruments[i].car_vibrato = f->readInt(1);
    instruments[i].car_tremolo = f->readInt(1);

    instruments[i].pitch_shift = f->readInt(1);

    f->readString(instruments[i].name, 21);
  }

  // load tracks
  for (i = 0; i < 64; i++) {
    if (f->ateof())
      break;

    for (j = 0; j < header.numchan; j++) {
      for (k = 0; k < 64; k++) {
        fmc_event event;

        // read event
        event.byte0 = f->readInt(1);
        event.byte1 = f->readInt(1);
        event.byte2 = f->readInt(1);

        // convert event
        m_tracks[t][k].note = event.byte0 & 0x7F;
        m_tracks[t][k].inst =
            ((event.byte0 & 0x80) >> 3) + (event.byte1 >> 4) + 1;
        m_tracks[t][k].command = conv_fx[event.byte1 & 0x0F];
        m_tracks[t][k].param1 = event.byte2 >> 4;
        m_tracks[t][k].param2 = event.byte2 & 0x0F;

        // fix effects
        if (m_tracks[t][k].command == 0x0E) // 0x0E (14): Retrig
          m_tracks[t][k].param1 = 3;
        if (m_tracks[t][k].command == 0x1A) { // 0x1A (26): Volume Slide
          if (m_tracks[t][k].param1 > m_tracks[t][k].param2) {
            m_tracks[t][k].param1 -= m_tracks[t][k].param2;
            m_tracks[t][k].param2 = 0;
          } else {
            m_tracks[t][k].param2 -= m_tracks[t][k].param1;
            m_tracks[t][k].param1 = 0;
          }
        }
      }

      t++;
    }
  }
  fp.close(f);

  // convert instruments
  for (i = 0; i < 31; i++)
    buildinst(i);

  // order length
  for (i = 0; i < 256; i++) {
    if (m_order[i] >= 0xFE) {
      m_length = i;
      break;
    }
  }

  // data for Protracker
  activechan = (0xffffffff >> (32 - header.numchan)) << (32 - header.numchan);
  nop = t / header.numchan;
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
  m_instruments[i].data[0] = ((instruments[i].synthesis & 1) ^ 1);
  m_instruments[i].data[0] |= ((instruments[i].feedback & 7) << 1);

  m_instruments[i].data[3] = ((instruments[i].mod_attack & 15) << 4);
  m_instruments[i].data[3] |= (instruments[i].mod_decay & 15);
  m_instruments[i].data[5] = ((15 - (instruments[i].mod_sustain & 15)) << 4);
  m_instruments[i].data[5] |= (instruments[i].mod_release & 15);
  m_instruments[i].data[9] = (63 - (instruments[i].mod_volume & 63));
  m_instruments[i].data[9] |= ((instruments[i].mod_ksl & 3) << 6);
  m_instruments[i].data[1] = (instruments[i].mod_freq_multi & 15);
  m_instruments[i].data[7] = (instruments[i].mod_waveform & 3);
  m_instruments[i].data[1] |= ((instruments[i].mod_sustain_sound & 1) << 5);
  m_instruments[i].data[1] |= ((instruments[i].mod_ksr & 1) << 4);
  m_instruments[i].data[1] |= ((instruments[i].mod_vibrato & 1) << 6);
  m_instruments[i].data[1] |= ((instruments[i].mod_tremolo & 1) << 7);

  m_instruments[i].data[4] = ((instruments[i].car_attack & 15) << 4);
  m_instruments[i].data[4] |= (instruments[i].car_decay & 15);
  m_instruments[i].data[6] = ((15 - (instruments[i].car_sustain & 15)) << 4);
  m_instruments[i].data[6] |= (instruments[i].car_release & 15);
  m_instruments[i].data[10] = (63 - (instruments[i].car_volume & 63));
  m_instruments[i].data[10] |= ((instruments[i].car_ksl & 3) << 6);
  m_instruments[i].data[2] = (instruments[i].car_freq_multi & 15);
  m_instruments[i].data[8] = (instruments[i].car_waveform & 3);
  m_instruments[i].data[2] |= ((instruments[i].car_sustain_sound & 1) << 5);
  m_instruments[i].data[2] |= ((instruments[i].car_ksr & 1) << 4);
  m_instruments[i].data[2] |= ((instruments[i].car_vibrato & 1) << 6);
  m_instruments[i].data[2] |= ((instruments[i].car_tremolo & 1) << 7);

  m_instruments[i].slide = instruments[i].pitch_shift;
}
