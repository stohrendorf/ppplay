#pragma once

/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * d00.h - D00 Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class D00Player : public Player
{
    DISABLE_COPY(D00Player)
public:
    static Player *factory();

    D00Player() = default;

    ~D00Player() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(const boost::optional<size_t>& subsong) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override;
    std::string title() const override
    {
        if(version > 1)
            return header->songname;
        else
            return std::string();
    }
    std::string author() const override
    {
        if(version > 1)
            return header->author;
        else
            return std::string();
    }
    std::string description() const override
    {
        if(*datainfo)
            return datainfo;
        else
            return std::string();
    }
    size_t subSongCount() const override;

private:
#pragma pack(push,1)
    struct d00header
    {
        char id[6];
        uint8_t type, version, speed, subsongs, soundcard;
        char songname[32], author[32], dummy[32];
        uint16_t tpoin, seqptr, instptr, infoptr, spfxptr, endmark;
    };

    struct d00header1
    {
        uint8_t version, speed, subsongs;
        uint16_t tpoin, seqptr, instptr, infoptr, lpulptr, endmark;
    };
#pragma pack(pop)

    struct
    {
        const uint16_t *order;
        uint16_t ordpos, pattpos, del, speed, rhcnt, key, freq, inst,
            spfx, ispfx, irhcnt;
        signed short transpose, slide, slideval, vibspeed;
        uint8_t seqend, vol, vibdepth, fxdel, modvol, cvol, levpuls,
            frameskip, nextnote, note, ilevpuls, trigger, fxflag;
    } channel[9];

    struct Sinsts
    {
        uint8_t data[11], tunelev, timer, sr, dummy[2];
    };
    const Sinsts* inst = nullptr;

    struct Sspfx
    {
        uint16_t instnr;
        int8_t halfnote;
        uint8_t modlev;
        int8_t modlevadd;
        uint8_t duration;
        uint16_t ptr;
    };
    const Sspfx* spfx = nullptr;

    struct Slevpuls
    {
        uint8_t level;
        int8_t voladd;
        uint8_t duration, ptr;
    };
    const Slevpuls* levpuls = nullptr;

    bool songend = false;
    uint8_t version = 0;
    size_t cursubsong = 0;
    char *datainfo = nullptr;
    const uint16_t *seqptr = nullptr;
    d00header *header = nullptr;
    d00header1 *header1 = nullptr;
    std::vector<char> filedata{};

    void setvolume(uint8_t chan);
    void setfreq(uint8_t chan);
    void setinst(uint8_t chan);
    void playnote(uint8_t chan);
    void vibrato(uint8_t chan);
};
