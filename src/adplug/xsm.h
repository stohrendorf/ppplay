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
 * xsm.h - eXtra Simple Music Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class XsmPlayer : public Player
{
    DISABLE_COPY(XsmPlayer)
public:
    static Player *factory()
    {
        return new XsmPlayer();
    }

    XsmPlayer() = default;
    ~XsmPlayer() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(int subsong) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override
    {
        return "eXtra Simple Music";
    }

private:
    struct Row
    {
        uint8_t data[9] = { 0,0,0,0,0,0,0,0,0 };
    };

    std::vector<Row> m_music{};
    size_t m_lastRow = 0;
    size_t m_currentRow = 0;
    bool m_songEnd = false;

    void playNote(int c, int note, int octv);
};
