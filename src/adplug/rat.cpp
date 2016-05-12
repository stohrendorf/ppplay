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

 /*
     - discovery -

   file(s) : PINA.EXE
      type : Experimental Connection BBStro tune
      tune : by (?)Ratt/GRIF
    player : by (?)Ratt/GRIF
   comment : there are bug in original replayer's adlib_init(): wrong frequency
   registers.
 */

#include <cstring>
#include "rat.h"

namespace
{
constexpr uint8_t rat_adlib_bases[18] = {
    0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12, 0x03, 0x04, 0x05, 0x0B,
    0x0C, 0x0D, 0x13, 0x14, 0x15
};

constexpr uint16_t rat_notes[16] = {
    0x157, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263,
    0x287, 0x000, 0x000, 0x000, 0x000 // by riven
};
}

Player* RatPlayer::factory()
{
    return new RatPlayer();
}

bool RatPlayer::xadplayer_load()
{
    if(xadHeader().fmt != RAT)
        return false;

    // load header
    std::copy_n(tune().begin(), sizeof(Header), reinterpret_cast<uint8_t*>(&m_ratHeader));

    // is 'RAT'-signed ?
    if(!std::equal(m_ratHeader.id, m_ratHeader.id + 3, "RAT"))
        return false;

    // is version 1.0 ?
    if(m_ratHeader.version != 0x10)
        return false;

    // load order
    m_trackdataByOrder = &tune()[0x40];

    // load instruments
    m_instruments = reinterpret_cast<const Instrument *>(&tune()[0x140]);

    // load pattern data
    unsigned short patseg = (m_ratHeader.patseg[1] << 8) + m_ratHeader.patseg[0];
    const Event* event_ptr = reinterpret_cast<const Event*>(&tune()[patseg << 4]);

    for(int i = 0; i < m_ratHeader.numpat; i++)
        for(int j = 0; j < 64; j++)
            for(int k = 0; k < m_ratHeader.numchan; k++)
            {
                m_tracks[i][j][k] = *event_ptr;
                ++event_ptr;
            }

    return true;
}

void RatPlayer::xadplayer_rewind(int)
{
    setCurrentOrder(m_ratHeader.order_start);
    setCurrentRow(0);
    m_volume = m_ratHeader.volume;

    setCurrentSpeed(m_ratHeader.speed);

    // clear channel data
    m_channels.fill(Channel());

    // init OPL
    getOpl()->writeReg(0x01, 0x20);
    getOpl()->writeReg(0x08, 0x00);
    getOpl()->writeReg(0xBD, 0x00);

    // set default frequencies
    for(int i = 0; i < 9; i++)
    {
        getOpl()->writeReg(0xA0 + i, 0x00);
        getOpl()->writeReg(0xA3 + i, 0x00);
        getOpl()->writeReg(0xB0 + i, 0x00);
        getOpl()->writeReg(0xB3 + i, 0x00);
    }

    // set default volumes
    for(int i = 0; i < 0x1F; i++)
        getOpl()->writeReg(0x40 + i, 0x3F);
}

