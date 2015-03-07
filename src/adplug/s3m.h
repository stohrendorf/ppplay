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

#ifndef H_ADPLUG_S3M
#define H_ADPLUG_S3M

#include "player.h"

class FileStream;

class Cs3mPlayer : public CPlayer {
    DISABLE_COPY(Cs3mPlayer)
    public:
        static CPlayer *factory();

    Cs3mPlayer();

    bool load(const std::string &filename);
    bool update();
    void rewind(int);
    size_t framesUntilUpdate();

    std::string gettype();
    std::string gettitle() { return std::string(m_header.name); }

    unsigned int getpatterns() { return m_header.patnum; }
    unsigned int getpattern() { return m_orders[m_order]; }
    unsigned int getorders() { return (m_header.ordnum - 1); }
    unsigned int getorder() { return m_order; }
    unsigned int getrow() { return m_crow; }
    unsigned int getspeed() { return m_speed; }
    unsigned int getinstruments() { return m_header.insnum; }
    std::string getinstrument(unsigned int n) {
        return std::string(m_instruments[n].name);
    }

protected:
#pragma pack(push,1)
    struct S3mHeader {
        char name[28]; // song name
        unsigned char kennung, typ, dummy[2];
        uint16_t ordnum, insnum, patnum, flags, cwtv, ffi;
        char scrm[4];
        uint8_t gv, is, it, mv, uc, dp, dummy2[8];
        uint16_t special;
        uint8_t chanset[32];
    };

    struct S3mInstrument {
        uint8_t type;
        char filename[15];
        uint8_t d00, d01, d02, d03, d04, d05, d06, d07, d08, d09, d0a, d0b, volume, dsk, dummy[2];
        uint32_t c2spd;
        char dummy2[12], name[28], scri[4];
    };
#pragma pack(pop)
    S3mInstrument m_instruments[99];

    struct S3mPattern {
        uint8_t note, octave, instrument, volume, effect, effectValue;
    };
    S3mPattern m_patterns[99][64][32];

    struct {
        uint16_t frequency, nextFrequency;
        uint8_t octave, volume, instrument, effect, effectValue, dualInfo, key, nextOctave, trigger, note;
    } m_channels[9];

    S3mHeader m_header;
    uint8_t m_orders[256];
    uint8_t m_crow, m_order, m_speed, m_tempo, m_patternDelay, songend, m_loopStart, m_loopCounter;

private:
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

#endif
