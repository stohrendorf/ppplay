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
 * protrack.cpp - Generic Protracker Player
 *
 * NOTES:
 * This is a generic Protracker-based formats player. It offers all Protracker
 * features, plus a good set of extensions to be compatible to other Protracker
 * derivatives. It is derived from the former SA2 player. If you got a
 * Protracker-like format, this is most certainly the player you want to use.
 */

#include <cstring>
#include "protrack.h"
#include "debug.h"

#define SPECIALARPLEN 256 // Standard length of special arpeggio lists
#define JUMPMARKER 0x80   // Orderlist jump marker

// SA2 compatible adlib note table
const std::array<uint16_t,12> sa2_notetable = { 340, 363, 385, 408, 432,
                                                458, 485, 514, 544, 577,
                                                611, 647 };

// SA2 compatible vibrato rate table
const unsigned char CmodPlayer::vibratotab[32] = { 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                   10, 11, 12, 13, 14, 15, 16,
                                                   16, 15, 14, 13, 12, 11, 10,
                                                   9, 8, 7, 6, 5, 4, 3, 2, 1 };

/*** public methods *************************************/

CmodPlayer::CmodPlayer() {
  m_order.resize(128);
  realloc_patterns(64, 64, 9);
  m_instruments.resize(250);
  init_notetable(sa2_notetable);
}

