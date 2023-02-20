#pragma once

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2005 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * imf.h - IMF Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class FileStream;

class ImfPlayer
  : public Player
{
public:
  DISABLE_COPY( ImfPlayer )

  ImfPlayer() = default;

  static Player* factory();

  bool load(const std::string& filename) override;

  bool update() override;

  void rewind(const boost::optional<size_t>& subsong) override;

  size_t framesUntilUpdate() const override
  {
    return static_cast<size_t>(SampleRate / m_timer);
  }

  std::string type() const override
  {
    return "IMF File Format";
  }

  std::string title() const override;

  std::string author() const override
  {
    return m_authorName;
  }

  std::string description() const override;

private:
  unsigned long m_pos = 0;
  unsigned short m_del = 0;
  bool m_songend = false;
  int m_rate = 0;
  float m_timer = 0;
  std::string m_footer{};

  std::string m_trackName{}
  , m_gameName{}
  , m_authorName{}
  , m_remarks{};

#pragma pack(push, 1)
  struct Sdata
  {
    uint8_t reg = 0, val = 0;
    uint16_t time = 0;
  };
#pragma pack(pop)
  std::vector<Sdata> m_data{};

  static int getrate(const FileStream& file);
};
