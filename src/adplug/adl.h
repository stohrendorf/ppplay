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

#ifndef H_ADPLUG_ADLPLAYER
#define H_ADPLUG_ADLPLAYER

#include "player.h"

class AdlibDriver;

class CadlPlayer : public CPlayer {
    DISABLE_COPY(CadlPlayer)
public:
    static CPlayer *factory();

    CadlPlayer();
    ~CadlPlayer() = default;

    bool load(const std::string &filename);
    bool update();
    void rewind(int subsong = -1);

    // refresh rate is fixed at 72Hz
    size_t framesUntilUpdate() const { return SampleRate / 72; }

    unsigned int subSongCount() const;
    unsigned int currentSubSong() const
    {
        return m_currentSubSong;
    }

    std::string type() const
    {
        return "Westwood ADL";
    }

private:
    int m_subSongCount = 0;
    int m_currentSubSong = 0;

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

#endif
