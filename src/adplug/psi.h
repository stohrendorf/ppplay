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
 * [xad] PSI player, by Riven the Mage <riven@ok.ru>
 */

#include "xad.h"

class PsiPlayer
  : public XadPlayer
{
public:
  DISABLE_COPY( PsiPlayer )

  static Player* factory();

  PsiPlayer() = default;

private:
  struct Header
  {
    uint16_t instr_ptr = 0;
    uint16_t seq_ptr = 0;
  };

  Header m_header{};

  struct PsiData
  {
    const uint8_t* instr_table = nullptr;
    const uint8_t* seq_table = nullptr;
    uint8_t note_delay[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t note_curdelay[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::array<bool, 9> looping{ {} };
  };

  PsiData m_psi{};

  //
  bool xadplayer_load() override
  {
    return xadHeader().fmt == PSI;
  }

  void xadplayer_rewind(const boost::optional<size_t>& subsong) override;

  void xadplayer_update() override;

  size_t framesUntilUpdate() const override;

  std::string type() const override;

  size_t instrumentCount() const override;
};
