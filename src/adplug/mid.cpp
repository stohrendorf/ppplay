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
 *
 * MIDI & MIDI-like file player - Last Update: 10/15/2005
 *                  by Phil Hassey - www.imitationpickles.org
 *                                   philhassey@hotmail.com
 *
 * Can play the following
 *      .LAA - a raw save of a Lucas Arts Adlib music
 *             or
 *             a raw save of a LucasFilm Adlib music
 *      .MID - a "midi" save of a Lucas Arts Adlib music
 *           - or general MIDI files
 *      .CMF - Creative Music Format
 *      .SCI - the sierra "midi" format.
 *             Files must be in the form
 *             xxxNAME.sci
 *             So that the loader can load the right patch file:
 *             xxxPATCH.003  (patch.003 must be saved from the
 *                            sierra resource from each game.)
 *
 * 6/2/2000:  v1.0 relased by phil hassey
 *      Status:  LAA is almost perfect
 *                      - some volumes are a bit off (intrument too quiet)
 *               MID is fine (who wants to listen to MIDI vid adlib anyway)
 *               CMF is okay (still needs the adlib rythm mode implemented
 *                            for real)
 * 6/6/2000:
 *      Status:  SCI:  there are two SCI formats, orginal and advanced.
 *                    original:  (Found in SCI/EGA Sierra Adventures)
 *                               played almost perfectly, I believe
 *                               there is one mistake in the instrument
 *                               loader that causes some sounds to
 *                               not be quite right.  Most sounds are fine.
 *                    advanced:  (Found in SCI/VGA Sierra Adventures)
 *                               These are multi-track files.  (Thus the
 *                               player had to be modified to work with
 *                               them.)  This works fine.
 *                               There are also multiple tunes in each file.
 *                               I think some of them are supposed to be
 *                               played at the same time, but I'm not sure
 *                               when.
 * 8/16/2000:
 *      Status:  LAA: now EGA and VGA lucas games work pretty well
 *
 * 10/15/2005: Changes by Simon Peter
 *	Added rhythm mode support for CMF format.
 *
 * 09/13/2008: Changes by Adam Nielsen (malvineous@shikadi.net)
 *      Fixed a couple of CMF rhythm mode bugs
 *      Disabled note velocity for CMF files
 *      Added support for nonstandard CMF AM+VIB controller (for VGFM CMFs)
 *
 * Other acknowledgements:
 *  Allegro - for the midi instruments and the midi volume table
 *  SCUMM Revisited - for getting the .LAA / .MIDs out of those
 *                    LucasArts files.
 *  FreeSCI - for some information on the sci music files
 *  SD - the SCI Decoder (to get all .sci out of the Sierra files)
 */

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include "mid.h"
#include "mididata.h"
#include "stream/filestream.h"

//#define TESTING
#ifdef TESTING
#define midiprintf printf
#else
#define midiprintf(...)
#endif

#define LUCAS_STYLE 1
#define CMF_STYLE 2
#define MIDI_STYLE 4
#define SIERRA_STYLE 8

// AdLib standard operator table
const unsigned char CmidPlayer::adlib_opadd[] = { 0x00, 0x01, 0x02, 0x08, 0x09,
                                                  0x0A, 0x10, 0x11, 0x12 };

// Map CMF drum channels 11 - 15 to corresponding AdLib drum channels
const int CmidPlayer::percussion_map[] = { 6, 7, 8, 8, 7 };

CPlayer *CmidPlayer::factory() { return new CmidPlayer(); }

uint8_t CmidPlayer::datalook(size_t pos) const {
  if (pos >= m_data.size())
    return 0;
  return m_data[pos];
}

uint32_t CmidPlayer::getnexti(size_t num) {
  uint32_t v = 0;
  for (size_t i = 0; i < num; i++) {
    v += (datalook(m_dataPos) << (8 * i));
    m_dataPos++;
  }
  return v;
}

uint32_t CmidPlayer::getnext(size_t num) {
  uint32_t v = 0;

  for (size_t i = 0; i < num; i++) {
    v <<= 8;
    v += datalook(m_dataPos);
    m_dataPos++;
  }
  return v;
}

uint32_t CmidPlayer::getval() {
  auto b = getnext(1);
  auto v = b & 0x7f;
  while ((b & 0x80) != 0) {
    b = getnext(1);
    v = (v << 7) | (b & 0x7F);
  }
  return v;
}

