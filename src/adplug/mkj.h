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
 * mkj.h - MKJamz Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class CmkjPlayer : public CPlayer {
    DISABLE_COPY(CmkjPlayer)
    public:
        static CPlayer *factory();

    CmkjPlayer() : CPlayer() {}
    ~CmkjPlayer() = default;

    bool load(const std::string &filename);
    bool update();
    void rewind(int);
    size_t framesUntilUpdate();

    std::string gettype() { return std::string("MKJamz Audio File"); }

private:
    int16_t m_maxChannel;
    int16_t m_maxNotes;
    std::vector<int16_t> m_songBuf;
    bool songend;

    struct {
        int16_t defined;
        short songptr, octave, waveform, pstat, speed, delay;
    } m_channels[9];
};