void RatPlayer::xadplayer_update()
{
    // process events
    for(int i = 0; i < m_ratHeader.numchan; i++)
    {
        const Event& event = reinterpret_cast<const Event&>(m_tracks[m_trackdataByOrder[currentOrder()]][currentRow()][i]);

        // is instrument ?
        if(event.instrument != 0xFF)
        {
            m_channels[i].instrument = event.instrument - 1;
            m_channels[i].volume = m_instruments[event.instrument - 1].volume;
        }

        // is volume ?
        if(event.volume != 0xFF)
            m_channels[i].volume = event.volume;

        // is note ?
        if(event.note != 0xFF)
        {
            // mute channel
            getOpl()->writeReg(0xB0 + i, 0x00);
            getOpl()->writeReg(0xA0 + i, 0x00);

            // if note != 0xFE then play
            if(event.note != 0xFE)
            {
                const auto ins = m_channels[i].instrument;

                // synthesis/feedback
                getOpl()->writeReg(0xC0 + i, m_instruments[ins].connect);

                // controls
                getOpl()->writeReg(0x20 + rat_adlib_bases[i], m_instruments[ins].mod_ctrl);
                getOpl()->writeReg(0x20 + rat_adlib_bases[i + 9], m_instruments[ins].car_ctrl);

                // volumes
                getOpl()->writeReg(0x40 + rat_adlib_bases[i],
                                   calculateVolume(m_instruments[ins].mod_volume,
                                                   m_channels[i].volume, m_volume));
                getOpl()->writeReg(0x40 + rat_adlib_bases[i + 9],
                                   calculateVolume(m_instruments[ins].car_volume,
                                                   m_channels[i].volume, m_volume));

                // attack/decay
                getOpl()->writeReg(0x60 + rat_adlib_bases[i], m_instruments[ins].mod_AD);
                getOpl()->writeReg(0x60 + rat_adlib_bases[i + 9], m_instruments[ins].car_AD);

                // sustain/release
                getOpl()->writeReg(0x80 + rat_adlib_bases[i], m_instruments[ins].mod_SR);
                getOpl()->writeReg(0x80 + rat_adlib_bases[i + 9], m_instruments[ins].car_SR);

                // waveforms
                getOpl()->writeReg(0xE0 + rat_adlib_bases[i], m_instruments[ins].mod_wave);
                getOpl()->writeReg(0xE0 + rat_adlib_bases[i + 9], m_instruments[ins].car_wave);

                // octave/frequency
                unsigned short insfreq =
                    (m_instruments[ins].freq[1] << 8) + m_instruments[ins].freq[0];
                unsigned short freq = insfreq * rat_notes[event.note & 0x0F] / 0x20AB;

                getOpl()->writeReg(0xA0 + i, freq & 0xFF);
                getOpl()->writeReg(0xB0 + i, (freq >> 8) | ((event.note & 0xF0) >> 2) | 0x20);
            }
        }

        // is effect ?
        if(event.fx != 0xFF)
        {
            m_channels[i].fx = event.fx;
            m_channels[i].fxp = event.fxp;
        }
    }

    // next row
    setCurrentRow(currentRow() + 1);

    // process effects
    for(int i = 0; i < m_ratHeader.numchan; i++)
    {
        const auto old_order_pos = currentOrder();

        switch(m_channels[i].fx)
        {
            case 0x01: // 0x01: Set Speed
                setCurrentSpeed(m_channels[i].fxp);
                break;
            case 0x02: // 0x02: Position Jump
                if(m_channels[i].fxp < m_ratHeader.order_end)
                    setCurrentOrder(m_channels[i].fxp);
                else
                    setCurrentOrder(0);

                // jumpback ?
                if(currentOrder() <= old_order_pos)
                    setXadLooping();

                setCurrentRow(0);
                break;
            case 0x03: // 0x03: Pattern Break (?)
                setCurrentRow(0x40);
                break;
        }

        m_channels[i].fx = 0;
    }

    // end of pattern ?
    if(currentRow() >= 0x40)
    {
        setCurrentRow(0);

        setCurrentOrder(currentOrder() + 1);

        // end of module ?
        if(currentOrder() == m_ratHeader.order_end)
        {
            setCurrentOrder(m_ratHeader.order_loop);

            setXadLooping();
        }
    }
}

size_t RatPlayer::framesUntilUpdate() const
{
    return SampleRate / 60;
}

std::string RatPlayer::type() const
{
    return "xad: rat player";
}

std::string RatPlayer::title() const
{
    return std::string(m_ratHeader.title, 32);
}

size_t RatPlayer::instrumentCount() const
{
    return m_ratHeader.numinst;
}

/* -------- Internal Functions ---------------------------- */

uint8_t RatPlayer::calculateVolume(uint8_t ivol, uint8_t cvol, uint8_t gvol)
{
    uint16_t vol;

    vol = ivol;
    vol &= 0x3F;
    vol ^= 0x3F;
    vol *= cvol;
    vol >>= 6;
    vol *= gvol;
    vol >>= 6;
    vol ^= 0x3F;

    vol |= ivol & 0xC0;

    return vol;
}