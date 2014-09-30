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
 * Johannes Bjerregaard's JBM Adlib Music Format player for AdPlug
 * Written by Dennis Lindroos <lindroos@nls.fi>, February-March 2007
 * - Designed and coded from scratch (only frequency-table taken from MUSIC.BIN)
 * - The percussion mode is buggy (?) but i'm not good enough to find them
 *   and honestly i think the melodic-mode tunes are much better ;)
 *
 * This version doesn't use the binstr.h functions (coded with custom func.)
 * This is my first attempt on writing a musicplayer for AdPlug, and i'm not
 * coding C++ very often..
 *
 * Released under the terms of the GNU General Public License.
 */

#include "jbm.h"

namespace {
constexpr uint16_t notetable[96] = {
  0x0158, 0x016d, 0x0183, 0x019a, 0x01b2, 0x01cc, 0x01e7, 0x0204, 0x0223,
  0x0244, 0x0266, 0x028b, 0x0558, 0x056d, 0x0583, 0x059a, 0x05b2, 0x05cc,
  0x05e7, 0x0604, 0x0623, 0x0644, 0x0666, 0x068b, 0x0958, 0x096d, 0x0983,
  0x099a, 0x09b2, 0x09cc, 0x09e7, 0x0a04, 0x0a23, 0x0a44, 0x0a66, 0x0a8b,
  0x0d58, 0x0d6d, 0x0d83, 0x0d9a, 0x0db2, 0x0dcc, 0x0de7, 0x0e04, 0x0e23,
  0x0e44, 0x0e66, 0x0e8b, 0x1158, 0x116d, 0x1183, 0x119a, 0x11b2, 0x11cc,
  0x11e7, 0x1204, 0x1223, 0x1244, 0x1266, 0x128b, 0x1558, 0x156d, 0x1583,
  0x159a, 0x15b2, 0x15cc, 0x15e7, 0x1604, 0x1623, 0x1644, 0x1666, 0x168b,
  0x1958, 0x196d, 0x1983, 0x199a, 0x19b2, 0x19cc, 0x19e7, 0x1a04, 0x1a23,
  0x1a44, 0x1a66, 0x1a8b, 0x1d58, 0x1d6d, 0x1d83, 0x1d9a, 0x1db2, 0x1dcc,
  0x1de7, 0x1e04, 0x1e23, 0x1e44, 0x1e66, 0x1e8b
};

constexpr uint8_t percmx_tab[4] = { 0x14, 0x12, 0x15, 0x11 };
constexpr uint8_t perchn_tab[5] = { 6, 7, 8, 8, 7 };
constexpr uint8_t percmaskoff[5] = { 0xef, 0xf7, 0xfb, 0xfd, 0xfe };
constexpr uint8_t percmaskon[5] = { 0x10, 0x08, 0x04, 0x02, 0x01 };

inline uint16_t getWord(const std::vector<uint8_t> &data, int pos) {
  return uint16_t(data[pos + 1] << 8) | data[pos];
}
}

/*** public methods *************************************/

CPlayer *CjbmPlayer::factory() { return new CjbmPlayer(); }

