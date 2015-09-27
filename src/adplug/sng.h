/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2002 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * sng.h - SNG Player by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_SNGPLAYER
#define H_ADPLUG_SNGPLAYER

#include "player.h"

class CsngPlayer : public CPlayer {
    DISABLE_COPY(CsngPlayer)
public:
    CsngPlayer() = default;
    static CPlayer *factory();

    bool load(const std::string &filename);
    bool update();
    void rewind(int subsong);
    size_t framesUntilUpdate() const
    {
        return SampleRate / 70;
    }

    std::string type() const
    {
        return "SNG File Format";
    }

private:
#pragma pack(push,1)
    struct SngHeader
    {
        char id[4] = { 0, 0, 0, 0 };
        uint16_t length = 0, start = 0, loop = 0;
        uint8_t delay = 0;
        uint8_t compressed = false;
    };
    struct Sdata {
        uint8_t val = 0, reg = 0;
    };
#pragma pack(pop)
    SngHeader m_header{};

    std::vector<Sdata> m_data{};

    unsigned char m_del = 0;
    unsigned short m_pos = 0;
    bool m_songEnd = false;
};

#endif
