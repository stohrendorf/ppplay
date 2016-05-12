/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * s3m.c - S3M Player by Simon Peter <dn.tlp@gmx.net>
 *
 * BUGS:
 * Extra Fine Slides (EEx, FEx) & Fine Vibrato (Uxy) are inaccurate
 */

#include "stream/filestream.h"

#include "s3m.h"

const char S3mPlayer::chnresolv[] = // S3M -> adlib channel conversion
{ -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7,
    8, -1, -1, -1, -1, -1, -1, -1 };

const unsigned short S3mPlayer::notetable[12] = // S3M adlib note table
{ 340, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647 };

const unsigned char S3mPlayer::vibratotab[32] = // vibrato rate table
{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 15, 14, 13, 12,
    11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

/*** public methods *************************************/

Player* S3mPlayer::factory()
{
    return new S3mPlayer();
}

S3mPlayer::S3mPlayer() : Player()
{
    memset(m_patterns, 255, sizeof(m_patterns));
    //memset(m_orders, 255, sizeof(m_orders));

    for(int i = 0; i < 99; i++) // setup pattern
        for(int j = 0; j < 64; j++)
            for(int k = 0; k < 32; k++)
            {
                m_patterns[i][j][k].instrument = 0;
                m_patterns[i][j][k].effectValue = 0;
            }
}

bool S3mPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if(!f)
        return false;

    // file validation section
    f >> m_header;
    bool adlibins = false;
    if(m_header.endOfFile != 0x1a || m_header.type != 16 || m_header.instrumentCount > 99)
    {
        return false;
    }
    else if(strncmp(m_header.scrm, "SCRM", 4))
    {
        return false;
    }
    else
    { // is an adlib module?
        f.seekrel(m_header.orderCount);
        uint16_t insptr[99];
        f.read(insptr, m_header.instrumentCount);
        for(int i = 0; i < m_header.instrumentCount; i++)
        {
            f.seek(insptr[i] * 16);
            uint8_t tmp;
            f >> tmp;
            if(tmp >= 2)
            {
                adlibins = true;
                break;
            }
        }
        if(!adlibins)
        {
            return false;
        }
    }

    // load section
    f.seek(sizeof(S3mHeader)); // rewind for load

    // security check
    if(m_header.orderCount > 256 || m_header.instrumentCount > 99 || m_header.patternCount > 99)
    {
        return false;
    }

    for(int i = 0; i < m_header.orderCount; ++i)
    {
        uint8_t tmp;
        f >> tmp;
        addOrder(tmp);
    }
    uint16_t insptr[99];
    f.read(insptr, m_header.instrumentCount);
    uint16_t pattptr[99];
    f.read(pattptr, m_header.patternCount);

    for(int i = 0; i < m_header.instrumentCount; i++)
    { // load instruments
        f.seek(insptr[i] * 16);
        f >> m_instruments[i];
    }

    for(int i = 0; i < m_header.patternCount; i++)
    { // depack patterns
        f.seek(pattptr[i] * 16);
        uint16_t ppatlen;
        f >> ppatlen;
        auto pattpos = f.pos();
        for(int row = 0; (row < 64) && (pattpos - pattptr[i] * 16 <= ppatlen); row++)
        {
            uint8_t bufval;
            do
            {
                f >> bufval;
                if(bufval & 32)
                {
                    uint8_t bufval2;
                    f >> bufval2;
                    m_patterns[i][row][bufval & 31].note = bufval2 & 15;
                    m_patterns[i][row][bufval & 31].octave = (bufval2 & 240) >> 4;
                    f >> m_patterns[i][row][bufval & 31].instrument;
                }
                if(bufval & 64)
                    f >> m_patterns[i][row][bufval & 31].volume;
                if(bufval & 128)
                {
                    f >> m_patterns[i][row][bufval & 31].effect;
                    f >> m_patterns[i][row][bufval & 31].effectValue;
                }
            } while(bufval);
        }
    }

    rewind(0);
    return true; // done
}

