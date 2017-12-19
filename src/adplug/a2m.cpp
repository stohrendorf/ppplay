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
 * a2m.cpp - A2M Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * This loader detects and loads version 1, 4, 5 & 8 files.
 *
 * version 1-4 files:
 * Following commands are ignored: FF1 - FF9, FAx - FEx
 *
 * version 5-8 files:
 * Instrument panning is ignored. Flags byte is ignored.
 * Following commands are ignored: Gxy, Hxy, Kxy - &xy
 */

#include "stream/filestream.h"
#include "a2m.h"

namespace
{
const std::array<int16_t, 6> copybits = {{4, 6, 8, 10, 12, 14}};

const std::array<int16_t, 6> copymin = {{0, 16, 80, 336, 1360, 5456}};
}

Player* A2mPlayer::factory()
{
    return new A2mPlayer();
}

bool A2mPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }
    const Command convfx[16] = {
        Command::None,
        Command::SlideUp,
        Command::SlideDown,
        Command::FineSlideUp,
        Command::FineSlideDown,
        Command::Porta,
        Command::PortaVolSlide,
        Command::Vibrato,
        Command::VibVolSlide,
        Command::SetVolume,
        Command::SetFineVolume2,
        Command::PatternBreak,
        Command::OrderJump,
        Command::RADSpeed,
        Command::SetTempo,
        Command::Special
    };
    const Command convinf1[16] = {
        Command::SFXTremolo,
        Command::SFXVibrato,
        Command::SFXWaveForm,
        Command::SFXSlideUp,
        Command::SFXSlideDown,
        Command::SFXVolumeUp,
        Command::SFXVolumeDown,
        Command::SFXFineVolumeUp,
        Command::SFXFineVolumeDown,
        Command::SFXRetrigger,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::SFXKeyOff
    };
    const Command newconvfx[] = {
        Command::None,
        Command::SlideUp,
        Command::SlideDown,
        Command::Porta,
        Command::Vibrato,
        Command::PortaVolSlide,
        Command::VibVolSlide,
        Command::FineSlideUp,
        Command::FineSlideDown,
        Command::ModulatorVolume,
        Command::SA2VolSlide,
        Command::OrderJump,
        Command::SetFineVolume2,
        Command::PatternBreak,
        Command::SetTempo,
        Command::RADSpeed,
        Command::Sentinel,
        Command::Sentinel,
        Command::CarrierVolume,
        Command::WaveForm,
        Command::Sentinel,
        Command::SA2Speed,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Sentinel,
        Command::Special,
        Command::Sentinel
    };

    // read header
    char id[10];
    f.read(id, 10);
    f.seekrel(4);
    uint8_t version;
    f >> version;
    uint8_t numpats;
    f >> numpats;

    // file validation section
    if( strncmp(id, "_A2module_", 10) != 0 || (version != 1 && version != 5 && version != 4 && version != 8) )
    {
        return false;
    }

    // load, depack & convert section
    //m_maxUsedPattern = numpats;
    uint16_t len[9];
    int t;
    if( version < 5 )
    {
        f.read(len, 5);
        t = 9;
    }
    else
    { // version >= 5
        f.read(len, 9);
        t = 18;
    }

    // block 0
    std::vector<uint16_t> m_secData;
    m_secData.resize(len[0] / 2u);
    std::vector<uint8_t> org;
    size_t orgPos = 0;
    if( version == 1 || version == 5 )
    {
        f.read(m_secData.data(), len[0] / 2u);
        org.resize(MAXBUF);
        sixDepak(m_secData.data(), org.data(), len[0]);
    }
    else
    {
        f.read(org.data(), len[0]);
    }
    m_songname.assign(&org[orgPos], &org[orgPos] + 42);
    orgPos += 43;
    m_author.assign(&org[orgPos], &org[orgPos] + 42);
    orgPos += 43;
    for( int i = 0; i < 250; ++i )
    {
        // m_songname.assign(&org[orgPos], &org[orgPos] + 32);
        orgPos += 33;
    }

    for( int i = 0; i < 250; i++ )
    { // instruments
        ModPlayer::Instrument& inst = addInstrument();
        inst.data[0] = org[orgPos + 10];
        inst.data[1] = org[orgPos + 0];
        inst.data[2] = org[orgPos + 1];
        inst.data[3] = org[orgPos + 4];
        inst.data[4] = org[orgPos + 5];
        inst.data[5] = org[orgPos + 6];
        inst.data[6] = org[orgPos + 7];
        inst.data[7] = org[orgPos + 8];
        inst.data[8] = org[orgPos + 9];
        inst.data[9] = org[orgPos + 2];
        inst.data[10] = org[orgPos + 3];

        if( version < 5 )
        {
            inst.misc = org[orgPos + 11];
        }
        else
        { // version >= 5 -> OPL3 format
            int pan = org[orgPos + 11];

            if( pan )
            {
                inst.data[0] |= (pan & 3) << 4; // set pan
            }
            else
            {
                inst.data[0] |= 48;
            } // enable both speakers
        }

        inst.slide = org[orgPos + 12];
        orgPos += 13;
    }

    for( int i = 0; i < 128; ++i )
    {
        addOrder(org[orgPos++]);
    }
    setRestartOrder(0);
    setInitialTempo(org[orgPos++]);
    setInitialSpeed(org[orgPos++]);
    uint8_t flags = 0;
    if( version >= 5 )
    {
        flags = org[orgPos];
    }
    if( version == 1 || version == 5 )
    {
        org.clear();
    }
    m_secData.clear();

    // blocks 1-4 or 1-8
    size_t alength = len[1];
    for( int i = 0; i < (version < 5 ? numpats / 16 : numpats / 8); i++ )
    {
        alength += len[i + 2];
    }

    m_secData.resize(alength / 2);
    if( version == 1 || version == 5 )
    {
        f.read(m_secData.data(), alength / 2);
        org.resize(MAXBUF * (numpats / (version == 1 ? 16 : 8) + 1));
        orgPos = 0;
        size_t secPos = 0;
        orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[1]);
        secPos += len[1] / 2;
        if( version == 1 )
        {
            if( numpats > 16 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[2]);
            }
            secPos += len[2] / 2;
            if( numpats > 32 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[3]);
            }
            secPos += len[3] / 2;
            if( numpats > 48 )
            {
                sixDepak(&m_secData[secPos], &org[orgPos], len[4]);
            }
        }
        else
        {
            if( numpats > 8 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[2]);
            }
            secPos += len[2] / 2;
            if( numpats > 16 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[3]);
            }
            secPos += len[3] / 2;
            if( numpats > 24 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[4]);
            }
            secPos += len[4] / 2;
            if( numpats > 32 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[5]);
            }
            secPos += len[5] / 2;
            if( numpats > 40 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[6]);
            }
            secPos += len[6] / 2;
            if( numpats > 48 )
            {
                orgPos += sixDepak(&m_secData[secPos], &org[orgPos], len[7]);
            }
            secPos += len[7] / 2;
            if( numpats > 56 )
            {
                sixDepak(&m_secData[secPos], &org[orgPos], len[8]);
            }
        }
        m_secData.clear();
    }
    else
    {
        org.assign(
            reinterpret_cast<const uint8_t*>(m_secData.data()),
            reinterpret_cast<const uint8_t*>(m_secData.data() + m_secData.size()));
        f.read(org.data(), alength);
    }

    if( version < 5 )
    {
        for( auto i = 0u; i < numpats; i++ )
        {
            for( auto j = 0u; j < 64; j++ )
            {
                for( auto k = 0u; k < 9; k++ )
                {
                    PatternCell& cell = patternCell(i * 9 + k, j);
                    unsigned char* o = &org[i * 64 * t * 4 + j * t * 4 + k * 4];

                    cell.note = o[0] == 255 ? 127 : o[0];
                    cell.instrument = o[1];
                    cell.command = convfx[o[2]];
                    cell.loNybble = o[3] & 0x0f;
                    if( cell.command != Command::Special )
                    {
                        cell.hiNybble = o[3] >> 4;
                    }
                    else
                    {
                        cell.command = convinf1[o[3] >> 4];
                        if( cell.command == Command::SFXKeyOff && !cell.loNybble )
                        { // convert key-off
                            cell.command = Command::NoteOff;
                            cell.hiNybble = 0;
                            cell.loNybble = 0;
                        }
                    }
                    switch( cell.command )
                    {
                        case Command::SFXWaveForm: // convert define waveform
                            cell.command = Command::WaveForm;
                            cell.hiNybble = cell.loNybble;
                            cell.loNybble = 0xf;
                            break;
                        case Command::SFXVolumeUp: // convert volume slide up
                            cell.command = Command::VolSlide;
                            cell.hiNybble = cell.loNybble;
                            cell.loNybble = 0;
                            break;
                        case Command::SFXVolumeDown: // convert volume slide down
                            cell.command = Command::VolSlide;
                            cell.hiNybble = 0;
                            break;
                    }
                }
            }
        }
    }
    else
    { // version >= 5
        realloc_patterns(64, 64, 18);

        for( auto i = 0u; i < numpats; i++ )
        {
            for( auto j = 0u; j < 18; j++ )
            {
                for( auto k = 0u; k < 64; k++ )
                {
                    PatternCell& cell = patternCell(i * 18 + j, k);
                    unsigned char* o = &org[i * 64 * t * 4 + j * 64 * 4 + k * 4];

                    cell.note = o[0] == 255 ? 127 : o[0];
                    cell.instrument = o[1];
                    cell.command = newconvfx[o[2]];
                    cell.hiNybble = o[3] >> 4;
                    cell.loNybble = o[3] & 0x0f;

                    // Convert '&' command
                    if( o[2] == 36 )
                    {
                        switch( cell.hiNybble )
                        {
                            case 0: // pattern delay (frames)
                                cell.command = Command::PatternDelay;
                                cell.hiNybble = 0;
                                // param2 already set correctly
                                break;

                            case 1: // pattern delay (rows)
                                cell.command = Command::SFXPatternDelay;
                                cell.hiNybble = 0;
                                // param2 already set correctly
                                break;
                        }
                    }
                }
            }
        }
    }

    init_trackord();

    if( version == 1 || version == 5 )
    {
        org.clear();
    }
    else
    {
        m_secData.clear();
    }

    // Process flags
    if( version >= 5 )
    {
        setOpl3Mode(); // All versions >= 5 are OPL3
        if( flags & 8 )
        {
            setTremolo();
        } // Tremolo depth
        if( flags & 16 )
        {
            setVibrato();
        } // Vibrato depth
    }

    rewind(size_t(0));
    return true;
}

