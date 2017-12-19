#pragma once

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2004 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * lds.h - LOUDNESS Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class LdsPlayer
    : public Player
{
public:
    DISABLE_COPY(LdsPlayer)

    static Player* factory()
    {
        return new LdsPlayer();
    }

    LdsPlayer() = default;

    bool load(const std::string& filename) override;

    bool update() override;

    void rewind(const boost::optional<size_t>& subsong) override;

    size_t framesUntilUpdate() const override
    {
        return SampleRate / 70;
    }

    std::string type() const override
    {
        return "LOUDNESS Sound System";
    }

    size_t instrumentCount() const override
    {
        return m_soundbank.size();
    }

private:
#pragma pack(push, 1)
    struct SoundBank
    {
        uint8_t mod_misc;
        uint8_t mod_vol;
        uint8_t mod_ad;
        uint8_t mod_sr;
        uint8_t mod_wave;
        uint8_t car_misc;
        uint8_t car_vol;
        uint8_t car_ad;
        uint8_t car_sr;
        uint8_t car_wave;
        uint8_t feedback;
        uint8_t keyoff;
        uint8_t portamento;
        uint8_t glide;
        uint8_t finetune;
        uint8_t vibrato;
        uint8_t vibdelay;
        uint8_t mod_trem;
        uint8_t car_trem;
        uint8_t tremwait;
        uint8_t arpeggio;
        std::array<uint8_t, 12> arp_tab;
        uint16_t start;
        uint16_t size;
        uint8_t fms;
        uint16_t transp;
        uint8_t midinst;
        uint8_t midvelo;
        uint8_t midkey;
        uint8_t midtrans;
        uint8_t middum1;
        uint8_t middum2;
    };
#pragma pack(pop)

    struct Channel
    {
        uint16_t gototune = 0;
        uint16_t lasttune = 0;
        uint16_t packpos = 0;
        uint8_t finetune = 0;
        uint8_t glideto = 0;
        uint8_t portspeed = 0;
        uint8_t nextvol = 0;
        uint8_t volmod = 0;
        uint8_t volcar = 0;
        uint8_t vibwait = 0;
        uint8_t vibspeed = 0;
        uint8_t vibrate = 0;
        uint8_t trmstay = 0;
        uint8_t trmwait = 0;
        uint8_t trmspeed = 0;
        uint8_t trmrate = 0;
        uint8_t trmcount = 0;
        uint8_t trcwait = 0;
        uint8_t trcspeed = 0;
        uint8_t trcrate = 0;
        uint8_t trccount = 0;
        uint8_t arp_size = 0;
        uint8_t arp_speed = 0;
        uint8_t keycount = 0;
        uint8_t vibcount = 0;
        uint8_t arp_pos = 0;
        uint8_t arp_count = 0;
        uint8_t packwait = 0;
        std::array<uint8_t, 12> arp_tab{{}};

        Channel()
        {
            arp_tab.fill(0);
        }

        struct ChanCheat
        {
            uint8_t chandelay = 0;
            uint8_t sound = 0;
            uint16_t high = 0;
        };
        ChanCheat chancheat{};
    };

#pragma pack(push, 1)
    struct Position
    {
        uint16_t patnum;
        uint8_t transpose;
    };
#pragma pack(pop)

    std::vector<SoundBank> m_soundbank{};
    std::array<Channel, 9> m_channels{{}};
    std::vector<Position> m_positions{};
    uint8_t m_fadeonoff = 0;
    uint8_t m_allvolume = 0;
    uint8_t m_hardfade = 0;
    uint8_t m_initialTempo = 0;
    uint8_t m_regbd = 0;
    uint8_t m_chandelay[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t m_mode = 0;
    uint8_t m_pattlen = 0;
    std::vector<uint16_t> m_patterns{};
    uint16_t m_jumppos = 0;
    bool m_playing = false;
    bool m_songlooped = false;
    size_t m_mainvolume = 0;

    void playsound(int inst_number, int channel_number, int tunehigh);
};