bool CmodPlayer::update() {
  unsigned char pattbreak = 0, donote, pattnr, chan, oplchan, info1, info2,
                info, pattern_delay;
  unsigned short track;
  unsigned long row;

  if (!speed) // song full stop
    return !songend;

  // effect handling (timer dependant)
  for (chan = 0; chan < channel.size(); chan++) {
    oplchan = set_opl_chip(chan);

    if (arplist.is_initialized() && arpcmd.is_initialized() &&
        m_instruments[channel[chan].inst].arpstart) { // special arpeggio
      if (channel[chan].arpspdcnt)
        channel[chan].arpspdcnt--;
      else if ((*arpcmd)[channel[chan].arppos] != 255) {
        switch ((*arpcmd)[channel[chan].arppos]) {
        case 252:
          channel[chan].vol1 = (*arplist)[channel[chan].arppos]; // set volume
          if (channel[chan].vol1 > 63)                        // ?????
            channel[chan].vol1 = 63;
          channel[chan].vol2 = channel[chan].vol1;
          setvolume(chan);
          break;
        case 253:
          channel[chan].key = 0;
          setfreq(chan);
          break; // release sustaining note
        case 254:
          channel[chan].arppos = (*arplist)[channel[chan].arppos];
          break; // arpeggio loop
        default:
          if ((*arpcmd)[channel[chan].arppos]) {
            if ((*arpcmd)[channel[chan].arppos] / 10)
              getOpl()->writeReg(0xe3 + m_opTable[oplchan],
                                 (*arpcmd)[channel[chan].arppos] / 10 - 1);
            if ((*arpcmd)[channel[chan].arppos] % 10)
              getOpl()->writeReg(0xe0 + m_opTable[oplchan],
                                 ((*arpcmd)[channel[chan].arppos] % 10) - 1);
            if ((*arpcmd)[channel[chan].arppos] < 10) // ?????
              getOpl()->writeReg(0xe0 + m_opTable[oplchan],
                                 (*arpcmd)[channel[chan].arppos] - 1);
          }
        }
        if ((*arpcmd)[channel[chan].arppos] != 252) {
          if ((*arplist)[channel[chan].arppos] <= 96)
            setnote(chan, channel[chan].note + (*arplist)[channel[chan].arppos]);
          if ((*arplist)[channel[chan].arppos] >= 100)
            setnote(chan, (*arplist)[channel[chan].arppos] - 100);
        } else
          setnote(chan, channel[chan].note);
        setfreq(chan);
        if ((*arpcmd)[channel[chan].arppos] != 255)
          channel[chan].arppos++;
        channel[chan].arpspdcnt =
            m_instruments[channel[chan].inst].arpspeed - 1;
      }
    }

    info1 = channel[chan].info1;
    info2 = channel[chan].info2;
    if (m_flags & Decimal)
      info = channel[chan].info1 * 10 + channel[chan].info2;
    else
      info = (channel[chan].info1 << 4) + channel[chan].info2;
    switch (channel[chan].fx) {
    case 0:
      if (info) { // arpeggio
        if (channel[chan].trigger < 2)
          channel[chan].trigger++;
        else
          channel[chan].trigger = 0;
        switch (channel[chan].trigger) {
        case 0:
          setnote(chan, channel[chan].note);
          break;
        case 1:
          setnote(chan, channel[chan].note + info1);
          break;
        case 2:
          setnote(chan, channel[chan].note + info2);
        }
        setfreq(chan);
      }
      break;
    case 1:
      slide_up(chan, info);
      setfreq(chan);
      break; // slide up
    case 2:
      slide_down(chan, info);
      setfreq(chan);
      break; // slide down
    case 3:
      tone_portamento(chan, channel[chan].portainfo);
      break; // tone portamento
    case 4:
      vibrato(chan, channel[chan].vibinfo1, channel[chan].vibinfo2);
      break; // vibrato
    case 5:  // tone portamento & volume slide
    case 6:
      if (channel[chan].fx == 5) // vibrato & volume slide
        tone_portamento(chan, channel[chan].portainfo);
      else
        vibrato(chan, channel[chan].vibinfo1, channel[chan].vibinfo2);
    case 10:
      if (del % 4) // SA2 volume slide
        break;
      if (info1)
        vol_up(chan, info1);
      else
        vol_down(chan, info2);
      setvolume(chan);
      break;
    case 14:
      if (info1 == 3) // retrig note
        if (!(del % (info2 + 1)))
          playnote(chan);
      break;
    case 16:
      if (del % 4) // AMD volume slide
        break;
      if (info1)
        vol_up_alt(chan, info1);
      else
        vol_down_alt(chan, info2);
      setvolume(chan);
      break;
    case 20: // RAD volume slide
      if (info < 50)
        vol_down_alt(chan, info);
      else
        vol_up_alt(chan, info - 50);
      setvolume(chan);
      break;
    case 26: // volume slide
      if (info1)
        vol_up(chan, info1);
      else
        vol_down(chan, info2);
      setvolume(chan);
      break;
    case 28:
      if (info1) {
        slide_up(chan, 1);
        channel[chan].info1--;
      }
      if (info2) {
        slide_down(chan, 1);
        channel[chan].info2--;
      }
      setfreq(chan);
      break;
    }
  }

  if (del) { // speed compensation
    del--;
    return !songend;
  }

  // arrangement handling
  if (!resolve_order())
    return !songend;
  pattnr = m_order[ord];

  if (!rw)
    AdPlug_LogWrite("\nCmodPlayer::update(): Pattern: %d, Order: %d\n", pattnr,
                    ord);
  AdPlug_LogWrite("CmodPlayer::update():%3d|", rw);

  // play row
  pattern_delay = 0;
  row = rw;
  for (chan = 0; chan < channel.size(); chan++) {
    oplchan = set_opl_chip(chan);

    if (!(activechan >> (31 - chan)) & 1) { // channel active?
      AdPlug_LogWrite("N/A|");
      continue;
    }
    if (!(track = trackord[pattnr][chan])) { // resolve track
      AdPlug_LogWrite("------------|");
      continue;
    } else
      track--;

    AdPlug_LogWrite("%3d%3d%2X%2X%2X|", m_tracks[track][row].note,
                    m_tracks[track][row].inst, m_tracks[track][row].command,
                    m_tracks[track][row].param1, m_tracks[track][row].param2);

    donote = 0;
    if (m_tracks[track][row].inst) {
      channel[chan].inst = m_tracks[track][row].inst - 1;
      if (!(m_flags & Faust)) {
        channel[chan].vol1 =
            63 - (m_instruments[channel[chan].inst].data[10] & 63);
        channel[chan].vol2 =
            63 - (m_instruments[channel[chan].inst].data[9] & 63);
        setvolume(chan);
      }
    }

    if (m_tracks[track][row].note &&
        m_tracks[track][row].command != 3) { // no tone portamento
      channel[chan].note = m_tracks[track][row].note;
      setnote(chan, m_tracks[track][row].note);
      channel[chan].nextfreq = channel[chan].freq;
      channel[chan].nextoct = channel[chan].oct;
      channel[chan].arppos = m_instruments[channel[chan].inst].arpstart;
      channel[chan].arpspdcnt = 0;
      if (m_tracks[track][row].note != 127) // handle key off
        donote = 1;
    }
    channel[chan].fx = m_tracks[track][row].command;
    channel[chan].info1 = m_tracks[track][row].param1;
    channel[chan].info2 = m_tracks[track][row].param2;

    if (donote)
      playnote(chan);

    // command handling (row dependant)
    info1 = channel[chan].info1;
    info2 = channel[chan].info2;
    if (m_flags & Decimal)
      info = channel[chan].info1 * 10 + channel[chan].info2;
    else
      info = (channel[chan].info1 << 4) + channel[chan].info2;
    switch (channel[chan].fx) {
    case 3: // tone portamento
      if (m_tracks[track][row].note) {
        if (m_tracks[track][row].note < 13)
          channel[chan].nextfreq = notetable[m_tracks[track][row].note - 1];
        else if (m_tracks[track][row].note % 12 > 0)
          channel[chan].nextfreq =
              notetable[(m_tracks[track][row].note % 12) - 1];
        else
          channel[chan].nextfreq = notetable[11];
        channel[chan].nextoct = (m_tracks[track][row].note - 1) / 12;
        if (m_tracks[track][row].note == 127) { // handle key off
          channel[chan].nextfreq = channel[chan].freq;
          channel[chan].nextoct = channel[chan].oct;
        }
      }
      if (info) // remember vars
        channel[chan].portainfo = info;
      break;

    case 4: // vibrato (remember vars)
      if (info) {
        channel[chan].vibinfo1 = info1;
        channel[chan].vibinfo2 = info2;
      }
      break;

    case 7:
      m_tempo = info;
      break; // set tempo

    case 8:
      channel[chan].key = 0;
      setfreq(chan);
      break; // release sustaining note

    case 9: // set carrier/modulator volume
      if (info1)
        channel[chan].vol1 = info1 * 7;
      else
        channel[chan].vol2 = info2 * 7;
      setvolume(chan);
      break;

    case 11: // position jump
      pattbreak = 1;
      rw = 0;
      if (info < ord)
        songend = 1;
      ord = info;
      break;

    case 12: // set volume
      channel[chan].vol1 = info;
      channel[chan].vol2 = info;
      if (channel[chan].vol1 > 63)
        channel[chan].vol1 = 63;
      if (channel[chan].vol2 > 63)
        channel[chan].vol2 = 63;
      setvolume(chan);
      break;

    case 13: // pattern break
      if (!pattbreak) {
        pattbreak = 1;
        rw = info;
        ord++;
      }
      break;

    case 14: // extended command
      switch (info1) {
      case 0: // define cell-tremolo
        if (info2)
          regbd |= 128;
        else
          regbd &= 127;
        getOpl()->writeReg(0xbd, regbd);
        break;

      case 1: // define cell-vibrato
        if (info2)
          regbd |= 64;
        else
          regbd &= 191;
        getOpl()->writeReg(0xbd, regbd);
        break;

      case 4: // increase volume fine
        vol_up_alt(chan, info2);
        setvolume(chan);
        break;

      case 5: // decrease volume fine
        vol_down_alt(chan, info2);
        setvolume(chan);
        break;

      case 6: // manual slide up
        slide_up(chan, info2);
        setfreq(chan);
        break;

      case 7: // manual slide down
        slide_down(chan, info2);
        setfreq(chan);
        break;

      case 8: // pattern delay (rows)
        pattern_delay = info2 * speed;
        break;
      }
      break;

    case 15: // SA2 set speed
      if (info <= 0x1f)
        speed = info;
      if (info >= 0x32)
        m_tempo = info;
      if (!info)
        songend = 1;
      break;

    case 17: // alternate set volume
      channel[chan].vol1 = info;
      if (channel[chan].vol1 > 63)
        channel[chan].vol1 = 63;
      if (m_instruments[channel[chan].inst].data[0] & 1) {
        channel[chan].vol2 = info;
        if (channel[chan].vol2 > 63)
          channel[chan].vol2 = 63;
      }

      setvolume(chan);
      break;

    case 18: // AMD set speed
      if (info <= 31 && info > 0)
        speed = info;
      if (info > 31 || !info)
        m_tempo = info;
      break;

    case 19: // RAD/A2M set speed
      speed = (info ? info : info + 1);
      break;

    case 21: // set modulator volume
      if (info <= 63)
        channel[chan].vol2 = info;
      else
        channel[chan].vol2 = 63;
      setvolume(chan);
      break;

    case 22: // set carrier volume
      if (info <= 63)
        channel[chan].vol1 = info;
      else
        channel[chan].vol1 = 63;
      setvolume(chan);
      break;

    case 23: // fine frequency slide up
      slide_up(chan, info);
      setfreq(chan);
      break;

    case 24: // fine frequency slide down
      slide_down(chan, info);
      setfreq(chan);
      break;

    case 25: // set carrier/modulator waveform
      if (info1 != 0x0f)
        getOpl()->writeReg(0xe3 + m_opTable[oplchan], info1);
      if (info2 != 0x0f)
        getOpl()->writeReg(0xe0 + m_opTable[oplchan], info2);
      break;

    case 27: // set chip tremolo/vibrato
      if (info1)
        regbd |= 128;
      else
        regbd &= 127;
      if (info2)
        regbd |= 64;
      else
        regbd &= 191;
      getOpl()->writeReg(0xbd, regbd);
      break;

    case 29: // pattern delay (frames)
      pattern_delay = info;
      break;
    }
  }

  // speed compensation
  del = speed - 1 + pattern_delay;

  if (!pattbreak) { // next row (only if no manual advance)
    rw++;
    if (rw >= nrows) {
      rw = 0;
      ord++;
    }
  }

  resolve_order(); // so we can report songend right away
  AdPlug_LogWrite("\n");
  return !songend;
}

