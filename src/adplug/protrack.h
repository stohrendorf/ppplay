/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
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
 * protrack.h - Generic Protracker Player by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_PROTRACK
#define H_PROTRACK

#include "player.h"
#include "stuff/field.h"

#include <boost/optional.hpp>

class CmodPlayer : public CPlayer {
    DISABLE_COPY(CmodPlayer)
    public:
        CmodPlayer();
    virtual ~CmodPlayer() = default;

    bool update();
    void rewind(int);
    size_t framesUntilUpdate();

    unsigned int getpatterns() { return numberOfPatterns; }
    unsigned int getpattern() { return m_order[ord]; }
    unsigned int getorders() { return m_length; }
    unsigned int getorder() { return ord; }
    unsigned int getrow() { return rw; }
    unsigned int getspeed() { return speed; }

protected:
    enum Flags {
        Standard = 0,
        Decimal = 1 << 0,
        Faust = 1 << 1,
        NoKeyOn = 1 << 2,
        Opl3 = 1 << 3,
        Tremolo = 1 << 4,
        Vibrato = 1 << 5,
        Percussion = 1 << 6
    };

    struct Instrument {
        uint8_t data[11]{0}, arpstart = 0, arpspeed = 0, arppos = 0, arpspdcnt = 0, misc = 0;
        int8_t slide = 0;
    };
    std::vector<Instrument> m_instruments{};

    struct Track {
        unsigned char note = 0, command = 0, inst = 0, param2 = 0, param1 = 0;

        Track() = default;
    };
    Field<Track> m_tracks{};

    std::vector<uint8_t> m_order{};
    uint8_t m_initspeed = 6;
    uint16_t m_tempo = 0, m_bpm = 0, numberOfPatterns = 0;
    unsigned long m_length = 0, m_restartpos = 0, activechan = 0xffffffff;
    int m_flags = Standard;
    Field<uint16_t> trackord{};
    boost::optional<std::array<uint8_t,256>> arplist{};
    boost::optional<std::array<uint8_t,256>> arpcmd{};

    struct Channel {
        unsigned short freq = 0, nextfreq = 0;
        unsigned char oct = 0, vol1 = 0, vol2 = 0, inst = 0, fx = 0, info1 = 0, info2 = 0, key = 0, nextoct = 0, note = 0,
        portainfo = 0, vibinfo1 = 0, vibinfo2 = 0, arppos = 0, arpspdcnt = 0;
        signed char trigger = 0;
    };
    std::vector<Channel> channel{};

    void init_trackord();
    void init_notetable(const std::array<uint16_t,12> &newnotetable);
    void realloc_patterns(unsigned long pats, unsigned long rows,
                          unsigned long chans);

private:
    static const unsigned char vibratotab[32];

    unsigned char speed = 0, del = 0, songend = 0, regbd = 0;
    std::array<uint16_t,12> notetable{{0,0,0,0,0,0,0,0,0,0,0,0}};
    unsigned long rw = 0, ord = 0, nrows = 0, npats = 0;

    void setvolume(unsigned char chan);
    void setvolume_alt(unsigned char chan);
    void setfreq(unsigned char chan);
    void playnote(unsigned char chan);
    void setnote(unsigned char chan, int note);
    void slide_down(unsigned char chan, int amount);
    void slide_up(unsigned char chan, int amount);
    void tone_portamento(unsigned char chan, unsigned char info);
    void vibrato(unsigned char chan, unsigned char speed, unsigned char depth);
    void vol_up(unsigned char chan, int amount);
    void vol_down(unsigned char chan, int amount);
    void vol_up_alt(unsigned char chan, int amount);
    void vol_down_alt(unsigned char chan, int amount);

    void dealloc_patterns();
    bool resolve_order();
    unsigned char set_opl_chip(unsigned char chan);
};

#endif
