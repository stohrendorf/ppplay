#pragma once

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

#include "player.h"

class HscPlayer : public Player
{
    DISABLE_COPY(HscPlayer)
public:
    static Player *factory();

    explicit HscPlayer(bool mtkMode)
        : Player()
        , m_mtkmode(mtkMode)
    {
    }

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(const boost::optional<size_t>& subsong) override;
    size_t framesUntilUpdate() const override
    {
        return static_cast<size_t>(SampleRate / 18.2);
    } // refresh rate is fixed at 18.2Hz

    std::string type() const override
    {
        return "HSC Adlib Composer / HSC-Tracker";
    }
    size_t instrumentCount() const override;

protected:
    struct HscNote
    {
        uint8_t note = 0, effect = 0;
    }; // note type in HSC pattern

    struct HscChan
    {
        uint8_t inst = 0;  // current instrument
        int8_t slide = 0;   // used for manual slide-effects
        uint16_t freq = 0; // actual replaying frequency
    };

private:
    HscChan m_channel[9];           // player channel-info
    uint8_t m_instr[128][12]; // instrument data
    HscNote m_patterns[50][64 * 9]; // pattern data
    uint8_t // various bytes & flags
        m_pattbreak = 0, m_mode6 = 0, m_bd = 0, m_fadein = 0;
    unsigned int m_del = 0;
    uint8_t m_adlFreq[9]; // adlib frequency registers
    bool m_mtkmode = false;          // flag: MPU-401 Trakker mode on/off
    bool m_songend = false;

    void setfreq(uint8_t chan, unsigned short freq);
    void setvolume(uint8_t chan, int volc, int volm);
    void setinstr(uint8_t chan, uint8_t insnr);

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
