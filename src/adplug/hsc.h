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
 * hsc.h - HSC Player by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_HSCPLAYER
#define H_ADPLUG_HSCPLAYER

#include "player.h"

class ChscPlayer : public CPlayer {
    DISABLE_COPY(ChscPlayer)
    public:
        static CPlayer *factory();

    explicit ChscPlayer(bool mtkMode)
        : CPlayer()
        , m_mtkmode(mtkMode)
    {
    }

    bool load(const std::string &filename);
    bool update();
    void rewind(int);
    size_t framesUntilUpdate() const
    {
        return static_cast<size_t>(SampleRate / 18.2);
    } // refresh rate is fixed at 18.2Hz

    std::string type() const
    {
        return "HSC Adlib Composer / HSC-Tracker";
    }
    uint32_t instrumentCount() const;

protected:
    struct HscNote {
        unsigned char note = 0, effect = 0;
    }; // note type in HSC pattern

    struct HscChan {
        unsigned char inst = 0;  // current instrument
        signed char slide = 0;   // used for manual slide-effects
        unsigned short freq = 0; // actual replaying frequency
    };


private:
    HscChan m_channel[9];           // player channel-info
    uint8_t m_instr[128][12]; // instrument data
    HscNote m_patterns[50][64 * 9]; // pattern data
    unsigned char // various bytes & flags
    m_pattbreak = 0, m_mode6 = 0, m_bd = 0, m_fadein = 0;
    unsigned int m_del = 0;
    unsigned char m_adlFreq[9]; // adlib frequency registers
    bool m_mtkmode = false;          // flag: MPU-401 Trakker mode on/off
    bool m_songend = false;

    void setfreq(unsigned char chan, unsigned short freq);
    void setvolume(unsigned char chan, int volc, int volm);
    void setinstr(unsigned char chan, unsigned char insnr);

protected:
    decltype(m_instr)& instrumentData()
    {
        return m_instr;
    }
    decltype(m_patterns)& patternData()
    {
        return m_patterns;
    }
};

#endif