bool CmidPlayer::load_sierra_ins(const std::string &fname,
                                 const CFileProvider &fp) {
  std::string pfilename = fname;
  int j = 0;
  for (int i = pfilename.length() - 1; i >= 0; i--)
    if (pfilename[i] == '/' || pfilename[i] == '\\') {
      j = i + 1;
      break;
    }
  pfilename.erase(j+3);
  pfilename += "patch.003";

  binistream* f = fp.open(pfilename);
  if (!f)
    return false;

  f->ignore(2);
  m_stins = 0;
  for (int i = 0; i < 2; i++) {
    for (int k = 0; k < 48; k++) {
      int l = i * 48 + k;
      midiprintf("\n%2d: ", l);
      unsigned char ins[28];
      for (j = 0; j < 28; j++)
        ins[j] = f->readInt(1);

      m_myInsBank[l][0] = (ins[9] * 0x80) + (ins[10] * 0x40) + (ins[5] * 0x20) +
                          (ins[11] * 0x10) + ins[1]; //1=ins5
      m_myInsBank[l][1] =
          (ins[22] * 0x80) + (ins[23] * 0x40) + (ins[18] * 0x20) +
          (ins[24] * 0x10) + ins[14];                //1=ins18

      m_myInsBank[l][2] = (ins[0] << 6) + ins[8];
      m_myInsBank[l][3] = (ins[13] << 6) + ins[21];

      m_myInsBank[l][4] = (ins[3] << 4) + ins[6];
      m_myInsBank[l][5] = (ins[16] << 4) + ins[19];
      m_myInsBank[l][6] = (ins[4] << 4) + ins[7];
      m_myInsBank[l][7] = (ins[17] << 4) + ins[20];

      m_myInsBank[l][8] = ins[26];
      m_myInsBank[l][9] = ins[27];

      m_myInsBank[l][10] = ((ins[2] << 1)) + (1 - (ins[12] & 1));
      //(ins[12] ? 0:1)+((ins[2]<<1));

      for (j = 0; j < 11; j++)
        midiprintf("%02X ", m_myInsBank[l][j]);
      m_stins++;
    }
    f->ignore(2);
  }

  fp.close(f);
  m_sMyInsBank = m_myInsBank;
  return true;
}

void CmidPlayer::sierra_next_section() {
  int i, j;

  for (i = 0; i < 16; i++)
    m_tracks[i].on = 0;

  midiprintf("\n\nnext adv sierra section:\n");

  m_dataPos = m_sierraPos;
  i = 0;
  j = 0;
  while (i != 0xff) {
    getnext(1);
    m_currentTrack = j;
    j++;
    m_tracks[m_currentTrack].on = 1;
    m_tracks[m_currentTrack].spos = getnext(1);
    m_tracks[m_currentTrack].spos +=
        (getnext(1) << 8) + 4; //4 best usually +3? not 0,1,2 or 5
    //       track[curtrack].spos=getnext(1)+(getnext(1)<<8)+4;		// dynamite!:
    // doesn't optimize correctly!!
    m_tracks[m_currentTrack].tend = m_data.size(); //0xFC will kill it
    m_tracks[m_currentTrack].iwait = 0;
    m_tracks[m_currentTrack].pv = 0;
    midiprintf("track %d starts at %lx\n", m_currentTrack,
               m_tracks[m_currentTrack].spos);

    getnext(2);
    i = getnext(1);
  }
  getnext(2);
  m_deltas = 0x20;
  m_sierraPos = m_dataPos;
  //getch();

  m_fwait = 0;
  m_doing = true;
}

bool CmidPlayer::load(const std::string &filename, const CFileProvider &fp) {
  binistream *f = fp.open(filename);
  if (!f)
    return false;

  unsigned char s[6];
  f->readString(reinterpret_cast<char*>(s), 6);

  FileType good = FileType::Unknown;
  m_subsongs = 0;
  switch (s[0]) {
  case 'A':
    if (s[1] == 'D' && s[2] == 'L')
      good = FileType::Lucas;
    break;
  case 'M':
    if (s[1] == 'T' && s[2] == 'h' && s[3] == 'd')
      good = FileType::Midi;
    break;
  case 'C':
    if (s[1] == 'T' && s[2] == 'M' && s[3] == 'F')
      good = FileType::Cmf;
    break;
  case 0x84:
    if (s[1] == 0x00 && load_sierra_ins(filename, fp)) {
      if (s[2] == 0xf0)
        good = FileType::AdvSierra;
      else
        good = FileType::Sierra;
    }
    break;
  default:
    if (s[4] == 'A' && s[5] == 'D')
      good = FileType::OldLucas;
    break;
  }

  if (good != FileType::Unknown)
    m_subsongs = 1;
  else {
    fp.close(f);
    return false;
  }

  m_type = good;
  f->seek(0);
  m_data.resize(fp.filesize(f));
  f->readString(reinterpret_cast<char*>(m_data.data()), m_data.size());

  fp.close(f);
  rewind(0);
  return true;
}

