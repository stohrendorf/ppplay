#pragma once

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * s3m.h - AdLib S3M Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class FileStream;

class S3mPlayer : public Player
{
    DISABLE_COPY(S3mPlayer)
public:
    static Player *factory();

    S3mPlayer();

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(const boost::optional<size_t>& subsong) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override;
    std::string title() const override
    {
        return m_header.name;
    }

    size_t instrumentCount() const override
    {
        return m_header.instrumentCount;
    }
    std::string instrumentTitle(size_t n) const override
    {
        return m_instruments[n].name;
    }

protected:
#pragma pack(push,1)
    struct S3mHeader
    {
        char name[28] = ""; // song name
        uint8_t endOfFile = 0;
        uint8_t type = 0;
        uint8_t dummy[2];
        uint16_t orderCount = 0;
        uint16_t instrumentCount = 0;
        uint16_t patternCount = 0;
        uint16_t flags = 0;
        uint16_t trackerVersion = 0;
        uint16_t ffi = 0;
        char scrm[4] = "";
        uint8_t gv = 0;
        uint8_t initialSpeed = 0;
        uint8_t initialTempo = 0;
        uint8_t mv = 0;
        uint8_t uc = 0;
        uint8_t dp = 0;
        uint8_t dummy2[8];
        uint16_t special = 0;
        std::array<uint8_t, 32> chanset{ {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0} };
    };

    struct S3mInstrument
    {
        uint8_t type = 0;
        char filename[15] = "";
        uint8_t d00 = 0, d01 = 0, d02 = 0, d03 = 0, d04 = 0, d05 = 0, d06 = 0, d07 = 0, d08 = 0, d09 = 0, d0a = 0, d0b = 0, volume = 0, dsk = 0, dummy[2];
        uint32_t c2spd = 0;
        char dummy2[12], name[28] = "", scri[4] = "";
    };
#pragma pack(pop)

    void setInstrument(size_t i, const S3mInstrument& instrument)
    {
        BOOST_ASSERT(i < 99);
        m_instruments[i] = instrument;
    }

    struct S3mCell
    {
        uint8_t note, octave, instrument, volume, effect, effectValue;
    };

    S3mCell* patternChannel(size_t pattern, size_t row)
    {
        BOOST_ASSERT(pattern < 99);
        BOOST_ASSERT(row < 64);
        return m_patterns[pattern][row];
    }

    void setHeader(const S3mHeader& header)
    {
        m_header = header;
    }

private:
    S3mInstrument m_instruments[99];

    S3mCell m_patterns[99][64][32];

    struct Channel
    {
        uint16_t frequency;
        uint16_t nextFrequency;
        uint8_t octave;
        uint8_t volume;
        uint8_t instrument;
        uint8_t effect;
        uint8_t effectValue;
        uint8_t dualInfo;
        uint8_t key;
        uint8_t nextOctave;
        uint8_t trigger;
        uint8_t note;
    };
    
    Channel m_channels[9];

    S3mHeader m_header{};
    uint8_t m_patternDelay = 0;
    bool m_songend = false;
    uint8_t m_loopStart = 0;
    uint8_t m_loopCounter = 0;

    static const char chnresolv[];
    static const unsigned short notetable[12];
    static const unsigned char vibratotab[32];

    void setvolume(unsigned char chan);
    void setfreq(unsigned char chan);
    void playnote(unsigned char chan);
    void slide_down(unsigned char chan, unsigned char amount);
    void slide_up(unsigned char chan, unsigned char amount);
    void vibrato(unsigned char chan, unsigned char effectValue);
    void tone_portamento(unsigned char chan, unsigned char effectValue);
};