bool S3mPlayer::update()
{
    // effect handling (timer dependant)
    for(int8_t realchan = 0; realchan < 9; realchan++)
    {
        auto effectValue = m_channels[realchan].effectValue; // fill infobyte cache
        switch(m_channels[realchan].effect)
        {
            case 11:
            case 12:
                if(m_channels[realchan].effect == 11) // dual command: H00 and Dxy
                    vibrato(realchan, m_channels[realchan].dualInfo);
                else // dual command: G00 and Dxy
                    tone_portamento(realchan, m_channels[realchan].dualInfo);
            case 4:
                if(effectValue <= 0x0f)
                { // volume slide down
                    if(m_channels[realchan].volume - effectValue >= 0)
                        m_channels[realchan].volume -= effectValue;
                    else
                        m_channels[realchan].volume = 0;
                }
                if((effectValue & 0x0f) == 0)
                { // volume slide up
                    if(m_channels[realchan].volume + (effectValue >> 4) <= 63)
                        m_channels[realchan].volume += effectValue >> 4;
                    else
                        m_channels[realchan].volume = 63;
                }
                setvolume(realchan);
                break;
            case 5:
                if(effectValue == 0xf0 || effectValue <= 0xe0)
                { // slide down
                    slide_down(realchan, effectValue);
                    setfreq(realchan);
                }
                break;
            case 6:
                if(effectValue == 0xf0 || effectValue <= 0xe0)
                { // slide up
                    slide_up(realchan, effectValue);
                    setfreq(realchan);
                }
                break;
            case 7:
                tone_portamento(realchan, m_channels[realchan].dualInfo);
                break; // tone portamento
            case 8:
                vibrato(realchan, m_channels[realchan].dualInfo);
                break; // vibrato
            case 10:
                m_channels[realchan].nextFrequency = m_channels[realchan].frequency; // arpeggio
                m_channels[realchan].nextOctave = m_channels[realchan].octave;
                switch(m_channels[realchan].trigger)
                {
                    case 0:
                        m_channels[realchan].frequency = notetable[m_channels[realchan].note];
                        break;
                    case 1:
                        if(m_channels[realchan].note + ((effectValue & 0xf0) >> 4) < 12)
                            m_channels[realchan].frequency =
                            notetable[m_channels[realchan].note + ((effectValue & 0xf0) >> 4)];
                        else
                        {
                            m_channels[realchan].frequency =
                                notetable[m_channels[realchan].note + ((effectValue & 0xf0) >> 4) - 12];
                            m_channels[realchan].octave++;
                        }
                        break;
                    case 2:
                        if(m_channels[realchan].note + (effectValue & 0x0f) < 12)
                            m_channels[realchan].frequency =
                            notetable[m_channels[realchan].note + (effectValue & 0x0f)];
                        else
                        {
                            m_channels[realchan].frequency =
                                notetable[m_channels[realchan].note + (effectValue & 0x0f) - 12];
                            m_channels[realchan].octave++;
                        }
                        break;
                }
                if(m_channels[realchan].trigger < 2)
                    m_channels[realchan].trigger++;
                else
                    m_channels[realchan].trigger = 0;
                setfreq(realchan);
                m_channels[realchan].frequency = m_channels[realchan].nextFrequency;
                m_channels[realchan].octave = m_channels[realchan].nextOctave;
                break;
            case 21:
                vibrato(realchan, effectValue / 4);
                break; // fine vibrato
        }
    }

    if(m_patternDelay)
    { // speed compensation
        m_patternDelay--;
        return !songend;
    }

    // arrangement handling
    auto pattnr = currentPattern();
    if(pattnr == 0xff || currentOrder() > orderCount())
    { // "--" end of song
        songend = 1; // set end-flag
        setCurrentOrder(0);
        pattnr = currentPattern();
        if(pattnr == 0xff)
            return !songend;
    }
    if(pattnr == 0xfe)
    { // "++" skip marker
        setCurrentOrder(currentOrder() + 1);
        pattnr = currentPattern();
    }

    // play row
    auto row = currentRow(); // fill row cache
    bool pattbreak = false;
    for(int chan = 0; chan < 32; chan++)
    {
        if(m_header.chanset[chan] & 128) // resolve S3M -> AdLib channels
            continue;
        const int realchan = chnresolv[m_header.chanset[chan] & 127];
        // set channel values
        bool donote = false;
        if(m_patterns[pattnr][row][chan].note < 14)
        {
            // tone portamento
            if(m_patterns[pattnr][row][chan].effect == 7 ||
               m_patterns[pattnr][row][chan].effect == 12)
            {
                m_channels[realchan].nextFrequency =
                    notetable[m_patterns[pattnr][row][chan].note];
                m_channels[realchan].nextOctave = m_patterns[pattnr][row][chan].octave;
            }
            else
            { // normal note
                m_channels[realchan].note = m_patterns[pattnr][row][chan].note;
                m_channels[realchan].frequency = notetable[m_patterns[pattnr][row][chan].note];
                m_channels[realchan].octave = m_patterns[pattnr][row][chan].octave;
                m_channels[realchan].key = 1;
                donote = true;
            }
        }
        if(m_patterns[pattnr][row][chan].note ==
           14)
        { // key off (is 14 here, cause note is only first 4 bits)
            m_channels[realchan].key = 0;
            setfreq(realchan);
        }
        if((m_channels[realchan].effect != 8 &&
            m_channels[realchan].effect != 11) && // vibrato begins
            (m_patterns[pattnr][row][chan].effect == 8 ||
             m_patterns[pattnr][row][chan].effect == 11))
        {
            m_channels[realchan].nextFrequency = m_channels[realchan].frequency;
            m_channels[realchan].nextOctave = m_channels[realchan].octave;
        }
        if(m_patterns[pattnr][row][chan].note >= 14)
            if((m_channels[realchan].effect == 8 ||
                m_channels[realchan].effect == 11) && // vibrato ends
                (m_patterns[pattnr][row][chan].effect != 8 &&
                 m_patterns[pattnr][row][chan].effect != 11))
            {
                m_channels[realchan].frequency = m_channels[realchan].nextFrequency;
                m_channels[realchan].octave = m_channels[realchan].nextOctave;
                setfreq(realchan);
            }
        if(m_patterns[pattnr][row][chan].instrument)
        { // set instrument
            m_channels[realchan].instrument = m_patterns[pattnr][row][chan].instrument - 1;
            if(m_instruments[m_channels[realchan].instrument].volume < 64)
                m_channels[realchan].volume = m_instruments[m_channels[realchan].instrument].volume;
            else
                m_channels[realchan].volume = 63;
            if(m_patterns[pattnr][row][chan].effect != 7)
                donote = true;
        }
        if(m_patterns[pattnr][row][chan].volume != 255)
        {
            if(m_patterns[pattnr][row][chan].volume < 64) // set volume
                m_channels[realchan].volume = m_patterns[pattnr][row][chan].volume;
            else
                m_channels[realchan].volume = 63;
        }
        m_channels[realchan].effect = m_patterns[pattnr][row][chan].effect; // set command
        if(m_patterns[pattnr][row][chan].effectValue) // set infobyte
            m_channels[realchan].effectValue = m_patterns[pattnr][row][chan].effectValue;

        // some commands reset the infobyte memory
        switch(m_channels[realchan].effect)
        {
            case 1:
            case 2:
            case 3:
            case 20:
                m_channels[realchan].effectValue = m_patterns[pattnr][row][chan].effectValue;
                break;
        }

        // play note
        if(donote)
            playnote(realchan);
        if(m_patterns[pattnr][row][chan].volume != 255) // set volume
            setvolume(realchan);

        // command handling (row dependant)
        auto info = m_channels[realchan].effectValue; // fill infobyte cache
        switch(m_channels[realchan].effect)
        {
            case 1:
                setCurrentSpeed(info);
                break; // set speed
            case 2:
                if(info <= currentOrder())
                    songend = 1;
                setCurrentOrder(info);
                setCurrentRow(0);
                pattbreak = true;
                break; // jump to order
            case 3:
                if(!pattbreak)
                {
                    setCurrentRow(info);
                    setCurrentOrder(currentOrder() + 1);
                    pattbreak = true;
                }
                break; // pattern break
            case 4:
                if(info > 0xf0)
                { // fine volume down
                    if(m_channels[realchan].volume - (info & 0x0f) >= 0)
                        m_channels[realchan].volume -= info & 0x0f;
                    else
                        m_channels[realchan].volume = 0;
                }
                if((info & 0x0f) == 0x0f && info >= 0x1f)
                { // fine volume up
                    if(m_channels[realchan].volume + ((info & 0xf0) >> 4) <= 63)
                        m_channels[realchan].volume += (info & 0xf0) >> 4;
                    else
                        m_channels[realchan].volume = 63;
                }
                setvolume(realchan);
                break;
            case 5:
                if(info > 0xf0)
                { // fine slide down
                    slide_down(realchan, info & 0x0f);
                    setfreq(realchan);
                }
                if(info > 0xe0 && info < 0xf0)
                { // extra fine slide down
                    slide_down(realchan, (info & 0x0f) / 4);
                    setfreq(realchan);
                }
                break;
            case 6:
                if(info > 0xf0)
                { // fine slide up
                    slide_up(realchan, info & 0x0f);
                    setfreq(realchan);
                }
                if(info > 0xe0 && info < 0xf0)
                { // extra fine slide up
                    slide_up(realchan, (info & 0x0f) / 4);
                    setfreq(realchan);
                }
                break;
            case 7: // tone portamento
            case 8:
                if((m_channels[realchan].effect ==
                    7 || // vibrato (remember info for dual commands)
                    m_channels[realchan].effect == 8) && m_patterns[pattnr][row][chan].effectValue)
                    m_channels[realchan].dualInfo = info;
                break;
            case 10:
                m_channels[realchan].trigger = 0;
                break; // arpeggio (set trigger)
            case 19:
                if(info == 0xb0) // set loop start
                    m_loopStart = row;
                if(info > 0xb0 && info <= 0xbf)
                { // pattern loop
                    if(!m_loopCounter)
                    {
                        m_loopCounter = info & 0x0f;
                        setCurrentRow(m_loopStart);
                        pattbreak = true;
                    }
                    else if(--m_loopCounter > 0)
                    {
                        setCurrentRow(m_loopStart);
                        pattbreak = true;
                    }
                }
                if((info & 0xf0) == 0xe0) // patterndelay
                    m_patternDelay = currentSpeed() * (info & 0x0f) - 1;
                break;
            case 20:
                setCurrentTempo(info);
                break; // set tempo
        }
    }

    if(!m_patternDelay)
        m_patternDelay = currentSpeed() - 1; // speed compensation
    if(!pattbreak)
    { // next row (only if no manual advance)
        setCurrentRow((currentRow() + 1) & 0x3f);
        if(currentRow() == 0)
        {
            setCurrentOrder(currentOrder() + 1);
            m_loopStart = 0;
        }
    }

    return !songend; // still playing
}

