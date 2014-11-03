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
 * lds.cpp - LOUDNESS Player by Simon Peter <dn.tlp@gmx.net>
 */

#include <string.h>

#include "lds.h"
#include "debug.h"
#include "stream/filestream.h"

namespace {
// Note frequency table (16 notes / octave)
constexpr uint16_t frequency[] = {
  343, 344, 345, 347, 348, 349, 350, 352, 353, 354, 356, 357, 358, 359, 361,
  362, 363, 365, 366, 367, 369, 370, 371, 373, 374, 375, 377, 378, 379, 381,
  382, 384, 385, 386, 388, 389, 391, 392, 393, 395, 396, 398, 399, 401, 402,
  403, 405, 406, 408, 409, 411, 412, 414, 415, 417, 418, 420, 421, 423, 424,
  426, 427, 429, 430, 432, 434, 435, 437, 438, 440, 442, 443, 445, 446, 448,
  450, 451, 453, 454, 456, 458, 459, 461, 463, 464, 466, 468, 469, 471, 473,
  475, 476, 478, 480, 481, 483, 485, 487, 488, 490, 492, 494, 496, 497, 499,
  501, 503, 505, 506, 508, 510, 512, 514, 516, 518, 519, 521, 523, 525, 527,
  529, 531, 533, 535, 537, 538, 540, 542, 544, 546, 548, 550, 552, 554, 556,
  558, 560, 562, 564, 566, 568, 571, 573, 575, 577, 579, 581, 583, 585, 587,
  589, 591, 594, 596, 598, 600, 602, 604, 607, 609, 611, 613, 615, 618, 620,
  622, 624, 627, 629, 631, 633, 636, 638, 640, 643, 645, 647, 650, 652, 654,
  657, 659, 662, 664, 666, 669, 671, 674, 676, 678, 681, 683
};

// Vibrato (sine) table
constexpr uint8_t vibtab[] = {
  0, 13, 25, 37, 50, 62, 74, 86, 98, 109, 120, 131, 142, 152, 162, 171, 180,
  189, 197, 205, 212, 219, 225, 231, 236, 240, 244, 247, 250, 252, 254, 255,
  255, 255, 254, 252, 250, 247, 244, 240, 236, 231, 225, 219, 212, 205, 197,
  189, 180, 171, 162, 152, 142, 131, 120, 109, 98, 86, 74, 62, 50, 37, 25, 13
};

// Tremolo (sine * sine) table
constexpr uint8_t tremtab[] = {
  0, 0, 1, 1, 2, 4, 5, 7, 10, 12, 15, 18, 21, 25, 29, 33, 37, 42, 47, 52, 57,
  62, 67, 73, 79, 85, 90, 97, 103, 109, 115, 121, 128, 134, 140, 146, 152, 158,
  165, 170, 176, 182, 188, 193, 198, 203, 208, 213, 218, 222, 226, 230, 234,
  237, 240, 243, 245, 248, 250, 251, 253, 254, 254, 255, 255, 255, 254, 254,
  253, 251, 250, 248, 245, 243, 240, 237, 234, 230, 226, 222, 218, 213, 208,
  203, 198, 193, 188, 182, 176, 170, 165, 158, 152, 146, 140, 134, 127, 121,
  115, 109, 103, 97, 90, 85, 79, 73, 67, 62, 57, 52, 47, 42, 37, 33, 29, 25, 21,
  18, 15, 12, 10, 7, 5, 4, 2, 1, 1, 0
};
constexpr uint16_t maxsound = 0x3f;
constexpr uint16_t maxpos = 0xff;
}

// 'maxsound' is maximum number of patches (instruments)
// 'maxpos' is maximum number of entries in position list (orderlist)

/*** public methods *************************************/

CldsPlayer::CldsPlayer() : CPlayer() {}

