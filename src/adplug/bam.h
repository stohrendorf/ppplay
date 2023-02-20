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
 * bam.h - Bob's Adlib Music Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class BamPlayer
  : public Player
{
public:
  DISABLE_COPY( BamPlayer )

  static Player* factory();

  BamPlayer() = default;

  bool load(const std::string& filename) override;

  bool update() override;

  void rewind(const boost::optional<size_t>& subsong) override;

  size_t framesUntilUpdate() const override
  {
    return SampleRate / 25;
  }

  std::string type() const override
  {
    return "Bob's Adlib Music";
  }

private:
  std::vector<uint8_t> m_song{};
  uint8_t m_delay = 0;
  size_t m_position = 0;
  size_t m_goSub = 0;
  bool m_songEnd = false;
  bool m_chorus = false;

  struct Label
  {
    size_t target = 0;
    bool defined = true;
    uint8_t count = 0xff;
  };
  std::array<Label, 16> m_labels{ {} };
};
