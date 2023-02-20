#pragma once

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * mkj.h - MKJamz Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class MkjPlayer
  : public Player
{
public:
  DISABLE_COPY( MkjPlayer )

  static Player* factory();

  MkjPlayer() = default;

  ~MkjPlayer() override = default;

  bool load(const std::string& filename) override;

  bool update() override;

  void rewind(const boost::optional<size_t>& subsong) override;

  size_t framesUntilUpdate() const override;

  std::string type() const override
  {
    return "MKJamz Audio File";
  }

private:
  int16_t m_channelCount = 0;
  int16_t m_maxNotes = 0;
  std::vector<int16_t> m_songBuf{};
  bool m_songEnd = true;

  struct Channel
  {
    int16_t defined = 0;
    uint16_t dataOfs = 0;
    int16_t octave = 4, waveform = 0, delay = 0, speed = 1;
  };

  Channel m_channels[9];
};
