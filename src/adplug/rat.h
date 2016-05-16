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
 * [xad] RAT player, by Riven the Mage <riven@ok.ru>
 */

#include "xad.h"

class RatPlayer : public XadPlayer
{
    DISABLE_COPY(RatPlayer)
public:
    static Player *factory();

    RatPlayer() = default;

private:
#pragma pack(push,1)
    struct Header
    {
        char id[3] = "";
        uint8_t version = 0;
        char title[32] = "";
        uint8_t numchan = 0;
        uint8_t reserved_25 = 0;
        uint8_t order_end = 0;
        uint8_t reserved_27 = 0;
        uint8_t numinst = 0; // ?: Number of Instruments
        uint8_t reserved_29 = 0;
        uint8_t numpat = 0; // ?: Number of Patterns
        uint8_t reserved_2B = 0;
        uint8_t order_start = 0;
        uint8_t reserved_2D = 0;
        uint8_t order_loop = 0;
        uint8_t reserved_2F = 0;
        uint8_t volume = 0;
        uint8_t speed = 0;
        uint8_t reserved_32[12] = "";
        uint8_t patseg[2] = { 0,0 };
    };

    struct Event
    {
        uint8_t note;
        uint8_t instrument;
        uint8_t volume;
        uint8_t fx;
        uint8_t fxp;
    };

    struct Instrument
    {
        uint8_t freq[2];
        uint8_t reserved_2[2];
        uint8_t mod_ctrl;
        uint8_t car_ctrl;
        uint8_t mod_volume;
        uint8_t car_volume;
        uint8_t mod_AD;
        uint8_t car_AD;
        uint8_t mod_SR;
        uint8_t car_SR;
        uint8_t mod_wave;
        uint8_t car_wave;
        uint8_t connect;
        uint8_t reserved_F;
        uint8_t volume;
        uint8_t reserved_11[3];
    };

#pragma pack(pop)

    bool xadplayer_load() override;
    void xadplayer_rewind(const boost::optional<size_t>& subsong) override;
    void xadplayer_update() override;
    size_t framesUntilUpdate() const override;
    std::string type() const override;
    std::string title() const override;
    size_t instrumentCount() const override;

    const uint8_t* m_trackdataByOrder = nullptr;
    Header m_ratHeader{};

    uint8_t m_volume = 0;

    const Instrument *m_instruments = nullptr;

    std::array<std::array<std::array<Event, 9>, 64>, 256> m_tracks{ {} };

    struct Channel
    {
        uint8_t instrument = 0;
        uint8_t volume = 0;
        uint8_t fx = 0;
        uint8_t fxp = 0;
    };

    std::array<Channel, 9> m_channels{ {} };

    static uint8_t calculateVolume(uint8_t ivol, uint8_t cvol, uint8_t gvol);
};
