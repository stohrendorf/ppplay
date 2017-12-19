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
 * adl.h - ADL player adaption by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class AdlibDriver;

class AdlPlayer
    : public Player
{
public:
    DISABLE_COPY(AdlPlayer)

    static Player* factory();

    AdlPlayer();

    ~AdlPlayer() override = default;

    bool load(const std::string& filename) override;

    bool update() override;

    void rewind(const boost::optional<size_t>& subsong) override;

    // refresh rate is fixed at 72Hz
    size_t framesUntilUpdate() const override
    {
        return SampleRate / 72;
    }

    size_t subSongCount() const override;

    size_t currentSubSong() const override
    {
        return m_currentSubSong;
    }

    std::string type() const override
    {
        return "Westwood ADL";
    }

private:
    size_t m_subSongCount = 0;
    size_t m_currentSubSong = 0;

    std::unique_ptr<AdlibDriver> m_driver;

    std::array<uint8_t, 120> m_trackEntries{{}};
    std::vector<uint8_t> m_soundDataPtr{};
    int m_sfxPlayingSound = -1;

    uint8_t m_sfxPriority = 0;
    uint8_t m_sfxFourthByteOfSong = 0;

    bool init();

    void process();

    void playTrack(uint8_t track);

    void playSoundEffect(uint8_t track);

    void play(uint8_t track);

    void unk1();

    void unk2();
};