bool CldsPlayer::load(const std::string &filename, const CFileProvider &fp) {
  // file validation section (actually just an extension check)
  if (!fp.extension(filename, ".lds"))
    return false;
  FileStream fs(filename);
  if (!fs.isOpen())
    return false;

  // file load section (header)
  fs >> m_mode;
  if (m_mode > 2) {
    return false;
  }
  fs >> m_speed >> m_tempo >> m_pattlen;
  for (int i = 0; i < 9; i++)
    fs >> m_chandelay[i];
  fs >> m_regbd;

  // load patches
  uint16_t numpatch;
  fs >> numpatch;
  m_soundbank.resize(numpatch);
  fs.read(m_soundbank.data(), numpatch);

  // load positions
  fs >> m_numposi;
  m_positions.resize(9 * m_numposi);
  for (size_t i = 0; i < m_numposi; i++)
    for (int j = 0; j < 9; j++) {
      fs >> m_positions[i * 9 + j];
      /*
     * patnum is a pointer inside the pattern space, but patterns are 16bit
     * word fields anyway, so it ought to be an even number (hopefully) and
     * we can just divide it by 2 to get our array index of 16bit words.
     */
      m_positions[i * 9 + j].patnum >>= 1;
    }

  AdPlug_LogWrite("CldsPlayer::load(\"%s\",fp): loading LOUDNESS file: mode = "
                  "%d, pattlen = %d, numpatch = %d, numposi = %d\n",
                  filename.c_str(), m_mode, m_pattlen, numpatch, m_numposi);

  // load patterns
  fs.seekrel(2); // ignore # of digital sounds (not played by this player)
  m_patterns.resize((fs.size() - fs.pos()) / 2 + 1);
  fs.read(m_patterns.data(), m_patterns.size());

  rewind(0);
  return true;
}