void S3mPlayer::rewind(int)
{
    // set basic variables
    songend = 0;
    setCurrentOrder(0);
    setCurrentRow(0);
    setCurrentTempo(m_header.initialTempo);
    setCurrentSpeed(m_header.initialSpeed);
    m_patternDelay = 0;
    m_loopStart = 0;
    m_loopCounter = 0;

    memset(m_channels, 0, sizeof(m_channels));

    getOpl()->writeReg(1, 32); // Go to ym3812 mode
}

std::string S3mPlayer::type() const
{
    char filever[5];

    switch(m_header.trackerVersion)
    { // determine version number
        case 0x1300:
            strcpy(filever, "3.00");
            break;
        case 0x1301:
            strcpy(filever, "3.01");
            break;
        case 0x1303:
            strcpy(filever, "3.03");
            break;
        case 0x1320:
            strcpy(filever, "3.20");
            break;
        default:
            strcpy(filever, "3.??");
    }

    return std::string("Scream Tracker ") + filever;
}

size_t S3mPlayer::framesUntilUpdate() const
{
    return static_cast<size_t>(SampleRate * 2.5 / currentTempo());
}

/*** private methods *************************************/

void S3mPlayer::setvolume(unsigned char chan)
{
    unsigned char op = s_opTable[chan], insnr = m_channels[chan].instrument;

    getOpl()->writeReg(
        0x43 + op, static_cast<uint8_t>(63 - ((63 - (m_instruments[insnr].d03 & 63)) / 63.0) *
                                        m_channels[chan].volume) + (m_instruments[insnr].d03 & 192));
    if(m_instruments[insnr].d0a & 1)
        getOpl()->writeReg(
            0x40 + op, static_cast<uint8_t>(63 - ((63 - (m_instruments[insnr].d02 & 63)) / 63.0) *
                                            m_channels[chan].volume) + (m_instruments[insnr].d02 & 192));
}

