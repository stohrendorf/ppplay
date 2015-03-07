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
 * hsc.cpp - HSC Player by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstring>

#include "stream/filestream.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "hsc.h"
#include "debug.h"

/*** public methods **************************************/

CPlayer *ChscPlayer::factory() { return new ChscPlayer(); }

bool ChscPlayer::load(const std::string &filename) {
  FileStream f(filename);

  // file validation section
  if (!f || f.extension() != ".hsc" || f.size() > 59187) {
    AdPlug_LogWrite("ChscPlayer::load(\"%s\"): Not a HSC file!\n", filename.c_str());
    return false;
  }

  // load section
  f.read((unsigned char *)m_instr, 128*12);
  for (int i = 0; i < 128; i++) { // correct instruments
    m_instr[i][2] ^= (m_instr[i][2] & 0x40) << 1;
    m_instr[i][3] ^= (m_instr[i][3] & 0x40) << 1;
    m_instr[i][11] >>= 4; // slide
  }
  f.read(m_song, 51);
  f.read((HscNote*)m_patterns, 50*64*9);

  rewind(0); // rewind module
  return true;
}

bool ChscPlayer::update() {
  // general vars
  unsigned char chan, pattnr, note, effect, eff_op, inst, vol, Okt, db;
  unsigned short Fnr;
  unsigned long pattoff;

  m_del--; // player speed handling
  if (m_del)
    return !m_songend; // nothing done

  if (m_fadein) // fade-in handling
    m_fadein--;

  pattnr = m_song[m_songpos];
  if (pattnr == 0xff) { // arrangement handling
    m_songend = 1;      // set end-flag
    m_songpos = 0;
    pattnr = m_song[m_songpos];
  } else if ((pattnr & 128) && (pattnr <= 0xb1)) { // goto pattern "nr"
    m_songpos = m_song[m_songpos] & 127;
    m_pattpos = 0;
    pattnr = m_song[m_songpos];
    m_songend = 1;
  }

  pattoff = m_pattpos * 9;
  for (chan = 0; chan < 9; chan++) { // handle all channels
    note = m_patterns[pattnr][pattoff].note;
    effect = m_patterns[pattnr][pattoff].effect;
    pattoff++;

    if (note & 128) { // set instrument
      setinstr(chan, effect);
      continue;
    }
    eff_op = effect & 0x0f;
    inst = m_channel[chan].inst;
    if (note)
      m_channel[chan].slide = 0;

    switch (effect & 0xf0) { // effect handling
    case 0:                  // global effect
      /* The following fx are unimplemented on purpose:
       * 02 - Slide Mainvolume up
       * 03 - Slide Mainvolume down (here: fade in)
       * 04 - Set Mainvolume to 0
       *
       * This is because i've never seen any HSC modules using the fx this
       * way.
       * All modules use the fx the way, i've implemented it.
       */
      switch (eff_op) {
      case 1:
        m_pattbreak++;
        break; // jump to next pattern
      case 3:
        m_fadein = 31;
        break; // fade in (divided by 2)
      case 5:
        m_mode6 = 1;
        break; // 6 voice mode on
      case 6:
        m_mode6 = 0;
        break; // 6 voice mode off
      }
      break;
    case 0x20:
    case 0x10: // manual slides
      if (effect & 0x10) {
        m_channel[chan].freq += eff_op;
        m_channel[chan].slide += eff_op;
      } else {
        m_channel[chan].freq -= eff_op;
        m_channel[chan].slide -= eff_op;
      }
      if (!note)
        setfreq(chan, m_channel[chan].freq);
      break;
    case 0x50: // set percussion instrument (unimplemented)
      break;
    case 0x60: // set feedback
      getOpl()->writeReg(
          0xc0 + chan, (m_instr[m_channel[chan].inst][8] & 1) + (eff_op << 1));
      break;
    case 0xa0: // set carrier volume
      vol = eff_op << 2;
      getOpl()->writeReg(0x43 + s_opTable[chan],
                         vol | (m_instr[m_channel[chan].inst][2] & ~63));
      break;
    case 0xb0: // set modulator volume
      vol = eff_op << 2;
      if (m_instr[inst][8] & 1)
        getOpl()->writeReg(0x40 + s_opTable[chan],
                           vol | (m_instr[m_channel[chan].inst][3] & ~63));
      else
        getOpl()->writeReg(0x40 + s_opTable[chan],
                           vol | (m_instr[inst][3] & ~63));
      break;
    case 0xc0: // set instrument volume
      db = eff_op << 2;
      getOpl()->writeReg(0x43 + s_opTable[chan],
                         db | (m_instr[m_channel[chan].inst][2] & ~63));
      if (m_instr[inst][8] & 1)
        getOpl()->writeReg(0x40 + s_opTable[chan],
                           db | (m_instr[m_channel[chan].inst][3] & ~63));
      break;
    case 0xd0:
      m_pattbreak++;
      m_songpos = eff_op;
      m_songend = 1;
      break;   // position jump
    case 0xf0: // set speed
      m_speed = eff_op;
      m_del = ++m_speed;
      break;
    }

    if (m_fadein) // fade-in volume setting
      setvolume(chan, m_fadein * 2, m_fadein * 2);

    if (!note) // note handling
      continue;
    note--;

    if ((note == 0x7f - 1) || ((note / 12) & ~7)) { // pause (7fh)
      m_adlFreq[chan] &= ~32;
      getOpl()->writeReg(0xb0 + chan, m_adlFreq[chan]);
      continue;
    }

    // play the note
    if (m_mtkmode) // imitate MPU-401 Trakker bug
      note--;
    Okt = ((note / 12) & 7) << 2;
    Fnr = s_noteTable[(note % 12)] + m_instr[inst][11] + m_channel[chan].slide;
    m_channel[chan].freq = Fnr;
    if (!m_mode6 || chan < 6)
      m_adlFreq[chan] = Okt | 32;
    else
      m_adlFreq[chan] = Okt; // never set key for drums
    getOpl()->writeReg(0xb0 + chan, 0);
    setfreq(chan, Fnr);
    if (m_mode6) {
      switch (chan) { // play drums
      case 6:
        getOpl()->writeReg(0xbd, m_bd & ~16);
        m_bd |= 48;
        break; // bass drum
      case 7:
        getOpl()->writeReg(0xbd, m_bd & ~1);
        m_bd |= 33;
        break; // hihat
      case 8:
        getOpl()->writeReg(0xbd, m_bd & ~2);
        m_bd |= 34;
        break; // cymbal
      }
      getOpl()->writeReg(0xbd, m_bd);
    }
  }

  m_del = m_speed;   // player speed-timing
  if (m_pattbreak) { // do post-effect handling
    m_pattpos = 0;   // pattern break!
    m_pattbreak = 0;
    m_songpos++;
    m_songpos %= 50;
    if (!m_songpos)
      m_songend = 1;
  } else {
    m_pattpos++;
    m_pattpos &= 63; // advance in pattern data
    if (!m_pattpos) {
      m_songpos++;
      m_songpos %= 50;
      if (!m_songpos)
        m_songend = 1;
    }
  }
  return !m_songend; // still playing
}