bool CldsPlayer::update() {
  if (!m_playing)
    return false;

  // handle fading
  if (m_fadeonoff) {
    if (m_fadeonoff <= 128) {
      if (m_allvolume > m_fadeonoff || m_allvolume == 0)
        m_allvolume -= m_fadeonoff;
      else {
        m_allvolume = 1;
        m_fadeonoff = 0;
        if (m_hardfade != 0) {
          m_playing = false;
          m_hardfade = 0;
          for (int i = 0; i < 9; i++)
            m_channels[i].keycount = 1;
        }
      }
    } else if (((m_allvolume + (0x100 - m_fadeonoff)) & 0xff) <= m_mainvolume)
      m_allvolume += 0x100 - m_fadeonoff;
    else {
      m_allvolume = m_mainvolume;
      m_fadeonoff = 0;
    }
  }
  // handle channel delay
  for (int chan = 0; chan < 9; chan++) {
    Channel *c = &m_channels[chan];
    if (c->chancheat.chandelay)
      if (!(--c->chancheat.chandelay))
        playsound(c->chancheat.sound, chan, c->chancheat.high);
  }

  // handle notes
  if (!m_tempoNow) {
    bool vbreak = false;
    for (int chan = 0; chan < 9; chan++) {
      Channel *c = &m_channels[chan];
      if (!c->packwait) {
        unsigned short patnum = m_positions[m_posplay * 9 + chan].patnum;
        unsigned char transpose = m_positions[m_posplay * 9 + chan].transpose;

        auto comword = m_patterns[patnum + c->packpos];
        auto comhi = comword >> 8;
        auto comlo = comword & 0xff;
        if (comword) {
          if (comhi == 0x80)
            c->packwait = comlo;
          else if (comhi >= 0x80) {
            switch (comhi) {
            case 0xff:
              c->volcar = (((c->volcar & 0x3f) * comlo) >> 6) & 0x3f;
              if (getOpl()->readReg(0xc0 + chan) & 1)
                c->volmod = (((c->volmod & 0x3f) * comlo) >> 6) & 0x3f;
              break;
            case 0xfe:
              m_tempo = comword & 0x3f;
              break;
            case 0xfd:
              c->nextvol = comlo;
              break;
            case 0xfc:
              m_playing = false;
              // in real player there's also full keyoff here, but we don't
              // need it
              break;
            case 0xfb:
              c->keycount = 1;
              break;
            case 0xfa:
              vbreak = true;
              m_jumppos = (m_posplay + 1) & maxpos;
              break;
            case 0xf9:
              vbreak = true;
              m_jumppos = comlo & maxpos;
              m_jumping = 1;
              if (m_jumppos < m_posplay)
                m_songlooped = true;
              break;
            case 0xf8:
              c->lasttune = 0;
              break;
            case 0xf7:
              c->vibwait = 0;
              // PASCAL: c->vibspeed = ((comlo >> 4) & 15) + 2;
              c->vibspeed = (comlo >> 4) + 2;
              c->vibrate = (comlo & 15) + 1;
              break;
            case 0xf6:
              c->glideto = comlo;
              break;
            case 0xf5:
              c->finetune = comlo;
              break;
            case 0xf4:
              if (!m_hardfade) {
                m_allvolume = m_mainvolume = comlo;
                m_fadeonoff = 0;
              }
              break;
            case 0xf3:
              if (!m_hardfade)
                m_fadeonoff = comlo;
              break;
            case 0xf2:
              c->trmstay = comlo;
              break;
            case 0xf1: // panorama
            case 0xf0: // progch
                       // MIDI commands (unhandled)
              AdPlug_LogWrite("CldsPlayer(): not handling MIDI command 0x%x, "
                              "value = 0x%x\n",
                              comhi);
              break;
            default:
              if (comhi < 0xa0)
                c->glideto = comhi & 0x1f;
              else
                AdPlug_LogWrite(
                    "CldsPlayer(): unknown command 0x%x encountered!"
                    " value = 0x%x\n",
                    comhi, comlo);
              break;
            }
          } else {
            unsigned char sound;
            unsigned short high;
            signed char transp = transpose & 127;
            /*
                   * Originally, in assembler code, the player first shifted
                   * logically left the transpose byte by 1 and then shifted
                   * arithmetically right the same byte to achieve the final,
                   * signed transpose value. Since we can't do arithmetic shifts
                   * in C, we just duplicate the 7th bit into the 8th one and
                   * discard the 8th one completely.
                   */

            if (transpose & 64)
              transp |= 128;

            if (transpose & 128) {
              sound = (comlo + transp) & maxsound;
              high = comhi << 4;
            } else {
              sound = comlo & maxsound;
              high = (comhi + transp) << 4;
            }

            /*
                PASCAL:
                  sound = comlo & maxsound;
                  high = (comhi + (((transpose + 0x24) & 0xff) - 0x24)) << 4;
                  */

            if (!m_chandelay[chan])
              playsound(sound, chan, high);
            else {
              c->chancheat.chandelay = m_chandelay[chan];
              c->chancheat.sound = sound;
              c->chancheat.high = high;
            }
          }
        }

        c->packpos++;
      } else
        c->packwait--;
    }

    m_tempoNow = m_tempo;
    /*
      The continue table is updated here, but this is only used in the
      original player, which can be paused in the middle of a song and then
      unpaused. Since AdPlug does all this for us automatically, we don't
      have a continue table here. The continue table update code is noted
      here for reference only.

      if(!pattplay) {
        conttab[speed & maxcont].position = posplay & 0xff;
        conttab[speed & maxcont].tempo = tempo;
      }
    */
    m_pattplay++;
    if (vbreak) {
      m_pattplay = 0;
      for (int i = 0; i < 9; i++)
        m_channels[i].packpos = m_channels[i].packwait = 0;
      m_posplay = m_jumppos;
    } else if (m_pattplay >= m_pattlen) {
      m_pattplay = 0;
      for (int i = 0; i < 9; i++)
        m_channels[i].packpos = m_channels[i].packwait = 0;
      m_posplay = (m_posplay + 1) & maxpos;
    }
  } else
    m_tempoNow--;

  // make effects
  for (int chan = 0; chan < 9; chan++) {
    opl::SlotView slotView = getOpl()->getSlotView(chan);
    Channel *c = &m_channels[chan];
    if (c->keycount > 0) {
      if (c->keycount == 1)
        slotView.setKeyOn(false);
      c->keycount--;
    }

    // arpeggio
    uint16_t arpreg = 0;
    if (c->arp_size != 0) {
      arpreg = c->arp_tab[c->arp_pos] << 4;
      if (arpreg == 0x800) {
        if (c->arp_pos > 0)
          c->arp_tab[0] = c->arp_tab[c->arp_pos - 1];
        c->arp_size = 1;
        c->arp_pos = 0;
        arpreg = c->arp_tab[0] << 4;
      }

      if (c->arp_count == c->arp_speed) {
        c->arp_pos++;
        if (c->arp_pos >= c->arp_size)
          c->arp_pos = 0;
        c->arp_count = 0;
      } else
        c->arp_count++;
    }

    // glide & portamento
    if (c->lasttune && (c->lasttune != c->gototune)) {
      if (c->lasttune > c->gototune) {
        if (c->lasttune - c->gototune < c->portspeed)
          c->lasttune = c->gototune;
        else
          c->lasttune -= c->portspeed;
      } else {
        if (c->gototune - c->lasttune < c->portspeed)
          c->lasttune = c->gototune;
        else
          c->lasttune += c->portspeed;
      }

      if (arpreg >= 0x800)
        arpreg = c->lasttune - (arpreg ^ 0xff0) - 16;
      else
        arpreg += c->lasttune;

      slotView.setBlock(arpreg / (12 * 16) - 1);
      slotView.setFnum(frequency[arpreg % (12 * 16)]);
    } else {
      // vibrato
      if (!c->vibwait) {
        if (c->vibrate) {
          auto wibc = vibtab[c->vibcount & 0x3f] * c->vibrate;

          auto tune = c->lasttune;
          if ((c->vibcount & 0x40) == 0)
            tune += (wibc >> 8);
          else
            tune -= (wibc >> 8);

          if (arpreg >= 0x800)
            tune -= (arpreg ^ 0xff0) + 16;
          else
            tune += arpreg;

          slotView.setBlock(tune / (12 * 16) - 1);
          slotView.setFnum(frequency[tune % (12 * 16)]);
          c->vibcount += c->vibspeed;
        } else if (c->arp_size != 0) { // no vibrato, just arpeggio
          auto tune = c->lasttune;
          if (arpreg >= 0x800)
            tune -= (arpreg ^ 0xff0) + 16;
          else
            tune += arpreg;

          slotView.setBlock(tune / (12 * 16) - 1);
          slotView.setFnum(frequency[tune % (12 * 16)]);
        }
      } else { // no vibrato, just arpeggio
        c->vibwait--;

        if (c->arp_size != 0) {
          auto tune = c->lasttune;
          if (arpreg >= 0x800)
            tune -= (arpreg ^ 0xff0) + 16;
          else
            tune += arpreg;

          slotView.setBlock(tune / (12 * 16) - 1);
          slotView.setFnum(frequency[tune % (12 * 16)]);
        }
      }
    }

    // tremolo (modulator)
    if (!c->trmwait) {
      if (c->trmrate) {
        auto tremc = tremtab[c->trmcount & 0x7f] * c->trmrate;
        int level = 0;
        if ((tremc >> 8) <= (c->volmod & 0x3f))
          level = (c->volmod & 0x3f) - (tremc >> 8);

        if (m_allvolume != 0 && (getOpl()->readReg(0xc0 + chan) & 1))
          slotView.modulator()
              .setTotalLevel(((level * m_allvolume) >> 8) ^ 0x3f);
        else
          slotView.modulator().setTotalLevel(level ^ 0x3f);

        c->trmcount += c->trmspeed;
      } else if (m_allvolume != 0 && (getOpl()->readReg(0xc0 + chan) & 1))
        slotView.modulator().setTotalLevel(
            ((((c->volmod & 0x3f) * m_allvolume) >> 8) ^ 0x3f) & 0x3f);
      else
        slotView.modulator().setTotalLevel((c->volmod ^ 0x3f) & 0x3f);
    } else {
      c->trmwait--;
      if (m_allvolume != 0 && (getOpl()->readReg(0xc0 + chan) & 1))
        slotView.modulator().setTotalLevel(
            ((((c->volmod & 0x3f) * m_allvolume) >> 8) ^ 0x3f) & 0x3f);
    }

    // tremolo (carrier)
    if (!c->trcwait) {
      if (c->trcrate) {
        auto tremc = tremtab[c->trccount & 0x7f] * c->trcrate;
        int level = 0;
        if ((tremc >> 8) <= (c->volcar & 0x3f))
          level = (c->volcar & 0x3f) - (tremc >> 8);

        if (m_allvolume != 0)
          slotView.carrier().setTotalLevel(((level * m_allvolume) >> 8) ^ 0x3f);
        else
          slotView.carrier().setTotalLevel(level ^ 0x3f);
        c->trccount += c->trcspeed;
      } else if (m_allvolume != 0)
        slotView.carrier().setTotalLevel(
            ((((c->volcar & 0x3f) * m_allvolume) >> 8) ^ 0x3f) & 0x3f);
      else
        slotView.carrier().setTotalLevel((c->volcar ^ 0x3f) & 0x3f);
    } else {
      c->trcwait--;
      if (m_allvolume != 0)
        slotView.carrier().setTotalLevel(
            ((((c->volcar & 0x3f) * m_allvolume) >> 8) ^ 0x3f) & 0x3f);
    }
  }

  return (!m_playing || m_songlooped) ? false : true;
}

