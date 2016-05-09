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
 * [xad] HYP player, by Riven the Mage <riven@ok.ru>
 */

 /*
     - discovery -

   file(s) : HT-EF2.COM, HT-EF3.COM
      type : Eiserne Front BBStro
      tune : by Shadowdancer [Hypnosis]
    player : by (?)Hetero [LKCC/SAC]
 */

#include "hyp.h"

const uint8_t CxadhypPlayer::hyp_adlib_registers[99] = {
    0x20, 0x23, 0x40, 0x43, 0x60, 0x63, 0x80, 0x83, 0xA0, 0xB0, 0xC0, 0x21, 0x24,
    0x41, 0x44, 0x61, 0x64, 0x81, 0x84, 0xA1, 0xB1, 0xC1, 0x22, 0x25, 0x42, 0x45,
    0x62, 0x65, 0x82, 0x85, 0xA2, 0xB2, 0xC2, 0x28, 0x2B, 0x48, 0x4B, 0x68, 0x6B,
    0x88, 0x8B, 0xA3, 0xB3, 0xC3, 0x29, 0x2C, 0x49, 0x4C, 0x69, 0x6C, 0x89, 0x8C,
    0xA4, 0xB4, 0xC4, 0x2A, 0x2D, 0x4A, 0x4D, 0x6A, 0x6D, 0x8A, 0x8D, 0xA5, 0xB5,
    0xC5, 0x30, 0x33, 0x50, 0x53, 0x70, 0x73, 0x90, 0x93, 0xA6, 0xB6, 0xC6, 0x31,
    0x34, 0x51, 0x54, 0x71, 0x74, 0x91, 0x94, 0xA7, 0xB7, 0xC7, 0x32, 0x35, 0x52,
    0x55, 0x72, 0x75, 0x92, 0x95, 0xA8, 0xB8, 0xC8
};

const uint16_t CxadhypPlayer::hyp_notes[73] = {
    0x0000, // by riven
    0x0956, 0x096B, 0x0980, 0x0998, 0x09B1, 0x09C9, 0x09E5, 0x0A03, 0x0A21,
    0x0A41, 0x0A63, 0x0A86, 0x0D56, 0x0D6B, 0x0D80, 0x0D98, 0x0DB1, 0x0DC9,
    0x0DE5, 0x0E03, 0x0E21, 0x0E41, 0x0E63, 0x0E86, 0x1156, 0x116B, 0x1180,
    0x1198, 0x11B1, 0x11C9, 0x11E5, 0x1203, 0x1221, 0x1241, 0x1263, 0x1286,
    0x1556, 0x156B, 0x1580, 0x1598, 0x15B1, 0x15C9, 0x15E5, 0x1603, 0x1621,
    0x1641, 0x1663, 0x1686, 0x1956, 0x196B, 0x1980, 0x1998, 0x19B1, 0x19C9,
    0x19E5, 0x1A03, 0x1A21, 0x1A41, 0x1A63, 0x1A86, 0x1D56, 0x1D6B, 0x1D80,
    0x1D98, 0x1DB1, 0x1DC9, 0x1DE5, 0x1E03, 0x1E21, 0x1E41, 0x1E63, 0x1E86
};

Player* CxadhypPlayer::factory()
{
    return new CxadhypPlayer();
}

void CxadhypPlayer::xadplayer_rewind(int)
{
    int i;

    setCurrentSpeed(tune()[5]);

    getOpl()->writeReg(0xBD, 0xC0);

    for(i = 0; i < 9; i++)
        getOpl()->writeReg(0xB0 + i, 0);

    // define instruments
    for(i = 0; i < 99; i++)
        getOpl()->writeReg(hyp_adlib_registers[i], tune()[6 + i]);

    m_hypPointer = 0x69;
}

void CxadhypPlayer::xadplayer_update()
{
    for(int i = 0; i < 9; i++)
    {
        const auto event = tune()[m_hypPointer++];

        if(event)
        {
            unsigned short freq = hyp_notes[event & 0x3F];

            unsigned char lofreq = (freq & 0xFF);
            unsigned char hifreq = (freq >> 8);

            // FIXME sto getOpl()->writeReg(0xB0 + i, adlib[0xB0 + i]);
            // FIXME sto adlib[0xB0 + i] &= 0xDF;

            if(!(event & 0x40))
            {
                getOpl()->writeReg(0xA0 + i, lofreq);
                getOpl()->writeReg(0xB0 + i, hifreq | 0x20);
            }
        }
    }

    m_hypPointer += 3;

    if(m_hypPointer >= tune().size())
    {
        m_hypPointer = 0x69;
        setXadLooping();
    }
}

size_t CxadhypPlayer::framesUntilUpdate() const
{
    return static_cast<size_t>(SampleRate / 60);
}

std::string CxadhypPlayer::type() const
{
    return "xad: hypnosis player";
}