bool CjbmPlayer::load(const std::string &filename, const CFileProvider &fp) {
  binistream *f = fp.open(filename);
  if (!f)
    return false;
  auto filelen = fp.filesize(f);

  if (!filelen || !fp.extension(filename, ".jbm")) {
    fp.close(f);
    return false;
  }

  // Allocate memory buffer m[] and read entire file into it

  m_fileData.resize(filelen);
  if (f->readString(reinterpret_cast<char *>(m_fileData.data()), filelen) !=
      filelen) {
    fp.close(f);
    return false;
  }

  fp.close(f);

  // The known .jbm files always seem to start with the number 0x0002

  if (getWord(m_fileData, 0) != 0x0002)
    return false;

  // Song tempo

  auto i = getWord(m_fileData, 2);
  m_timer = 1193810.0 / (i ? i : 0xffff);

  m_seqTable = getWord(m_fileData, 4);
  m_insTable = getWord(m_fileData, 6);

  // The flags word has atleast 1 bit, the Adlib's rhythm mode, but
  // currently we don't support that :(

  m_flags = getWord(m_fileData, 8);

  // Instrument datas are directly addressed with m[]

  m_insCount = (filelen - m_insTable) >> 4;

  // Voice' and sequence pointers

  m_seqCount = 0xffff;
  for (int i = 0; i < 11; i++) {
    m_voices[i].trkpos = m_voices[i].trkstart =
        getWord(m_fileData, 10 + (i << 1));
    if (m_voices[i].trkpos && m_voices[i].trkpos < m_seqCount)
      m_seqCount = m_voices[i].trkpos;
  }
  m_seqCount = (m_seqCount - m_seqTable) >> 1;
  m_sequences.resize(m_seqCount);
  for (int i = 0; i < m_seqCount; i++)
    m_sequences[i] = getWord(m_fileData, m_seqTable + (i << 1));

  rewind(0);
  return true;
}

bool CjbmPlayer::update() {
  short c, spos, frq;

  for (c = 0; c < 11; c++) {
    if (!m_voices[c].trkpos) // Unused channel
      continue;

    if (--m_voices[c].delay)
      continue;

    // Turn current note/percussion off

    if (m_voices[c].note & 0x7f)
      opl_noteonoff(c, 0);

    // Process events until we have a note

    spos = m_voices[c].seqpos;
    while (!m_voices[c].delay) {
      switch (m_fileData[spos]) {
      case 0xFD: // Set Instrument
        m_voices[c].instr = m_fileData[spos + 1];
        set_opl_instrument(c, &m_voices[c]);
        spos += 2;
        break;
      case 0xFF: // End of Sequence
        m_voices[c].seqno = m_fileData[++m_voices[c].trkpos];
        if (m_voices[c].seqno == 0xff) {
          m_voices[c].trkpos = m_voices[c].trkstart;
          m_voices[c].seqno = m_fileData[m_voices[c].trkpos];
          //voicemask &= 0x7ff-(1<<c);
          m_voicemask &= ~(1 << c);
        }
        spos = m_voices[c].seqpos = m_sequences[m_voices[c].seqno];
        break;
      default: // Note Event
        if ((m_fileData[spos] & 127) > 95)
          return 0;

        m_voices[c].note = m_fileData[spos];
        m_voices[c].vol = m_fileData[spos + 1];
        m_voices[c].delay =
            (m_fileData[spos + 2] + (m_fileData[spos + 3] << 8)) + 1;

        frq = notetable[m_voices[c].note & 127];
        m_voices[c].frq[0] = uint8_t(frq);
        m_voices[c].frq[1] = frq >> 8;
        spos += 4;
      }
    }
    m_voices[c].seqpos = spos;

    // Write new volume to the carrier operator, or percussion

    if (m_flags & 1 && c > 6)
      getOpl()->writeReg(0x40 + percmx_tab[c - 7], m_voices[c].vol ^ 0x3f);
    else
      getOpl()->writeReg(0x43 + m_opTable[c], m_voices[c].vol ^ 0x3f);

    // Write new frequencies and Gate bit

    opl_noteonoff(c, !(m_voices[c].note & 0x80));
  }
  return (m_voicemask);
}

void CjbmPlayer::rewind(int) {
  int c;

  m_voicemask = 0;

  for (c = 0; c < 11; c++) {
    m_voices[c].trkpos = m_voices[c].trkstart;

    if (!m_voices[c].trkpos)
      continue;

    m_voicemask |= (1 << c);

    m_voices[c].seqno = m_fileData[m_voices[c].trkpos];
    m_voices[c].seqpos = m_sequences[m_voices[c].seqno];

    m_voices[c].note = 0;
    m_voices[c].delay = 1;
  }

  getOpl()->writeReg(0x01, 32);

  // Set rhythm mode if flags bit #0 is set
  // AM and Vibrato are full depths (taken from DosBox RAW output)
  getOpl()->writeReg(0xbd, 0xC0 | (m_flags & 1) << 5);

#if 0
  if (flags & 1) {
    voice[7].frq[0] = 0x58;
    voice[7].frq[1] = 0x09; // XXX
    voice[8].frq[0] = 0x04;
    voice[8].frq[1] = 0x0a; // XXX
    opl_noteonoff(7, &voice[7], 0);
    opl_noteonoff(8, &voice[8], 0);
  }
#endif

  return;
}

