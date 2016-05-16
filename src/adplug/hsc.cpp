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
 * hsc.cpp - HSC Player by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "hsc.h"

 /*** public methods **************************************/

Player* HscPlayer::factory()
{
    return new HscPlayer(false);
}

bool HscPlayer::load(const std::string& filename)
{
    FileStream f(filename);

    // file validation section
    if(!f || f.extension() != ".hsc" || f.size() > 59187)
    {
        return false;
    }

    // load section
    f.read(reinterpret_cast<uint8_t*>(m_instr), 128 * 12);
    for(int i = 0; i < 128; i++)
    { // correct instruments
        m_instr[i][2] ^= (m_instr[i][2] & 0x40) << 1;
        m_instr[i][3] ^= (m_instr[i][3] & 0x40) << 1;
        m_instr[i][11] >>= 4; // slide
    }
    for(int i = 0; i < 51; ++i)
    {
        uint8_t tmp;
        f >> tmp;
        if(tmp == 0xff)
        {
            f.seekrel(50 - i);
            break;
        }
        addOrder(tmp);
    }
    f.read(reinterpret_cast<HscNote*>(m_patterns), 50 * 64 * 9);

    rewind(0); // rewind module
    return true;
}

bool HscPlayer::update()
{
    m_del--; // player speed handling
    if(m_del > 0)
        return !m_songend; // nothing done

    if(m_fadein) // fade-in handling
        m_fadein--;

    auto pattnr = currentPattern();
    if(pattnr == 0xff)
    { // arrangement handling
        m_songend = true; // set end-flag
        setCurrentOrder(0);
        pattnr = currentPattern();
    }
    else if((pattnr & 128) && (pattnr <= 0xb1))
    { // goto pattern "nr"
        setCurrentOrder(currentPattern() & 0x7f);
        setCurrentRow(0);
        pattnr = currentPattern();
        m_songend = true;
    }

    auto pattoff = currentRow() * 9;
    for(int chan = 0; chan < 9; chan++)
    { // handle all channels
        auto note = m_patterns[pattnr][pattoff].note;
        const auto effect = m_patterns[pattnr][pattoff].effect;
        pattoff++;

        if(note & 128)
        { // set instrument
            setinstr(chan, effect);
            continue;
        }
        const auto eff_op = effect & 0x0f;
        const auto inst = m_channel[chan].inst;
        if(note)
            m_channel[chan].slide = 0;

        switch(effect & 0xf0)
        { // effect handling
            case 0: // global effect
                /* The following fx are unimplemented on purpose:
           * 02 - Slide Mainvolume up
           * 03 - Slide Mainvolume down (here: fade in)
           * 04 - Set Mainvolume to 0
           *
           * This is because i've never seen any HSC modules using the fx this
           * way.
           * All modules use the fx the way, i've implemented it.
           */
                switch(eff_op)
                {
                    case 1:
                        m_pattbreak++;
                        break; // jump to next pattern
                    case 3:
                        m_fadein = 31;
                        break; // fade in (divided by 2)
                    case 5:
                        m_mode6 = 1;
                        break; // 6 voice mode on
                    case 6:
                        m_mode6 = 0;
                        break; // 6 voice mode off
                }
                break;
            case 0x20:
            case 0x10: // manual slides
                if(effect & 0x10)
                {
                    m_channel[chan].freq += eff_op;
                    m_channel[chan].slide += eff_op;
                }
                else
                {
                    m_channel[chan].freq -= eff_op;
                    m_channel[chan].slide -= eff_op;
                }
                if(!note)
                    setfreq(chan, m_channel[chan].freq);
                break;
            case 0x50: // set percussion instrument (unimplemented)
                break;
            case 0x60: // set feedback
                getOpl()->writeReg(
                    0xc0 + chan, (m_instr[m_channel[chan].inst][8] & 1) + (eff_op << 1));
                break;
            case 0xa0: // set carrier volume
            {
                auto vol = eff_op << 2;
                getOpl()->writeReg(0x43 + s_opTable[chan], vol | (m_instr[m_channel[chan].inst][2] & ~63));
            }
            break;
            case 0xb0: // set modulator volume
            {
                auto vol = eff_op << 2;
                if(m_instr[inst][8] & 1)
                    getOpl()->writeReg(0x40 + s_opTable[chan], vol | (m_instr[m_channel[chan].inst][3] & ~63));
                else
                    getOpl()->writeReg(0x40 + s_opTable[chan], vol | (m_instr[inst][3] & ~63));
            }
            break;
            case 0xc0: // set instrument volume
            {
                auto db = eff_op << 2;
                getOpl()->writeReg(0x43 + s_opTable[chan], db | (m_instr[m_channel[chan].inst][2] & ~63));
                if(m_instr[inst][8] & 1)
                    getOpl()->writeReg(0x40 + s_opTable[chan], db | (m_instr[m_channel[chan].inst][3] & ~63));
            }
            break;
            case 0xd0:
                m_pattbreak++;
                setCurrentOrder(eff_op);
                m_songend = true;
                break; // position jump
            case 0xf0: // set speed
                setCurrentSpeed(eff_op + 1);
                m_del = eff_op + 1;
                break;
        }

        if(m_fadein) // fade-in volume setting
            setvolume(chan, m_fadein * 2, m_fadein * 2);

        if(!note) // note handling
            continue;
        note--;

        if((note == 0x7f - 1) || ((note / 12) & ~7))
        { // pause (7fh)
            m_adlFreq[chan] &= ~32;
            getOpl()->writeReg(0xb0 + chan, m_adlFreq[chan]);
            continue;
        }

        // play the note
        if(m_mtkmode) // imitate MPU-401 Trakker bug
            note--;
        const auto Okt = ((note / 12) & 7) << 2;
        const auto Fnr = s_noteTable[(note % 12)] + m_instr[inst][11] + m_channel[chan].slide;
        m_channel[chan].freq = Fnr;
        if(!m_mode6 || chan < 6)
            m_adlFreq[chan] = Okt | 32;
        else
            m_adlFreq[chan] = Okt; // never set key for drums
        getOpl()->writeReg(0xb0 + chan, 0);
        setfreq(chan, Fnr);
        if(m_mode6)
        {
            switch(chan)
            { // play drums
                case 6:
                    getOpl()->writeReg(0xbd, m_bd & ~16);
                    m_bd |= 48;
                    break; // bass drum
                case 7:
                    getOpl()->writeReg(0xbd, m_bd & ~1);
                    m_bd |= 33;
                    break; // hihat
                case 8:
                    getOpl()->writeReg(0xbd, m_bd & ~2);
                    m_bd |= 34;
                    break; // cymbal
            }
            getOpl()->writeReg(0xbd, m_bd);
        }
    }

    m_del = currentSpeed(); // player speed-timing
    if(m_pattbreak)
    { // do post-effect handling
        setCurrentRow(0); // pattern break!
        m_pattbreak = 0;
        setCurrentOrder((currentOrder() + 1) % 50);
        if(currentOrder() == 0)
            m_songend = true;
    }
    else
    {
        setCurrentRow((currentRow() + 1) & 0x3f);
        if(currentRow() == 0)
        {
            setCurrentOrder((currentOrder() + 1) % 50);
            if(currentOrder() == 0)
                m_songend = true;
        }
    }
    return !m_songend; // still playing
}

