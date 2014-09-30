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

class CldsPlayer: public CPlayer
{
public:
    static CPlayer *factory() { return new CldsPlayer(); }

    CldsPlayer();

    bool load(const std::string &filename, const CFileProvider &fp);
    virtual bool update();
    virtual void rewind(int subsong = -1);
    size_t framesUntilUpdate() override {
        return SampleRate/70;
    }

    std::string gettype() { return std::string("LOUDNESS Sound System"); }
    unsigned int getorders() { return m_numposi; }
    unsigned int getorder() { return m_posplay; }
    unsigned int getrow() { return m_pattplay; }
    unsigned int getspeed() { return m_speed; }
    unsigned int getinstruments() { return m_soundbank.size(); }

private:
#pragma pack(push,1)
    struct SoundBank {
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
        std::array<uint8_t,12> arp_tab;
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

    struct Channel {
        uint16_t gototune, lasttune, packpos;
        uint8_t finetune, glideto, portspeed, nextvol, volmod, volcar,
        vibwait, vibspeed, vibrate, trmstay, trmwait, trmspeed, trmrate, trmcount,
        trcwait, trcspeed, trcrate, trccount, arp_size, arp_speed, keycount,
        vibcount, arp_pos, arp_count, packwait;
        std::array<uint8_t,12> arp_tab;

        struct {
            uint8_t chandelay, sound;
            uint16_t high;
        } chancheat;
    };

#pragma pack(push,1)
    struct Position {
        uint16_t patnum;
        uint8_t transpose;
    };
#pragma pack(pop)

    std::vector<SoundBank> m_soundbank;
    Channel m_channels[9];
    std::vector<Position> m_positions;
    uint8_t m_jumping, m_fadeonoff, m_allvolume, m_hardfade, m_tempoNow, m_pattplay, m_tempo, m_regbd, m_chandelay[9], m_mode, m_pattlen;
    std::vector<uint16_t> m_patterns;
    uint16_t m_posplay, m_jumppos, m_speed;
    bool m_playing, m_songlooped;
    uint16_t m_numposi;
    size_t m_patternsSize, m_mainvolume;

    void playsound(int inst_number, int channel_number, int tunehigh);
};