unsigned char CmodPlayer::set_opl_chip(
    unsigned char chan) /*
                         * Sets OPL chip according to channel number. Channels
                         * 0-8 are on first chip,
                         * channels 9-17 are on second chip. Returns
                         * corresponding OPL channel
                         * number.
                         */
    {
  return chan % 9;
}

bool CmodPlayer::
    resolve_order() /*
                     * Resolves current orderlist entry, checking
                     * for jumps and loops.
                     *
                     * Returns true on correct processing, false if
                     * immediate recursive loop
                     * has been detected.
                     */
    {
  if (ord < m_length) {
    while (m_order[ord] >= JUMPMARKER) { // jump to order
      unsigned long neword = m_order[ord] - JUMPMARKER;

      if (neword <= ord)
        songend = 1;
      if (neword == ord)
        return false;
      ord = neword;
    }
  } else {
    songend = 1;
    ord = m_restartpos;
  }

  return true;
}

void CmodPlayer::rewind(int) {
  unsigned long i;

  // Reset playing variables
  songend = del = ord = rw = regbd = 0;
  m_tempo = m_bpm;
  speed = m_initspeed;

  // Reset channel data
  {
    auto nchans = channel.size();
    channel.clear();
    channel.resize(nchans);
  }

  // Compute number of patterns, if needed
  if (!nop)
    for (i = 0; i < m_length; i++)
      nop = (m_order[i] > nop ? m_order[i] : nop);

  getOpl()->writeReg(1, 32); // Go to ym3812 mode

  // Enable OPL3 extensions if flagged
  if (m_flags & Opl3) {
    //FIXME sto opl->setchip(1);
    //FIXME sto opl->writeReg(1, 32);
    //FIXME sto opl->writeReg(5, 1);
    //FIXME sto opl->setchip(0);
  }

  // Enable tremolo/vibrato depth if flagged
  if (m_flags & Tremolo)
    regbd |= 128;
  if (m_flags & Vibrato)
    regbd |= 64;
  if (regbd)
    getOpl()->writeReg(0xbd, regbd);
}

