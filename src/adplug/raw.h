#pragma once

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2005 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * raw.h - RAW Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class RawPlayer : public Player
{
    DISABLE_COPY(RawPlayer)
public:
    static Player *factory();

    RawPlayer() = default;

    ~RawPlayer() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(int) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override
    {
        return "RdosPlay RAW";
    }

private:
#pragma pack(push,1)
    struct TrackData
    {
        uint8_t param, command;
    };
#pragma pack(pop)
    std::vector<TrackData> m_data{};

    size_t m_dataPosition = 0;
    uint8_t m_delay = 0;
    bool m_songend = false;
};