void CmidPlayer::midi_fm_instrument(int voice, unsigned char *inst) {
  if ((m_adlibStyle & SIERRA_STYLE) != 0)
    getOpl()->writeReg(0xbd, 0); //just gotta make sure this happens..
                                 //'cause who knows when it'll be
                                 //reset otherwise.

  getOpl()->writeReg(0x20 + adlib_opadd[voice], inst[0]);
  getOpl()->writeReg(0x23 + adlib_opadd[voice], inst[1]);

  if (m_adlibStyle & LUCAS_STYLE) {
    getOpl()->writeReg(0x43 + adlib_opadd[voice], 0x3f);
    if ((inst[10] & 1) == 0)
      getOpl()->writeReg(0x40 + adlib_opadd[voice], inst[2]);
    else
      getOpl()->writeReg(0x40 + adlib_opadd[voice], 0x3f);

  } else if ((m_adlibStyle & SIERRA_STYLE) || (m_adlibStyle & CMF_STYLE)) {
    getOpl()->writeReg(0x40 + adlib_opadd[voice], inst[2]);
    getOpl()->writeReg(0x43 + adlib_opadd[voice], inst[3]);

  } else {
    getOpl()->writeReg(0x40 + adlib_opadd[voice], inst[2]);
    if ((inst[10] & 1) == 0)
      getOpl()->writeReg(0x43 + adlib_opadd[voice], inst[3]);
    else
      getOpl()->writeReg(0x43 + adlib_opadd[voice], 0);
  }

  getOpl()->writeReg(0x60 + adlib_opadd[voice], inst[4]);
  getOpl()->writeReg(0x63 + adlib_opadd[voice], inst[5]);
  getOpl()->writeReg(0x80 + adlib_opadd[voice], inst[6]);
  getOpl()->writeReg(0x83 + adlib_opadd[voice], inst[7]);
  getOpl()->writeReg(0xe0 + adlib_opadd[voice], inst[8]);
  getOpl()->writeReg(0xe3 + adlib_opadd[voice], inst[9]);

  getOpl()->writeReg(0xc0 + voice, inst[10]);
}

void CmidPlayer::midi_fm_percussion(int ch, unsigned char *inst) {
  // map CMF drum channels 12 - 15 to corresponding AdLib drum operators
  // bass drum (channel 11) not mapped, cause it's handled like a normal
  // instrument
  static constexpr int map_chan[] = { 0x14, 0x12, 0x15, 0x11 };
  int opadd = map_chan[ch - 12];

  getOpl()->writeReg(0x20 + opadd, inst[0]);
  getOpl()->writeReg(0x40 + opadd, inst[2]);
  getOpl()->writeReg(0x60 + opadd, inst[4]);
  getOpl()->writeReg(0x80 + opadd, inst[6]);
  getOpl()->writeReg(0xe0 + opadd, inst[8]);
  if (opadd < 0x13) // only output this for the modulator, not the carrier, as
                    // it affects the entire channel
    getOpl()->writeReg(0xc0 + percussion_map[ch - 11], inst[10]);
}

void CmidPlayer::midi_fm_volume(int voice, int volume) {
  auto vol = volume;
  if ((m_adlibStyle & SIERRA_STYLE) == 0) //sierra likes it loud!
      {
    vol >>= 2;
  }

  if ((m_adlibStyle & LUCAS_STYLE) != 0) {
    if ((getOpl()->readReg(0xc0 + voice) & 1) == 1)
      getOpl()->writeReg(
          0x40 + adlib_opadd[voice],
          (63 - vol) | (getOpl()->readReg(0x40 + adlib_opadd[voice]) & 0xc0));
    getOpl()->writeReg(
        0x43 + adlib_opadd[voice],
        (63 - vol) | (getOpl()->readReg(0x43 + adlib_opadd[voice]) & 0xc0));
  } else {
    if ((getOpl()->readReg(0xc0 + voice) & 1) == 1)
      getOpl()->writeReg(
          0x40 + adlib_opadd[voice],
          (63 - vol) | (getOpl()->readReg(0x40 + adlib_opadd[voice]) & 0xc0));
    getOpl()->writeReg(
        0x43 + adlib_opadd[voice],
        (63 - vol) | (getOpl()->readReg(0x43 + adlib_opadd[voice]) & 0xc0));
  }
}