void S3mPlayer::setfreq(unsigned char chan)
{
    getOpl()->writeReg(0xa0 + chan, m_channels[chan].frequency & 255);
    if(m_channels[chan].key)
        getOpl()->writeReg(0xb0 + chan, (((m_channels[chan].frequency & 768) >> 8) +
        (m_channels[chan].octave << 2)) | 32);
    else
        getOpl()->writeReg(0xb0 + chan, ((m_channels[chan].frequency & 768) >> 8) +
        (m_channels[chan].octave << 2));
}

void S3mPlayer::playnote(unsigned char chan)
{
    unsigned char op = s_opTable[chan], insnr = m_channels[chan].instrument;

    getOpl()->writeReg(0xb0 + chan, 0); // stop old note

    // set instrument data
    getOpl()->writeReg(0x20 + op, m_instruments[insnr].d00);
    getOpl()->writeReg(0x23 + op, m_instruments[insnr].d01);
    getOpl()->writeReg(0x40 + op, m_instruments[insnr].d02);
    getOpl()->writeReg(0x43 + op, m_instruments[insnr].d03);
    getOpl()->writeReg(0x60 + op, m_instruments[insnr].d04);
    getOpl()->writeReg(0x63 + op, m_instruments[insnr].d05);
    getOpl()->writeReg(0x80 + op, m_instruments[insnr].d06);
    getOpl()->writeReg(0x83 + op, m_instruments[insnr].d07);
    getOpl()->writeReg(0xe0 + op, m_instruments[insnr].d08);
    getOpl()->writeReg(0xe3 + op, m_instruments[insnr].d09);
    getOpl()->writeReg(0xc0 + chan, m_instruments[insnr].d0a);

    // set frequency & play
    m_channels[chan].key = 1;
    setfreq(chan);
}