/*** private methods ************************************/

void CjbmPlayer::opl_noteonoff(int channel, bool state) {
  if (m_flags & 1 && channel > 5) {
    // Percussion
    getOpl()->writeReg(0xa0 + perchn_tab[channel - 6],
                       m_voices[channel].frq[0]);
    getOpl()->writeReg(0xb0 + perchn_tab[channel - 6],
                       m_voices[channel].frq[1]);
    getOpl()->writeReg(
        0xbd, state ? getOpl()->readReg(0xbd) | percmaskon[channel - 6]
                    : getOpl()->readReg(0xbd) & percmaskoff[channel - 6]);
  } else {
    // Melodic mode or Rhythm mode melodic channels
    getOpl()->writeReg(0xa0 + channel, m_voices[channel].frq[0]);
    getOpl()->writeReg(0xb0 + channel, state ? m_voices[channel].frq[1] | 0x20
                                             : m_voices[channel].frq[1] & 0x1f);
  }
  return;
}

void CjbmPlayer::set_opl_instrument(int channel, JBMVoice *voice) {
  const auto filePos = m_insTable + (voice->instr << 4);

  // Sanity check on instr number - or we'll be reading outside m[] !

  if (voice->instr >= m_insCount)
    return;

  // For rhythm mode, multiplexed drums. I don't care about waveforms!
  if ((m_flags & 1) && (channel > 6)) {
    getOpl()->writeReg(0x20 + percmx_tab[channel - 7], m_fileData[filePos + 0]);
    getOpl()->writeReg(0x40 + percmx_tab[channel - 7],
                       m_fileData[filePos + 1] ^ 0x3f);
    getOpl()->writeReg(0x60 + percmx_tab[channel - 7], m_fileData[filePos + 2]);
    getOpl()->writeReg(0x80 + percmx_tab[channel - 7], m_fileData[filePos + 3]);

    getOpl()->writeReg(0xc0 + perchn_tab[channel - 6],
                       m_fileData[filePos + 8] & 15);
    return;
  }

  // AM/VIB/EG/KSR/FRQMUL, KSL/OUTPUT, ADSR for 1st operator
  getOpl()->writeReg(0x20 + m_opTable[channel], m_fileData[filePos + 0]);
  getOpl()->writeReg(0x40 + m_opTable[channel], m_fileData[filePos + 1] ^ 0x3f);
  getOpl()->writeReg(0x60 + m_opTable[channel], m_fileData[filePos + 2]);
  getOpl()->writeReg(0x80 + m_opTable[channel], m_fileData[filePos + 3]);

  // AM/VIB/EG/KSR/FRQMUL, KSL/OUTPUT, ADSR for 2nd operator
  getOpl()->writeReg(0x23 + m_opTable[channel], m_fileData[filePos + 4]);
  getOpl()->writeReg(0x43 + m_opTable[channel], m_fileData[filePos + 5] ^ 0x3f);
  getOpl()->writeReg(0x63 + m_opTable[channel], m_fileData[filePos + 6]);
  getOpl()->writeReg(0x83 + m_opTable[channel], m_fileData[filePos + 7]);

  // WAVEFORM for operators
  getOpl()->writeReg(0xe0 + m_opTable[channel],
                     (m_fileData[filePos + 8] >> 4) & 3);
  getOpl()->writeReg(0xe3 + m_opTable[channel],
                     (m_fileData[filePos + 8] >> 6) & 3);

  // FEEDBACK/FM mode
  getOpl()->writeReg(0xc0 + channel, m_fileData[filePos + 8] & 15);

  return;
}