void CmidPlayer::midi_fm_playnote(int voice, int note, int volume) {
  note -= 12;
  if (note > 7 * 12 + 11)
    note = 7 * 12 + 11;
  else if (note < 0)
    note = 0;

  static constexpr int fnums[] = { 0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca,
                                   0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287 };
  int freq = fnums[note % 12];
  int oct = note / 12;

  midi_fm_volume(voice, volume);
  getOpl()->writeReg(0xa0 + voice, freq & 0xff);

  auto c = ((freq & 0x300) >> 8) + ((oct & 7) << 2) +
           (m_melodicMode || voice < 6 ? (1 << 5) : 0);
  getOpl()->writeReg(0xb0 + voice, c & 0xff);
}

void CmidPlayer::midi_fm_endnote(int voice) {
  //midi_fm_volume(voice,0);
  //getOpl()->writeReg(0xb0+voice,0);

  getOpl()->writeReg(
      0xb0 + voice,
      getOpl()->readReg(0xb0 + voice) & (255 - 32));
}

void CmidPlayer::midi_fm_reset() {
  int i;

  for (i = 0; i < 256; i++)
    getOpl()->writeReg(i, 0);

  getOpl()->writeReg(0x01, 0x20);
  getOpl()->writeReg(0xBD, 0xc0);
}