size_t CmodPlayer::framesUntilUpdate() { return SampleRate * 2.5 / m_tempo; }

void CmodPlayer::init_trackord() {
  unsigned long i;

  for (i = 0; i < npats * channel.size(); i++)
    trackord[i / channel.size()][i % channel.size()] = i + 1;
}

void CmodPlayer::init_notetable(const std::array<uint16_t,12>& newnotetable) {
  notetable = newnotetable;
}

void CmodPlayer::realloc_patterns(unsigned long pats, unsigned long rows,
                                  unsigned long chans) {
  dealloc_patterns();

  // set new number of tracks, rows and channels
  npats = pats;
  nrows = rows;

  channel.clear();
  channel.resize(chans);

  // alloc new patterns
  m_tracks.reset(pats*chans, rows);
  trackord.reset(pats, chans, 0);
}

void CmodPlayer::dealloc_patterns() {
  // dealloc everything previously allocated
  if (npats && nrows && !channel.empty()) {
    m_tracks.clear();
    trackord.clear();
    channel.clear();
  }
}

/*** private methods *************************************/

void CmodPlayer::setvolume(unsigned char chan) {
  unsigned char oplchan = set_opl_chip(chan);

  if (m_flags & Faust)
    setvolume_alt(chan);
  else {
    getOpl()->writeReg(0x40 + m_opTable[oplchan],
                       63 - channel[chan].vol2 +
                           (m_instruments[channel[chan].inst].data[9] & 192));
    getOpl()->writeReg(0x43 + m_opTable[oplchan],
                       63 - channel[chan].vol1 +
                           (m_instruments[channel[chan].inst].data[10] & 192));
  }
}

