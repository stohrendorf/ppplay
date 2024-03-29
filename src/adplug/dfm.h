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
 * dfm.h - Digital-FM Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "mod.h"

class DfmPlayer
  : public ModPlayer
{
public:
  DISABLE_COPY( DfmPlayer )

  static Player* factory();

  DfmPlayer() = default;

  bool load(const std::string& filename) override;

  size_t framesUntilUpdate() const override;

  std::string type() const override;

  size_t instrumentCount() const override
  {
    return 32;
  }

  std::string instrumentTitle(size_t n) const override
  {
    return m_instName[n];
  }

  std::string description() const override
  {
    return m_songInfo;
  }

private:
  std::string m_songInfo{};
  std::array<std::string, 32> m_instName{ {} };
  std::string m_type{};
};