void CldsPlayer::rewind(int) {
  // init all with 0
  m_tempoNow = 3;
  m_playing = true;
  m_songlooped = false;
  m_jumping = m_fadeonoff = m_allvolume = m_hardfade = m_pattplay = m_posplay =
      m_jumppos = m_mainvolume = 0;
  memset(m_channels, 0, sizeof(m_channels));

  // OPL2 init
  getOpl()->writeReg(1, 0x20);
  getOpl()->writeReg(8, 0);
  getOpl()->writeReg(0xbd, m_regbd);

  for (int i = 0; i < 9; i++) {
    getOpl()->writeReg(0x20 + s_opTable[i], 0);
    getOpl()->writeReg(0x23 + s_opTable[i], 0);
    getOpl()->writeReg(0x40 + s_opTable[i], 0x3f);
    getOpl()->writeReg(0x43 + s_opTable[i], 0x3f);
    getOpl()->writeReg(0x60 + s_opTable[i], 0xff);
    getOpl()->writeReg(0x63 + s_opTable[i], 0xff);
    getOpl()->writeReg(0x80 + s_opTable[i], 0xff);
    getOpl()->writeReg(0x83 + s_opTable[i], 0xff);
    getOpl()->writeReg(0xe0 + s_opTable[i], 0);
    getOpl()->writeReg(0xe3 + s_opTable[i], 0);
    getOpl()->writeReg(0xa0 + i, 0);
    getOpl()->writeReg(0xb0 + i, 0);
    getOpl()->writeReg(0xc0 + i, 0);
  }
}