void CmodPlayer::setvolume_alt(unsigned char chan) {
  unsigned char oplchan = set_opl_chip(chan);
  unsigned char ivol2 = m_instruments[channel[chan].inst].data[9] & 63;
  unsigned char ivol1 = m_instruments[channel[chan].inst].data[10] & 63;

  getOpl()->writeReg(0x40 + m_opTable[oplchan],
                     (((63 - (channel[chan].vol2 & 63)) + ivol2) >> 1) +
                         (m_instruments[channel[chan].inst].data[9] & 192));
  getOpl()->writeReg(0x43 + m_opTable[oplchan],
                     (((63 - (channel[chan].vol1 & 63)) + ivol1) >> 1) +
                         (m_instruments[channel[chan].inst].data[10] & 192));
}

void CmodPlayer::setfreq(unsigned char chan) {
  unsigned char oplchan = set_opl_chip(chan);

  getOpl()->writeReg(0xa0 + oplchan, channel[chan].freq & 255);
  if (channel[chan].key)
    getOpl()->writeReg(0xb0 + oplchan, ((channel[chan].freq & 768) >> 8) +
                                               (channel[chan].oct << 2) + 0x20);
  else
    getOpl()->writeReg(0xb0 + oplchan, ((channel[chan].freq & 768) >> 8) +
                                           (channel[chan].oct << 2));
}

void CmodPlayer::playnote(unsigned char chan) {
  unsigned char oplchan = set_opl_chip(chan);
  unsigned char op = m_opTable[oplchan], insnr = channel[chan].inst;

  if (!(m_flags & NoKeyOn))
    getOpl()->writeReg(0xb0 + oplchan, 0); // stop old note

  // set instrument data
  getOpl()->writeReg(0x20 + op, m_instruments[insnr].data[1]);
  getOpl()->writeReg(0x23 + op, m_instruments[insnr].data[2]);
  getOpl()->writeReg(0x60 + op, m_instruments[insnr].data[3]);
  getOpl()->writeReg(0x63 + op, m_instruments[insnr].data[4]);
  getOpl()->writeReg(0x80 + op, m_instruments[insnr].data[5]);
  getOpl()->writeReg(0x83 + op, m_instruments[insnr].data[6]);
  getOpl()->writeReg(0xe0 + op, m_instruments[insnr].data[7]);
  getOpl()->writeReg(0xe3 + op, m_instruments[insnr].data[8]);
  getOpl()->writeReg(0xc0 + oplchan, m_instruments[insnr].data[0]);
  getOpl()->writeReg(0xbd, m_instruments[insnr].misc); // set misc. register

  // set frequency, volume & play
  channel[chan].key = 1;
  setfreq(chan);

  if (m_flags & Faust) {
    channel[chan].vol2 = 63;
    channel[chan].vol1 = 63;
  }
  setvolume(chan);
}

void CmodPlayer::setnote(unsigned char chan, int note) {
  if (note > 96) {
    if (note == 127) { // key off
      channel[chan].key = 0;
      setfreq(chan);
      return;
    } else
      note = 96;
  }

  if (note < 13)
    channel[chan].freq = notetable[note - 1];
  else if (note % 12 > 0)
    channel[chan].freq = notetable[(note % 12) - 1];
  else
    channel[chan].freq = notetable[11];
  channel[chan].oct = (note - 1) / 12;
  channel[chan].freq +=
      m_instruments[channel[chan].inst].slide; // apply pre-slide
}