size_t A2mPlayer::framesUntilUpdate() const
{
    if( currentTempo() != 18 )
    {
        return SampleRate / currentTempo();
    }
    else
    {
        return static_cast<size_t>(SampleRate / 18.2);
    }
}

/*** private methods *************************************/

void A2mPlayer::initTree()
{
    for( int i = 2; i <= TWICEMAX; i++ )
    {
        m_dad[i] = i / 2;
        m_freq[i] = 1;
    }

    for( int i = 1; i <= MAXCHAR; i++ )
    {
        m_leftc[i] = 2 * i;
        m_rightc[i] = 2 * i + 1;
    }
}

void A2mPlayer::updateFreq(uint16_t a, uint16_t b)
{
    do
    {
        m_freq[m_dad[a]] = m_freq[a] + m_freq[b];
        a = m_dad[a];
        if( a != ROOT )
        {
            if( m_leftc[m_dad[a]] == a )
            {
                b = m_rightc[m_dad[a]];
            }
            else
            {
                b = m_leftc[m_dad[a]];
            }
        }
    } while( a != ROOT );

    if( m_freq[ROOT] == MAXFREQ )
    {
        for( a = 1; a <= TWICEMAX; a++ )
        {
            m_freq[a] >>= 1;
        }
    }
}

void A2mPlayer::updateModel(uint16_t code)
{
    uint16_t a = code + SUCCMAX;

    m_freq[a]++;
    if( m_dad[a] != ROOT )
    {
        auto code1 = m_dad[a];
        if( m_leftc[code1] == a )
        {
            updateFreq(a, m_rightc[code1]);
        }
        else
        {
            updateFreq(a, m_leftc[code1]);
        }

        do
        {
            const auto code2 = m_dad[code1];
            uint16_t b;
            if( m_leftc[code2] == code1 )
            {
                b = m_rightc[code2];
            }
            else
            {
                b = m_leftc[code2];
            }

            if( m_freq[a] > m_freq[b] )
            {
                if( m_leftc[code2] == code1 )
                {
                    m_rightc[code2] = a;
                }
                else
                {
                    m_leftc[code2] = a;
                }

                uint16_t c;
                if( m_leftc[code1] == a )
                {
                    m_leftc[code1] = b;
                    c = m_rightc[code1];
                }
                else
                {
                    m_rightc[code1] = b;
                    c = m_leftc[code1];
                }

                m_dad[b] = code1;
                m_dad[a] = code2;
                updateFreq(b, c);
                a = b;
            }

            a = m_dad[a];
            code1 = m_dad[a];
        } while( code1 != ROOT );
    }
}