/*** private methods *************************************/

void CldsPlayer::playsound(int inst_number, int channel_number, int tunehigh) {
  Channel *c = &m_channels[channel_number]; // current channel
  SoundBank *i = &m_soundbank[inst_number]; // current instrument
  opl::SlotView slotView = getOpl()->getSlotView(channel_number);

  // set fine tune
  tunehigh += ((i->finetune + c->finetune + 0x80) & 0xff) - 0x80;

  // arpeggio handling
  if (!i->arpeggio) {
    unsigned short arpcalc = i->arp_tab[0] << 4;

    if (arpcalc > 0x800)
      tunehigh -= (arpcalc ^ 0xff0) + 16;
    else
      tunehigh += arpcalc;
  }

  // glide handling
  if (c->glideto != 0) {
    c->gototune = tunehigh;
    c->portspeed = c->glideto;
    c->glideto = c->finetune = 0;
    return;
  }

  // set modulator registers
  slotView.modulator().setAm(i->mod_misc & 0x80);
  slotView.modulator().setVib(i->mod_misc & 0x40);
  slotView.modulator().setEgt(i->mod_misc & 0x20);
  slotView.modulator().setKsr(i->mod_misc & 0x10);
  slotView.modulator().setMult(i->mod_misc & 0x0f);
  auto volcalc = i->mod_vol;
  if (!c->nextvol || !(i->feedback & 1))
    c->volmod = volcalc;
  else
    c->volmod = (volcalc & 0xc0) | ((((volcalc & 0x3f) * c->nextvol) >> 6));

  if ((i->feedback & 1) == 1 && m_allvolume != 0) {
    slotView.modulator()
        .setTotalLevel((((c->volmod & 0x3f) * m_allvolume) >> 8) ^ 0x3f);
    slotView.modulator().setKsl(c->volmod >> 6);
  } else {
    slotView.modulator().setTotalLevel(c->volmod ^ 0x3f);
    slotView.modulator().setKsl(0);
  }
  slotView.modulator().setAttackRate(i->mod_ad >> 4);
  slotView.modulator().setDecayRate(i->mod_ad & 15);
  slotView.modulator().setSustainLevel(i->mod_sr >> 4);
  slotView.modulator().setReleaseRate(i->mod_sr & 15);
  slotView.modulator().setWave(i->mod_wave);

  // Set carrier registers
  slotView.carrier().setAm(i->car_misc & 0x80);
  slotView.carrier().setVib(i->car_misc & 0x40);
  slotView.carrier().setEgt(i->car_misc & 0x20);
  slotView.carrier().setKsr(i->car_misc & 0x10);
  slotView.carrier().setMult(i->car_misc & 0x0f);
  volcalc = i->car_vol;
  if (!c->nextvol)
    c->volcar = volcalc;
  else
    c->volcar = (volcalc & 0xc0) | ((((volcalc & 0x3f) * c->nextvol) >> 6));

  if (m_allvolume != 0) {
    slotView.carrier()
        .setTotalLevel((((c->volcar & 0x3f) * m_allvolume) >> 8) ^ 0x3f);
    slotView.carrier().setKsl(c->volcar >> 6);
  } else {
    slotView.carrier().setTotalLevel(c->volcar ^ 0x3f);
    slotView.carrier().setKsl(0);
  }
  slotView.carrier().setAttackRate(i->car_ad >> 4);
  slotView.carrier().setDecayRate(i->car_ad & 15);
  slotView.carrier().setSustainLevel(i->car_sr >> 4);
  slotView.carrier().setReleaseRate(i->car_sr & 15);
  slotView.carrier().setWave(i->car_wave);

  slotView.setCnt(i->feedback & 1);
  slotView.setFeedback((i->feedback >> 1) & 7);
  slotView.setOutput(i->feedback & 0x80, i->feedback & 0x40, i->feedback & 0x20,
                     i->feedback & 0x10);
  slotView.setKeyOn(false); // key off

  auto freq = frequency[tunehigh % (12 * 16)];
  auto octave = tunehigh / (12 * 16) - 1;
  if (!i->glide) {
    if (!i->portamento || !c->lasttune) {
      slotView.setBlock(octave);
      slotView.setFnum(freq);
      c->lasttune = c->gototune = tunehigh;
    } else {
      c->gototune = tunehigh;
      c->portspeed = i->portamento;
    }
  } else {
    slotView.setBlock(octave);
    slotView.setFnum(freq);
    c->lasttune = tunehigh;
    c->gototune =
        tunehigh + ((i->glide + 0x80) & 0xff) - 0x80; // set destination
    c->portspeed = i->portamento;
  }
  slotView.setKeyOn(true);

  if (!i->vibrato)
    c->vibwait = c->vibspeed = c->vibrate = 0;
  else {
    c->vibwait = i->vibdelay;
    // PASCAL:    c->vibspeed = ((i->vibrato >> 4) & 15) + 1;
    c->vibspeed = (i->vibrato >> 4) + 2;
    c->vibrate = (i->vibrato & 15) + 1;
  }

  if (!(c->trmstay & 0xf0)) {
    c->trmwait = (i->tremwait & 0xf0) >> 3;
    // PASCAL:    c->trmspeed = (i->mod_trem >> 4) & 15;
    c->trmspeed = i->mod_trem >> 4;
    c->trmrate = i->mod_trem & 15;
    c->trmcount = 0;
  }

  if (!(c->trmstay & 0x0f)) {
    c->trcwait = (i->tremwait & 15) << 1;
    // PASCAL:    c->trcspeed = (i->car_trem >> 4) & 15;
    c->trcspeed = i->car_trem >> 4;
    c->trcrate = i->car_trem & 15;
    c->trccount = 0;
  }

  c->arp_size = i->arpeggio & 15;
  c->arp_speed = i->arpeggio >> 4;
  c->arp_tab = i->arp_tab;
  c->keycount = i->keyoff;
  c->nextvol = c->glideto = c->finetune = c->vibcount = c->arp_pos =
      c->arp_count = 0;
}