void CmodPlayer::slide_down(unsigned char chan, int amount) {
  channel[chan].freq -= amount;
  if (channel[chan].freq <= 342) {
    if (channel[chan].oct) {
      channel[chan].oct--;
      channel[chan].freq <<= 1;
    } else
      channel[chan].freq = 342;
  }
}

void CmodPlayer::slide_up(unsigned char chan, int amount) {
  channel[chan].freq += amount;
  if (channel[chan].freq >= 686) {
    if (channel[chan].oct < 7) {
      channel[chan].oct++;
      channel[chan].freq >>= 1;
    } else
      channel[chan].freq = 686;
  }
}

void CmodPlayer::tone_portamento(unsigned char chan, unsigned char info) {
  if (channel[chan].freq + (channel[chan].oct << 10) <
      channel[chan].nextfreq + (channel[chan].nextoct << 10)) {
    slide_up(chan, info);
    if (channel[chan].freq + (channel[chan].oct << 10) >
        channel[chan].nextfreq + (channel[chan].nextoct << 10)) {
      channel[chan].freq = channel[chan].nextfreq;
      channel[chan].oct = channel[chan].nextoct;
    }
  }
  if (channel[chan].freq + (channel[chan].oct << 10) >
      channel[chan].nextfreq + (channel[chan].nextoct << 10)) {
    slide_down(chan, info);
    if (channel[chan].freq + (channel[chan].oct << 10) <
        channel[chan].nextfreq + (channel[chan].nextoct << 10)) {
      channel[chan].freq = channel[chan].nextfreq;
      channel[chan].oct = channel[chan].nextoct;
    }
  }
  setfreq(chan);
}

void CmodPlayer::vibrato(unsigned char chan, unsigned char speed,
                         unsigned char depth) {
  int i;

  if (!speed || !depth)
    return;

  if (depth > 14)
    depth = 14;

  for (i = 0; i < speed; i++) {
    channel[chan].trigger++;
    while (channel[chan].trigger >= 64)
      channel[chan].trigger -= 64;
    if (channel[chan].trigger >= 16 && channel[chan].trigger < 48)
      slide_down(chan, vibratotab[channel[chan].trigger - 16] / (16 - depth));
    if (channel[chan].trigger < 16)
      slide_up(chan, vibratotab[channel[chan].trigger + 16] / (16 - depth));
    if (channel[chan].trigger >= 48)
      slide_up(chan, vibratotab[channel[chan].trigger - 48] / (16 - depth));
  }
  setfreq(chan);
}

void CmodPlayer::vol_up(unsigned char chan, int amount) {
  if (channel[chan].vol1 + amount < 63)
    channel[chan].vol1 += amount;
  else
    channel[chan].vol1 = 63;

  if (channel[chan].vol2 + amount < 63)
    channel[chan].vol2 += amount;
  else
    channel[chan].vol2 = 63;
}

void CmodPlayer::vol_down(unsigned char chan, int amount) {
  if (channel[chan].vol1 - amount > 0)
    channel[chan].vol1 -= amount;
  else
    channel[chan].vol1 = 0;

  if (channel[chan].vol2 - amount > 0)
    channel[chan].vol2 -= amount;
  else
    channel[chan].vol2 = 0;
}

void CmodPlayer::vol_up_alt(unsigned char chan, int amount) {
  if (channel[chan].vol1 + amount < 63)
    channel[chan].vol1 += amount;
  else
    channel[chan].vol1 = 63;
  if (m_instruments[channel[chan].inst].data[0] & 1) {
    if (channel[chan].vol2 + amount < 63)
      channel[chan].vol2 += amount;
    else
      channel[chan].vol2 = 63;
  }
}

void CmodPlayer::vol_down_alt(unsigned char chan, int amount) {
  if (channel[chan].vol1 - amount > 0)
    channel[chan].vol1 -= amount;
  else
    channel[chan].vol1 = 0;
  if (m_instruments[channel[chan].inst].data[0] & 1) {
    if (channel[chan].vol2 - amount > 0)
      channel[chan].vol2 -= amount;
    else
      channel[chan].vol2 = 0;
  }
}
