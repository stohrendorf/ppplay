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

class CbamPlayer : public CPlayer {
  DISABLE_COPY(CbamPlayer)
public:
  static CPlayer *factory();

  CbamPlayer() = default;

  bool load(const std::string &filename);
  bool update();
  void rewind(int);
  size_t framesUntilUpdate() override { return SampleRate / 25; }

  std::string gettype() { return "Bob's Adlib Music"; }

private:
  static const unsigned short freq[];

  std::vector<uint8_t> m_song{};
  uint8_t del = 0;
  size_t pos = 0, gosub = 0;
  bool songend = false, chorus = false;

  struct {
    size_t target = 0;
    bool defined = false;
    uint8_t count = 0;
  } label[16];
};