bool CmidPlayer::update() {
  midiprintf("update\n");
  long w, v, note, vel, ctrl, nv, x, l, lnum;
  int i = 0, j, c;
  int on, onl, numchan;
  int ret;

  if (m_doing) {
    // just get the first wait and ignore it :>
    for (m_currentTrack = 0; m_currentTrack < 16; m_currentTrack++)
      if (m_tracks[m_currentTrack].on) {
        m_dataPos = m_tracks[m_currentTrack].pos;
        if (m_type != FileType::Sierra && m_type != FileType::AdvSierra)
          m_tracks[m_currentTrack].iwait += getval();
        else
          m_tracks[m_currentTrack].iwait += getnext(1);
        m_tracks[m_currentTrack].pos = m_dataPos;
      }
    m_doing = false;
  }

  m_iwait = 0;
  ret = 1;

  while (m_iwait == 0 && ret == 1) {
    for (m_currentTrack = 0; m_currentTrack < 16; m_currentTrack++)
      if (m_tracks[m_currentTrack].on && m_tracks[m_currentTrack].iwait == 0 &&
          m_tracks[m_currentTrack].pos < m_tracks[m_currentTrack].tend) {
        m_dataPos = m_tracks[m_currentTrack].pos;

        v = getnext(1);

        //  This is to do implied MIDI events.
        if (v < 0x80) {
          v = m_tracks[m_currentTrack].pv;
          m_dataPos--;
        }
        m_tracks[m_currentTrack].pv = v;

        c = v & 0x0f;
        midiprintf("[cmd=%2X]", v);
        switch (v & 0xf0) {
        case 0x80: /*note off*/
          note = getnext(1);
          getnext(1); // vel
          for (i = 0; i < 9; i++)
            if (m_chp[i][0] == c && m_chp[i][1] == note) {
              midi_fm_endnote(i);
              m_chp[i][0] = -1;
            }
          break;
        case 0x90: /*note on*/
                   //  doing=0;
          note = getnext(1);
          vel = getnext(1);

          if (!m_melodicMode)
            numchan = 6;
          else
            numchan = 9;

          if (m_ch[c].on != 0) {
            for (i = 0; i < 18; i++)
              m_chp[i][2]++;

            if (c < 11 || m_melodicMode) {
              j = 0;
              on = -1;
              onl = 0;
              for (i = 0; i < numchan; i++)
                if (m_chp[i][0] == -1 && m_chp[i][2] > onl) {
                  onl = m_chp[i][2];
                  on = i;
                  j = 1;
                }

              if (on == -1) {
                onl = 0;
                for (i = 0; i < numchan; i++)
                  if (m_chp[i][2] > onl) {
                    onl = m_chp[i][2];
                    on = i;
                  }
              }

              if (j == 0)
                midi_fm_endnote(on);
            } else
              on = percussion_map[c - 11];

            if (vel != 0 && m_ch[c].inum >= 0 && m_ch[c].inum < 128) {
              if (m_melodicMode ||
                  c < 12) // 11 == bass drum, handled like a normal
                          // instrument, on == channel 6 thanks to
                          // percussion_map[] above
                midi_fm_instrument(on, m_ch[c].ins);
              else
                midi_fm_percussion(c, m_ch[c].ins);

              if (m_adlibStyle & MIDI_STYLE) {
                nv = ((m_ch[c].vol * vel) / 128);
                if ((m_adlibStyle & LUCAS_STYLE) != 0)
                  nv *= 2;
                if (nv > 127)
                  nv = 127;
                nv = my_midi_fm_vol_table.at(nv);
                if ((m_adlibStyle & LUCAS_STYLE) != 0)
                  nv = static_cast<int>(std::sqrt(float(nv)) * 11);
              } else if (m_adlibStyle & CMF_STYLE) {
                // CMF doesn't support note velocity (even though some files
                // have them!)
                nv = 127;
              } else {
                nv = vel;
              }

              midi_fm_playnote(on, note + m_ch[c].nshift,
                               nv * 2); // sets freq in rhythm mode
              m_chp[on][0] = c;
              m_chp[on][1] = note;
              m_chp[on][2] = 0;

              if (!m_melodicMode && c >= 11) {
                // Still need to turn off the perc instrument before playing
                // it again,
                // as not all songs send a noteoff.
                getOpl()->writeReg(
                    0xbd, getOpl()->readReg(0xbd) & ~(0x10 >> (c - 11)));
                // Play the perc instrument
                getOpl()->writeReg(
                    0xbd, getOpl()->readReg(0xbd) | (0x10 >> (c - 11)));
              }

            } else {
              if (vel == 0) { //same code as end note
                if (!m_melodicMode && c >= 11) {
                  // Turn off the percussion instrument
                  getOpl()->writeReg(
                      0xbd, getOpl()->readReg(0xbd) & ~(0x10 >> (c - 11)));
                  //midi_fm_endnote(percussion_map[c]);
                  m_chp[percussion_map[c - 11]][0] = -1;
                } else {
                  for (i = 0; i < 9; i++) {
                    if (m_chp[i][0] == c && m_chp[i][1] == note) {
                      // midi_fm_volume(i,0);  // really end the note
                      midi_fm_endnote(i);
                      m_chp[i][0] = -1;
                    }
                  }
                }
              } else {
                // i forget what this is for.
                m_chp[on][0] = -1;
                m_chp[on][2] = 0;
              }
            }
            midiprintf(" [%d:%d:%d:%d]\n", c, m_ch[c].inum, note, vel);
          }
          else {
            midiprintf("off");
          }
          break;
        case 0xa0:    /*key after touch */
          getnext(1); // note
          getnext(1); // vel
          break;
        case 0xb0: /*control change .. pitch bend? */
          ctrl = getnext(1);
          vel = getnext(1);

          switch (ctrl) {
          case 0x07:
            midiprintf("(pb:%d: %d %d)", c, ctrl, vel);
            m_ch[c].vol = vel;
            midiprintf("vol");
            break;
          case 0x63:
            if (m_adlibStyle & CMF_STYLE) {
              // Custom extension to allow CMF files to switch the
              // AM+VIB depth on and off (officially this is on,
              // and there's no way to switch it off.)  Controller
              // values:
              //   0 == AM+VIB off
              //   1 == VIB on
              //   2 == AM on
              //   3 == AM+VIB on
              getOpl()->writeReg(
                  0xbd, (getOpl()->readReg(0xbd) & ~0xC0) | (vel << 6));
              midiprintf(" AM+VIB depth change - AM %s, VIB %s\n",
                         (getOpl()->readReg(0xbd) & 0x80) ? "on" : "off",
                         (getOpl()->readReg(0xbd) & 0x40) ? "on" : "off");
            }
            break;
          case 0x67:
            midiprintf("Rhythm mode: %d\n", vel);
            if ((m_adlibStyle & CMF_STYLE) != 0) {
              m_melodicMode = (vel == 0);
              if (!m_melodicMode)
                getOpl()->writeReg(0xbd, getOpl()->readReg(0xbd) | (1 << 5));
              else
                getOpl()->writeReg(0xbd, getOpl()->readReg(0xbd) & ~(1 << 5));
            }
            break;
          }
          break;
        case 0xc0: /*patch change*/
          x = getnext(1);
          m_ch[c].inum = x;
          for (j = 0; j < 11; j++)
            m_ch[c].ins[j] = m_myInsBank[m_ch[c].inum][j];
          break;
        case 0xd0: /*chanel touch*/
          getnext(1);
          break;
        case 0xe0: /*pitch wheel*/
          getnext(1);
          getnext(1);
          break;
        case 0xf0:
          switch (v) {
          case 0xf0:
          case 0xf7: /*sysex*/
            l = getval();
            if (datalook(m_dataPos + l) == 0xf7)
              i = 1;
            midiprintf("{sysex len=%d}", l);
            midiprintf("\n");

            if (datalook(m_dataPos) == 0x7d &&
                datalook(m_dataPos + 1) == 0x10 &&
                datalook(m_dataPos + 2) < 16) {
              m_adlibStyle = LUCAS_STYLE | MIDI_STYLE;
              getnext(1);
              getnext(1);
              c = getnext(1);
              getnext(1);

              //  getnext(22); //temp
              m_ch[c].ins[0] = (getnext(1) << 4) + getnext(1);
              m_ch[c].ins[2] =  0xff - (((getnext(1) << 4) + getnext(1)) & 0x3f);
              m_ch[c].ins[4] = 0xff - ((getnext(1) << 4) + getnext(1));
              m_ch[c].ins[6] = 0xff - ((getnext(1) << 4) + getnext(1));
              m_ch[c].ins[8] = (getnext(1) << 4) + getnext(1);

              m_ch[c].ins[1] = (getnext(1) << 4) + getnext(1);
              m_ch[c].ins[3] =  0xff - (((getnext(1) << 4) + getnext(1)) & 0x3f);
              m_ch[c].ins[5] = 0xff - ((getnext(1) << 4) + getnext(1));
              m_ch[c].ins[7] = 0xff - ((getnext(1) << 4) + getnext(1));
              m_ch[c].ins[9] = (getnext(1) << 4) + getnext(1);

              i = (getnext(1) << 4);
              i += getnext(1);
              m_ch[c].ins[10] = i;

              //if ((i&1)==1) ch[c].ins[10]=1;

              midiprintf("\n%d: ", c);
              for (i = 0; i < 11; i++)
                midiprintf("%2X ", m_ch[c].ins[i]);
              getnext(l - 26);
            } else {
              midiprintf("\n");
              for (j = 0; j < l; j++)
                midiprintf("%2X ", getnext(1));
            }

            midiprintf("\n");
            if (i == 1)
              getnext(1);
            break;
          case 0xf1:
            break;
          case 0xf2:
            getnext(2);
            break;
          case 0xf3:
            getnext(1);
            break;
          case 0xf4:
            break;
          case 0xf5:
            break;
          case 0xf6: /*something*/
          case 0xf8:
          case 0xfa:
          case 0xfb:
          case 0xfc:
            //this ends the track for sierra.
            if (m_type == FileType::Sierra || m_type == FileType::AdvSierra) {
              m_tracks[m_currentTrack].tend = m_dataPos;
              midiprintf("endmark: %ld -- %lx\n", m_dataPos, m_dataPos);
            }
            break;
          case 0xfe:
            break;
          case 0xfd:
            break;
          case 0xff:
            v = getnext(1);
            l = getval();
            midiprintf("\n");
            midiprintf("{%X_%X}", v, l);
            if (v == 0x51) {
              lnum = getnext(l);
              m_msqtr = lnum; /*set tempo*/
              midiprintf("(qtr=%ld)", m_msqtr);
            } else {
              for (i = 0; i < l; i++)
                midiprintf("%2X ", getnext(1));
            }
            break;
          }
          break;
        default:
          midiprintf("!", v); /* if we get down here, a error occurred */
          break;
        }
        if (m_dataPos < m_tracks[m_currentTrack].tend) {
          if (m_type != FileType::Sierra && m_type != FileType::AdvSierra)
            w = getval();
          else
            w = getnext(1);
          m_tracks[m_currentTrack].iwait = w;
          /*
            if (w!=0)
                {
                midiprintf("\n<%d>",w);
                f =
((float)w/(float)deltas)*((float)msqtr/(float)1000000);
                if (doing==1) f=0; //not playing yet. don't wait yet
                }
                */
        } else
          m_tracks[m_currentTrack].iwait = 0;

        m_tracks[m_currentTrack].pos = m_dataPos;
      }

    ret = 0; //end of song.
    m_iwait = 0;
    for (m_currentTrack = 0; m_currentTrack < 16; m_currentTrack++)
      if (m_tracks[m_currentTrack].on == 1 &&
          m_tracks[m_currentTrack].pos < m_tracks[m_currentTrack].tend)
        ret = 1; //not yet..

    if (ret == 1) {
      m_iwait = 0xffffff; // bigger than any wait can be!
      for (m_currentTrack = 0; m_currentTrack < 16; m_currentTrack++)
        if (m_tracks[m_currentTrack].on == 1 &&
            m_tracks[m_currentTrack].pos < m_tracks[m_currentTrack].tend &&
            m_tracks[m_currentTrack].iwait < m_iwait)
          m_iwait = m_tracks[m_currentTrack].iwait;
    }
  }

  if (m_iwait != 0 && ret == 1) {
    for (m_currentTrack = 0; m_currentTrack < 16; m_currentTrack++)
      if (m_tracks[m_currentTrack].on)
        m_tracks[m_currentTrack].iwait -= m_iwait;

    m_fwait = 1.0f / ((float(m_iwait) / m_deltas) * (m_msqtr / 1000000.0));
  }
  else
    m_fwait = 50; // 1/50th of a second

  midiprintf("end update\n");

  if (ret)
    return true;
  else
    return false;
}

