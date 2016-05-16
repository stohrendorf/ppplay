#pragma once

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
 * ksm.h - KSM Player for AdPlug by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class FileStream;

class KsmPlayer : public Player
{
    DISABLE_COPY(KsmPlayer)
public:
    static Player *factory();

    KsmPlayer() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(const boost::optional<size_t>& subsong) override;
    size_t framesUntilUpdate() const override
    {
        return SampleRate / 240;
    }

    std::string type() const override
    {
        return "Ken Silverman's Music Format";
    }
    size_t instrumentCount() const override
    {
        return 16;
    }
    std::string instrumentTitle(size_t n) const override;

private:
    std::vector<uint32_t> m_notes{};
    size_t m_updateCounter = 0;
    size_t m_nextUpdate = 0;
    size_t m_slotAges[18] = { 0 };
    size_t m_notesOffset = 0;
    uint8_t m_bdRegister = 0;
    size_t m_channelCount = 0;
    uint8_t m_channelInstruments[16] = { 0 };
    uint8_t m_quanters[16] = { 0 };
    uint8_t m_channelFlags[16] = { 0 };
    uint8_t m_channelVolumes[16] = { 0 };
    uint8_t m_channelNotes[18] = { 0 };
    uint8_t m_slotToTrack[18] = { 0 };
    char m_instrumentNames[256][20] = { "" };
    std::array<uint8_t, 11> m_instruments[256];

    bool m_songEnd = false;

    void loadInstruments(FileStream& f);
    void storeInstrument(int chan, const std::array<uint8_t, 11>& data);
};
