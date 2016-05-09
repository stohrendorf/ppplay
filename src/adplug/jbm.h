/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * jbm.h - JBM Player by Dennis Lindroos <lindroos@nls.fi>
 */

#ifndef H_ADPLUG_JBMPLAYER
#define H_ADPLUG_JBMPLAYER

#include "player.h"

class CjbmPlayer : public CPlayer {
    DISABLE_COPY(CjbmPlayer)
    public:
        CjbmPlayer() = default;
    static CPlayer *factory();

    bool load(const std::string &filename);
    bool update();
    void rewind(int subsong);

    size_t framesUntilUpdate() const
    {
        return static_cast<size_t>(SampleRate / m_timer);
    }

    std::string type() const
    {
        return (m_flags & 1) ? "JBM Adlib Music [rhythm mode]" : "JBM Adlib Music";
    }
    std::string author() const
    {
        return "Johannes Bjerregaard";
    }

private:
    std::vector<uint8_t> m_fileData{};
    float m_timer = 0;
    uint16_t m_flags = 0;
    uint16_t m_voicemask = 0;
    uint16_t m_seqTable = 0;
    uint16_t m_seqCount = 0;
    uint16_t m_insTable = 0;
    uint16_t m_insCount = 0;
    std::vector<uint16_t> m_sequences{};

    struct JBMVoice {
        uint16_t trkpos = 0;
        uint16_t trkstart = 0;
        uint16_t seqpos = 0;
        uint8_t seqno = 0;
        uint8_t note = 0;
        int16_t vol = 0;
        int16_t delay = 0;
        int16_t instr = 0;
        uint8_t frq[2] = { 0, 0 };
        uint8_t ivol = 0;
        uint8_t dummy = 0;
    };

    JBMVoice m_voices[11];

    void set_opl_instrument(int, JBMVoice *voice);
    void opl_noteonoff(int, bool);
};

#endif