size_t CmidPlayer::framesUntilUpdate() {
  return SampleRate / (m_fwait > 0.01 ? m_fwait : 0.01);
}

void CmidPlayer::rewind(int subsong) {
  long o_sierra_pos;
  unsigned char ins[16];

  m_dataPos = 0;
  m_tins = 0;
  m_adlibStyle = MIDI_STYLE | CMF_STYLE;
  m_melodicMode = true;
  m_myInsBank = midi_fm_instruments;
  for (int i = 0; i < 16; i++) {
    m_ch[i].inum = 0;
    for (int j = 0; j < 11; j++)
      m_ch[i].ins[j] = m_myInsBank[m_ch[i].inum][j];
    m_ch[i].vol = 127;
    m_ch[i].nshift = -12;
    m_ch[i].on = 1;
  }

  /* General init */
  for (int i = 0; i < 9; i++) {
    m_chp[i][0] = -1;
    m_chp[i][2] = 0;
  }

  m_deltas = 250; // just a number,  not a standard
  m_msqtr = 500000;
  m_fwait = 123; // gotta be a small thing.. sorta like nothing
  m_iwait = 0;

  m_subsongs = 1;

  for (int i = 0; i < 16; i++) {
    m_tracks[i].tend = 0;
    m_tracks[i].spos = 0;
    m_tracks[i].pos = 0;
    m_tracks[i].iwait = 0;
    m_tracks[i].on = 0;
    m_tracks[i].pv = 0;
  }
  m_currentTrack = 0;

  /* specific to file-type init */

  m_dataPos = 0;
  getnext(1);
  switch (m_type) {
  case FileType::Unknown:
    throw std::runtime_error("Unexpected");
  case FileType::Lucas:
    getnext(24); //skip junk and get to the midi.
    m_adlibStyle = LUCAS_STYLE | MIDI_STYLE;
  //note: no break, we go right into midi headers...
  case FileType::Midi:
    if (m_type != FileType::Lucas)
      m_tins = 128;
    getnext(11); /*skip header*/
    m_deltas = getnext(2);
    midiprintf("deltas:%ld\n", m_deltas);
    getnext(4);

    m_currentTrack = 0;
    m_tracks[m_currentTrack].on = 1;
    m_tracks[m_currentTrack].tend = getnext(4);
    m_tracks[m_currentTrack].spos = m_dataPos;
    midiprintf("tracklen:%ld\n", m_tracks[m_currentTrack].tend);
    break;
  case FileType::Cmf: {
    getnext(3);             // ctmf
    getnexti(2);            //version
    auto n = getnexti(2);   // instrument offset
    auto m = getnexti(2);   // music offset
    m_deltas = getnexti(2); //ticks/qtr note
    m_msqtr = 1000000 / getnexti(2) * m_deltas;
    //the stuff in the cmf is click ticks per second..

    if (auto i = getnexti(2))
      m_title = reinterpret_cast<char*>(m_data.data()) + i;
    if (auto i = getnexti(2))
      m_author = reinterpret_cast<char*>(m_data.data()) + i;
    if (auto i = getnexti(2))
      m_remarks = reinterpret_cast<char*>(m_data.data()) + i;

    getnext(16);     // channel in use table ..
    auto i = getnexti(2); // num instr
    if (i > 128)
      i = 128;   // to ward of bad numbers...
    getnexti(2); //basic tempo

    midiprintf("\nioff:%d\nmoff%d\ndeltas:%ld\nmsqtr:%ld\nnumi:%d\n", n, m,
               m_deltas, m_msqtr, i);
    m_dataPos = n; // jump to instruments
    m_tins = i;
    for (uint32_t j = 0; j < i; j++) {
      midiprintf("\n%d: ", j);
      for (int l = 0; l < 14; l++) {
        m_myInsBank[j][l] = getnext(1);
        midiprintf("%2X ", m_myInsBank[j][l]);
      }
      getnext(2);
    }

    for (i = 0; i < 16; i++)
      m_ch[i].nshift = 0;

    m_adlibStyle = CMF_STYLE;

    m_currentTrack = 0;
    m_tracks[m_currentTrack].on = 1;
    m_tracks[m_currentTrack].tend =
        m_data.size();                 // music until the end of the file
    m_tracks[m_currentTrack].spos = m; //jump to midi music
    break;
  }
  case FileType::OldLucas: {
    m_msqtr = 250000;
    m_dataPos = 9;
    m_deltas = getnext(1);

    m_dataPos = 0x19; // jump to instruments
    m_tins = 8;
    for (int j = 0; j < m_tins; j++) {
      midiprintf("\n%d: ", j);
      for (int l = 0; l < 16; l++)
        ins[l] = getnext(1);

      m_myInsBank[j][10] = ins[2];
      m_myInsBank[j][0] = ins[3];
      m_myInsBank[j][2] = ins[4];
      m_myInsBank[j][4] = ins[5];
      m_myInsBank[j][6] = ins[6];
      m_myInsBank[j][8] = ins[7];
      m_myInsBank[j][1] = ins[8];
      m_myInsBank[j][3] = ins[9];
      m_myInsBank[j][5] = ins[10];
      m_myInsBank[j][7] = ins[11];
      m_myInsBank[j][9] = ins[12];

      for (int l = 0; l < 11; l++)
        midiprintf("%2X ", m_myInsBank[j][l]);
    }

    for (int i = 0; i < 16; i++) {
      if (i < m_tins) {
        m_ch[i].inum = i;
        for (int j = 0; j < 11; j++)
          m_ch[i].ins[j] = m_myInsBank[m_ch[i].inum][j];
      }
    }

    m_adlibStyle = LUCAS_STYLE | MIDI_STYLE;

    m_currentTrack = 0;
    m_tracks[m_currentTrack].on = 1;
    m_tracks[m_currentTrack].tend =
        m_data.size();                    // music until the end of the file
    m_tracks[m_currentTrack].spos = 0x98; //jump to midi music
    break;
  }
  case FileType::AdvSierra: {
    m_myInsBank = m_sMyInsBank;
    m_tins = m_stins;
    m_deltas = 0x20;
    getnext(11); //worthless empty space and "stuff" :)

    o_sierra_pos = m_sierraPos = m_dataPos;
    sierra_next_section();
    while (datalook(m_sierraPos - 2) != 0xff) {
      sierra_next_section();
      m_subsongs++;
    }

    if (subsong < 0 || subsong >= m_subsongs)
      subsong = 0;

    m_sierraPos = o_sierra_pos;
    sierra_next_section();
    auto i = 0;
    while (static_cast<int>(i) != subsong) {
      sierra_next_section();
      i++;
    }

    m_adlibStyle = SIERRA_STYLE | MIDI_STYLE; //advanced sierra tunes use volume
    break;
  }
  case FileType::Sierra:
    m_myInsBank = m_sMyInsBank;
    m_tins = m_stins;
    getnext(2);
    m_deltas = 0x20;

    m_currentTrack = 0;
    m_tracks[m_currentTrack].on = 1;
    m_tracks[m_currentTrack].tend =
        m_data.size(); // music until the end of the file

    for (auto i = 0; i < 16; i++) {
      m_ch[i].nshift = 0;
      m_ch[i].on = getnext(1);
      m_ch[i].inum = getnext(1);
      for (int j = 0; j < 11; j++)
        m_ch[i].ins[j] = m_myInsBank[m_ch[i].inum][j];
    }

    m_tracks[m_currentTrack].spos = m_dataPos;
    m_adlibStyle = SIERRA_STYLE | MIDI_STYLE;
    break;
  }

  /*        sprintf(info,"%s\r\nTicks/Quarter Note: %ld\r\n",info,deltas);
      sprintf(info,"%sms/Quarter Note: %ld",info,msqtr); */

  for (auto i = 0; i < 16; i++)
    if (m_tracks[i].on) {
      m_tracks[i].pos = m_tracks[i].spos;
      m_tracks[i].pv = 0;
      m_tracks[i].iwait = 0;
    }

  m_doing = true;
  midi_fm_reset();
}

std::string CmidPlayer::gettype() {
  switch (m_type) {
  case FileType::Lucas:
    return "LucasArts AdLib MIDI";
  case FileType::Midi:
    return "General MIDI";
  case FileType::Cmf:
    return "Creative Music Format (CMF MIDI)";
  case FileType::OldLucas:
    return "Lucasfilm Adlib MIDI";
  case FileType::AdvSierra:
    return "Sierra On-Line VGA MIDI";
  case FileType::Sierra:
    return "Sierra On-Line EGA MIDI";
  default:
    return "MIDI unknown";
  }
}

bool CDukePlayer::load(const std::string &filename, const CFileProvider &) {
  FileStream fs(filename);
  try {
    m_emidi.reset(new ppp::EMidi(fs, true));
  }
  catch (...) {
    return false;
  }

  return true;
}