void HscPlayer::rewind(const boost::optional<size_t>&)
{
    // rewind HSC player
    setCurrentRow(0);
    setCurrentOrder(0);
    m_pattbreak = 0;
    setCurrentSpeed(2);
    m_del = 1;
    m_songend = false;
    m_mode6 = 0;
    m_bd = 0;
    m_fadein = 0;

    getOpl()->writeReg(1, 32);
    getOpl()->writeReg(8, 128);
    getOpl()->writeReg(0xbd, 0);

    for(uint8_t i = 0; i < 9; i++)
        setinstr(i, i); // init channels
}

size_t HscPlayer::instrumentCount() const
{
    size_t instnum = 0;
    // count instruments
    for(int instcnt = 0; instcnt < 128; instcnt++)
    {
        for(int i = 0; i < 12; i++)
            if(m_instr[instcnt][i])
                ++instnum;
    }

    return instnum;
}

/*** private methods *************************************/

void HscPlayer::setfreq(uint8_t chan, unsigned short freq)
{
    m_adlFreq[chan] = (m_adlFreq[chan] & ~3) | (freq >> 8);

    getOpl()->writeReg(0xa0 + chan, freq & 0xff);
    getOpl()->writeReg(0xb0 + chan, m_adlFreq[chan]);
}

void HscPlayer::setvolume(uint8_t chan, int volc, int volm)
{
    const uint8_t* ins = m_instr[m_channel[chan].inst];
    char op = s_opTable[chan];

    getOpl()->writeReg(0x43 + op, volc | (ins[2] & ~63));
    if(ins[8] & 1) // carrier
        getOpl()->writeReg(0x40 + op, volm | (ins[3] & ~63));
    else
        getOpl()->writeReg(0x40 + op, ins[3]); // modulator
}

void HscPlayer::setinstr(uint8_t chan, uint8_t insnr)
{
    const uint8_t* ins = m_instr[insnr];
    uint8_t op = s_opTable[chan];

    m_channel[chan].inst = insnr; // set internal instrument
    getOpl()->writeReg(0xb0 + chan, 0); // stop old note

    // set instrument
    getOpl()->writeReg(0xc0 + chan, ins[8]);
    getOpl()->writeReg(0x23 + op, ins[0]); // carrier
    getOpl()->writeReg(0x20 + op, ins[1]); // modulator
    getOpl()->writeReg(0x63 + op, ins[4]); // bits 0..3 = decay; 4..7 = attack
    getOpl()->writeReg(0x60 + op, ins[5]);
    getOpl()->writeReg(0x83 + op, ins[6]); // 0..3 = release; 4..7 = sustain
    getOpl()->writeReg(0x80 + op, ins[7]);
    getOpl()->writeReg(0xe3 + op, ins[9]); // bits 0..1 = Wellenform
    getOpl()->writeReg(0xe0 + op, ins[10]);
    setvolume(chan, ins[2] & 63, ins[3] & 63);
}