void ChscPlayer::rewind(int) {
  int i; // counter

  // rewind HSC player
  m_pattpos = 0;
  m_songpos = 0;
  m_pattbreak = 0;
  m_speed = 2;
  m_del = 1;
  m_songend = 0;
  m_mode6 = 0;
  m_bd = 0;
  m_fadein = 0;

  getOpl()->writeReg(1, 32);
  getOpl()->writeReg(8, 128);
  getOpl()->writeReg(0xbd, 0);

  for (i = 0; i < 9; i++)
    setinstr((char) i, (char) i); // init channels
}

unsigned int ChscPlayer::getpatterns() {
  unsigned char poscnt, pattcnt = 0;

  // count patterns
  for (poscnt = 0; poscnt < 51 && m_song[poscnt] != 0xff; poscnt++)
    if (m_song[poscnt] > pattcnt)
      pattcnt = m_song[poscnt];

  return (pattcnt + 1);
}

unsigned int ChscPlayer::getorders() {
  unsigned char poscnt;

  // count positions
  for (poscnt = 0; poscnt < 51; poscnt++)
    if (m_song[poscnt] == 0xff)
      break;

  return poscnt;
}

unsigned int ChscPlayer::getinstruments() {
  unsigned char instcnt, instnum = 0, i;
  bool isinst;

  // count instruments
  for (instcnt = 0; instcnt < 128; instcnt++) {
    isinst = false;
    for (i = 0; i < 12; i++)
      if (m_instr[instcnt][i])
        isinst = true;
    if (isinst)
      instnum++;
  }

  return instnum;
}

/*** private methods *************************************/

void ChscPlayer::setfreq(unsigned char chan, unsigned short freq) {
  m_adlFreq[chan] = (m_adlFreq[chan] & ~3) | (freq >> 8);

  getOpl()->writeReg(0xa0 + chan, freq & 0xff);
  getOpl()->writeReg(0xb0 + chan, m_adlFreq[chan]);
}

void ChscPlayer::setvolume(unsigned char chan, int volc, int volm) {
  unsigned char *ins = m_instr[m_channel[chan].inst];
  char op = s_opTable[chan];

  getOpl()->writeReg(0x43 + op, volc | (ins[2] & ~63));
  if (ins[8] & 1) // carrier
    getOpl()->writeReg(0x40 + op, volm | (ins[3] & ~63));
  else
    getOpl()->writeReg(0x40 + op, ins[3]); // modulator
}

void ChscPlayer::setinstr(unsigned char chan, unsigned char insnr) {
  unsigned char *ins = m_instr[insnr];
  char op = s_opTable[chan];

  m_channel[chan].inst = insnr;       // set internal instrument
  getOpl()->writeReg(0xb0 + chan, 0); // stop old note

  // set instrument
  getOpl()->writeReg(0xc0 + chan, ins[8]);
  getOpl()->writeReg(0x23 + op, ins[0]); // carrier
  getOpl()->writeReg(0x20 + op, ins[1]); // modulator
  getOpl()->writeReg(0x63 + op, ins[4]); // bits 0..3 = decay; 4..7 = attack
  getOpl()->writeReg(0x60 + op, ins[5]);
  getOpl()->writeReg(0x83 + op, ins[6]); // 0..3 = release; 4..7 = sustain
  getOpl()->writeReg(0x80 + op, ins[7]);
  getOpl()->writeReg(0xe3 + op, ins[9]); // bits 0..1 = Wellenform
  getOpl()->writeReg(0xe0 + op, ins[10]);
  setvolume(chan, ins[2] & 63, ins[3] & 63);
}