void S3mPlayer::slide_down(unsigned char chan, unsigned char amount)
{
    if(m_channels[chan].frequency - amount > 340)
        m_channels[chan].frequency -= amount;
    else if(m_channels[chan].octave > 0)
    {
        m_channels[chan].octave--;
        m_channels[chan].frequency = 684;
    }
    else
        m_channels[chan].frequency = 340;
}

void S3mPlayer::slide_up(unsigned char chan, unsigned char amount)
{
    if(m_channels[chan].frequency + amount < 686)
        m_channels[chan].frequency += amount;
    else if(m_channels[chan].octave < 7)
    {
        m_channels[chan].octave++;
        m_channels[chan].frequency = 341;
    }
    else
        m_channels[chan].frequency = 686;
}

void S3mPlayer::vibrato(unsigned char chan, unsigned char info)
{
    unsigned char i, speed, depth;

    speed = info >> 4;
    depth = (info & 0x0f) / 2;

    for(i = 0; i < speed; i++)
    {
        m_channels[chan].trigger++;
        while(m_channels[chan].trigger >= 64)
            m_channels[chan].trigger -= 64;
        if(m_channels[chan].trigger >= 16 && m_channels[chan].trigger < 48)
            slide_down(chan, static_cast<uint8_t>(vibratotab[m_channels[chan].trigger - 16] /
            (16 - depth)));
        if(m_channels[chan].trigger < 16)
            slide_up(chan, static_cast<uint8_t>(vibratotab[m_channels[chan].trigger + 16] /
            (16 - depth)));
        if(m_channels[chan].trigger >= 48)
            slide_up(chan, static_cast<uint8_t>(vibratotab[m_channels[chan].trigger - 48] /
            (16 - depth)));
    }
    setfreq(chan);
}

void S3mPlayer::tone_portamento(unsigned char chan, unsigned char info)
{
    if(m_channels[chan].frequency + (m_channels[chan].octave << 10) <
       m_channels[chan].nextFrequency + (m_channels[chan].nextOctave << 10))
        slide_up(chan, info);
    if(m_channels[chan].frequency + (m_channels[chan].octave << 10) >
       m_channels[chan].nextFrequency + (m_channels[chan].nextOctave << 10))
        slide_down(chan, info);
    setfreq(chan);
}