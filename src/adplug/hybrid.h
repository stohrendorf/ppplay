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
 * [xad] HYBRID player, by Riven the Mage <riven@ok.ru>
 */

#include "xad.h"

class HybridPlayer : public XadPlayer
{
    DISABLE_COPY(HybridPlayer)
public:
    static Player *factory();

    HybridPlayer() = default;

protected:
#pragma pack(push,1)
    struct Instrument
    {
        char name[7];
        uint8_t data[11];
    };
#pragma pack(pop)

    bool xadplayer_load() override;
    void xadplayer_rewind(const boost::optional<size_t>& subsong) override;
    void xadplayer_update() override;
    size_t framesUntilUpdate() const override;
    std::string type() const override;
    std::string instrumentTitle(size_t i) const override;
    size_t instrumentCount() const override;

private:
    const uint8_t* m_orderOffsets = nullptr;

    const Instrument *m_instruments = nullptr;

    struct Channel
    {
        uint16_t freq = 0;
        uint16_t freq_slide = 0;
    };

    Channel m_channels[9];

    uint8_t m_speedCounter = 0;
};