uint16_t A2mPlayer::inputCode(uint16_t bits)
{
    uint16_t code = 0;

    for( int i = 1; i <= bits; i++ )
    {
        if( !m_bitcount )
        {
            if( m_bitcount == MAXBUF )
            {
                m_bufcount = 0;
            }
            m_bitbuffer = m_wdbuf[m_bufcount];
            m_bufcount++;
            m_bitcount = 15;
        }
        else
        {
            m_bitcount--;
        }

        if( m_bitbuffer > 0x7fff )
        {
            code |= 1u << (i - 1);
        }
        m_bitbuffer <<= 1;
    }

    return code;
}

uint16_t A2mPlayer::uncompress()
{
    uint16_t a = 1;

    do
    {
        if( !m_bitcount )
        {
            if( m_bufcount == MAXBUF )
            {
                m_bufcount = 0;
            }
            m_bitbuffer = m_wdbuf[m_bufcount];
            m_bufcount++;
            m_bitcount = 15;
        }
        else
        {
            m_bitcount--;
        }

        if( m_bitbuffer > 0x7fff )
        {
            a = m_rightc[a];
        }
        else
        {
            a = m_leftc[a];
        }
        m_bitbuffer <<= 1;
    } while( a <= MAXCHAR );

    a -= SUCCMAX;
    updateModel(a);
    return a;
}

