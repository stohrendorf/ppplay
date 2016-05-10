#pragma once

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * adtrack.h - Adlib Tracker 1.0 Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "protrack.h"

class CadtrackLoader : public CmodPlayer
{
    DISABLE_COPY(CadtrackLoader)
public:
    static Player *factory();

    CadtrackLoader() = default;

    bool load(const std::string &filename) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override
    {
        return "Adlib Tracker 1.0";
    }
    size_t instrumentCount() const override
    {
        return 9;
    }

private:
    enum Operators
    {
        Carrier = 1,
        Modulator = 0
    };

#pragma pack(push,1)
    struct Instrument
    {
        struct Operator
        {
            uint16_t appampmod;
            uint16_t appvib;
            uint16_t maintsuslvl;
            uint16_t keybscale;
            uint16_t octave;
            uint16_t freqrisevollvldn;
            uint16_t softness;
            uint16_t attack;
            uint16_t decay;
            uint16_t release;
            uint16_t sustain;
            uint16_t feedback;
            uint16_t waveform;
        };
        Operator op[2];
    };
#pragma pack(pop)

    using CmodPlayer::addInstrument;
    void addInstrument(Instrument *i);
};
