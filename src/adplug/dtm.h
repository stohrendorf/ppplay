#pragma once

/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  dtm.h - DTM loader by Riven the Mage <riven@ok.ru>
*/

#include "mod.h"

class DtmPlayer : public ModPlayer
{
    DISABLE_COPY(DtmPlayer)
public:
    static Player *factory();

    DtmPlayer() = default;

    bool load(const std::string &filename) override;
    void rewind(int subsong) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override;
    std::string title() const override;
    std::string author() const override;
    std::string description() const override;
    std::string instrumentTitle(size_t n) const override;
    size_t instrumentCount() const override;

private:

    struct dtm_header
    {
        char id[12] = "";
        uint8_t version = 0;
        char title[20] = "";
        char author[20] = "";
        uint8_t numpat = 0;
        uint8_t numinst = 0;
    };

    dtm_header m_header{};

    std::string m_description{};

    struct dtm_instrument
    {
        char name[13] = "";
        uint8_t data[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
    };
    dtm_instrument m_instruments[128];

    static std::vector<uint8_t> unpack_pattern(const std::vector<uint8_t>& input);
};
