#pragma once

/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
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
 * d00.h - D00 Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class D00Player
  : public Player
{
public:
  DISABLE_COPY( D00Player )

  static Player* factory();

  D00Player() = default;

  ~D00Player() override = default;

  bool load(const std::string& filename) override;

  bool update() override;

  void rewind(const boost::optional<size_t>& subsong) override;

  size_t framesUntilUpdate() const override;

  std::string type() const override;

  std::string title() const override
  {
    if( m_version > 1 )
    {
      return m_header->songname;
    }
    else
    {
      return std::string();
    }
  }

  std::string author() const override
  {
    if( m_version > 1 )
    {
      return m_header->author;
    }
    else
    {
      return std::string();
    }
  }

  std::string description() const override
  {
    if( *m_description )
    {
      return m_description;
    }
    else
    {
      return std::string();
    }
  }

  size_t subSongCount() const override;

private:
#pragma pack(push, 1)
  struct d00header
  {
    char id[6];
    uint8_t type, version, speed, subsongs, soundcard;
    char songname[32], author[32], dummy[32];
    uint16_t trackPointerOfs, orderListOfs, instrumentOfs, infoptr, spfxptr, endmark;
  };

  struct d00header1
  {
    uint8_t version, speed, subsongs;
    uint16_t trackPointerOfs, orderListOfs, instrumentOfs, infoptr, lpulptr, endmark;
  };

  struct Instrument
  {
    uint8_t data[11], finetune, timer, sr, dummy[2];
  };
#pragma pack(pop)

  struct Channel
  {
    const uint16_t* patternData;
    uint16_t orderPos, patternPos, delay, speed, restHoldDelay, frequency, instrument,
      spfx = 0xffff, ispfx, irhcnt;
    signed short transpose, noteSlideSpeed, noteSlideValue, vibratoSpeed;
    uint8_t volume, vibratoDepth, fxDelay, modulatorVolume, carrierVolume, levpuls = 0xff,
      frameskip, nextNote, note, ilevpuls = 0xff, trigger, fxflag;

    bool keyOn = false;
    bool seqend = false;
  };

  Channel m_channels[9];

  const Instrument* m_instruments = nullptr;

  struct Sspfx
  {
    uint16_t instnr;
    int8_t halfnote;
    uint8_t modlev;
    int8_t modlevadd;
    uint8_t duration;
    uint16_t ptr;
  };
  const Sspfx* m_spfx = nullptr;

  struct Slevpuls
  {
    uint8_t level;
    int8_t voladd;
    uint8_t duration, ptr;
  };
  const Slevpuls* m_levPuls = nullptr;

  bool m_songEnd = false;
  uint8_t m_version = 0;
  size_t m_currentSubSong = 0;
  char* m_description = nullptr;
  const uint16_t* m_orders = nullptr;
  d00header* m_header = nullptr;
  d00header1* m_header1 = nullptr;
  std::vector<char> m_fileData{};

  void setvolume(uint8_t chan);

  void setfreq(uint8_t chan);

  void setinst(uint8_t chan);

  void playnote(uint8_t chan);

  void vibrato(uint8_t chan);
};