void A2mPlayer::decode()
{
    uint16_t count = 0;

    initTree();

    for( auto c = uncompress(); c != TERMINATE; c = uncompress() )
    {
        if( c < 256 )
        {
            m_obuf[m_obufcount] = static_cast<uint8_t>(c);
            m_obufcount++;
            if( m_obufcount == MAXBUF )
            {
                m_outputSize = MAXBUF;
                m_obufcount = 0;
            }

            m_buf[count] = static_cast<uint8_t>(c);
            count++;
            if( count == MAXSIZE )
            {
                count = 0;
            }
        }
        else
        {
            const auto t = c - FIRSTCODE;
            const auto index = t / CODESPERRANGE;
            const auto len = t + MINCOPY - index * CODESPERRANGE;
            const auto dist = inputCode(copybits[index]) + len + copymin[index];

            auto j = count;
            auto k = count - dist;
            if( count < dist )
            {
                k += MAXSIZE;
            }

            for( auto i = 0; i <= len - 1; i++ )
            {
                m_obuf[m_obufcount] = m_buf[k];
                m_obufcount++;
                if( m_obufcount == MAXBUF )
                {
                    m_outputSize = MAXBUF;
                    m_obufcount = 0;
                }

                m_buf[j] = m_buf[k];
                j++;
                k++;
                if( j == MAXSIZE )
                {
                    j = 0;
                }
                if( k == MAXSIZE )
                {
                    k = 0;
                }
            }

            count += len;
            if( count >= MAXSIZE )
            {
                count -= MAXSIZE;
            }
        }
    }
    m_outputSize = m_obufcount;
}

size_t A2mPlayer::sixDepak(uint16_t* source, uint8_t* dest, size_t size)
{
    if( size + 4096 > MAXBUF )
    {
        return 0;
    }

    m_buf.resize(MAXSIZE);
    m_bitcount = 0;
    m_bitbuffer = 0;
    m_obufcount = 0;
    m_bufcount = 0;
    m_wdbuf = source;
    m_obuf = dest;

    decode();
    m_buf.clear();
    return m_outputSize;
}
