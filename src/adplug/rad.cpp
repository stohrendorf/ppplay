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
 * rad.cpp - RAD Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * BUGS:
 * some volumes are dropped out
 */

#include <cstring>
#include "rad.h"

CPlayer *CradLoader::factory() { return new CradLoader(); }

bool CradLoader::load(const std::string &filename, const CFileProvider &fp) {
  binistream *f = fp.open(filename);
  if (!f)
    return false;

  // file validation section
  char id[16];
  f->readString(id, 16);
  m_version = f->readInt(1);
  if (strncmp(id, "RAD by REALiTY!!", 16) || m_version != 0x10) {
    fp.close(f);
    return false;
  }

  // load section
  m_radflags = f->readInt(1);
  if (m_radflags & 128) { // description
    m_desc.clear();
    for (uint8_t buf = f->readInt(1); buf != 0; buf = f->readInt(1)) {
      if (buf == 1)
        m_desc += "\n";
      else if (buf >= 2 && buf <= 0x1f)
        for (int i = 0; i < buf; i++)
          m_desc += " ";
      else {
        m_desc += char(buf);
      }
    }
  }
  for (uint8_t buf = f->readInt(1); buf != 0;
       buf = f->readInt(1)) { // instruments
    buf--;
    m_instruments[buf].data[2] = f->readInt(1);
    m_instruments[buf].data[1] = f->readInt(1);
    m_instruments[buf].data[10] = f->readInt(1);
    m_instruments[buf].data[9] = f->readInt(1);
    m_instruments[buf].data[4] = f->readInt(1);
    m_instruments[buf].data[3] = f->readInt(1);
    m_instruments[buf].data[6] = f->readInt(1);
    m_instruments[buf].data[5] = f->readInt(1);
    m_instruments[buf].data[0] = f->readInt(1);
    m_instruments[buf].data[8] = f->readInt(1);
    m_instruments[buf].data[7] = f->readInt(1);
  }
  m_length = f->readInt(1);
  for (unsigned long i = 0; i < m_length; i++)
    m_order[i] = f->readInt(1); // orderlist
  uint16_t patofs[32];
  for (int i = 0; i < 32; i++)
    patofs[i] = f->readInt(2); // pattern offset table
  init_trackord();             // patterns
  for (int i = 0; i < 32; i++) {
    if (patofs[i]) {
      f->seek(patofs[i]);
      while (true) {
        uint8_t buf = f->readInt(1);
        uint8_t b = buf & 127;
        while (true) {
          uint8_t ch = f->readInt(1);
          uint8_t c = ch & 127;
          uint8_t inp = f->readInt(1);
          m_tracks[i * 9 + c][b].note = inp & 127;
          m_tracks[i * 9 + c][b].inst = (inp & 128) >> 3;
          inp = f->readInt(1);
          m_tracks[i * 9 + c][b].inst += inp >> 4;
          m_tracks[i * 9 + c][b].command = inp & 15;
          if (inp & 15) {
            inp = f->readInt(1);
            m_tracks[i * 9 + c][b].param1 = inp / 10;
            m_tracks[i * 9 + c][b].param2 = inp % 10;
          }
          if (ch & 0x80)
            break;
        }
        if (buf & 0x80)
          break;
      }
    }
    else
      std::fill_n(trackord[i], 9, 0);
  }
  fp.close(f);

  // convert replay data
  for (int i = 0; i < 32 * 9; i++) // convert patterns
    for (int j = 0; j < 64; j++) {
      if (m_tracks[i][j].note == 15)
        m_tracks[i][j].note = 127;
      if (m_tracks[i][j].note > 16 && m_tracks[i][j].note < 127)
        m_tracks[i][j].note -= 4 * (m_tracks[i][j].note >> 4);
      if (m_tracks[i][j].note && m_tracks[i][j].note < 126)
        m_tracks[i][j].note++;
      static constexpr uint8_t convfx[16] = { 255, 1, 2, 3, 255, 5, 255, 255,
                                              255, 255, 20, 255, 17, 0xd, 255,
                                              19 };
      m_tracks[i][j].command = convfx[m_tracks[i][j].command];
    }
  m_restartpos = 0;
  m_initspeed = m_radflags & 31;
  m_bpm = m_radflags & 64 ? 0 : 50;
  m_flags = Decimal;

  rewind(0);
  return true;
}

size_t CradLoader::framesUntilUpdate() {
  if (m_tempo)
    return SampleRate / m_tempo;
  else
    return SampleRate / 18.2;
}
