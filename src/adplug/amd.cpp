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
 * amd.cpp - AMD Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"

#include "amd.h"
#include <stuff/stringutils.h>

Player* AmdPlayer::factory()
{
    return new AmdPlayer();
}

bool AmdPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if(!f)
        return false;
#pragma pack(push,1)
    struct
    {
        char id[9];
        uint8_t version;
    } header;
#pragma pack(pop)
    const Command convfx[10] = {
        Command::None,
        Command::SlideUp,
        Command::SlideDown,
        Command::SetVolume,
        Command::SetFineVolume2,
        Command::OrderJump,
        Command::PatternBreak,
        Command::AMDSpeed,
        Command::Porta,
        Command::Special };
    const unsigned char convvol[64] = {
        0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 0xa, 0xa, 0xb,
        0xc, 0xc, 0xd, 0xe, 0xe, 0xf, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x14,
        0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x21,
        0x22, 0x23, 0x25, 0x26, 0x28, 0x29, 0x2b, 0x2d, 0x2e, 0x30, 0x32, 0x35,
        0x37, 0x3a, 0x3c, 0x3f
    };

    // file validation section
    if(f.size() < 1072)
    {
        return false;
    }
    f.seek(1062);
    f >> header;
    if(strncmp(header.id, "<o\xefQU\xeeRoR", 9) && strncmp(header.id, "MaDoKaN96", 9))
    {
        return false;
    }

    // load section
    f.seek(0);

    static constexpr size_t StringLength = 24;

    char str[StringLength];
    f.read(str, StringLength);
    m_amdSongname = stringncpy(str, StringLength);
    f.read(str, StringLength);
    m_author = stringncpy(str, StringLength);
    for(int i = 0; i < 26; i++)
    {
        f.read(str, StringLength - 1);
        m_instrumentNames[i] = stringncpy(str, StringLength - 1);
        std::replace(m_instrumentNames[i].begin(), m_instrumentNames[i].end(), '\xff', '\x20');

        Instrument::Data data;
        f.read(data.data(), 11);

        static const uint8_t mapping[11] = { 10, 0, 5, 2, 7, 3, 8, 4, 9, 1, 6 };
        ModPlayer::Instrument& inst = addInstrument();
        for(int j = 0; j < 11; ++j)
            inst.data[j] = data[mapping[j]];
    }
    uint8_t orderCount;
    f >> orderCount;
    if(orderCount > 128)
        orderCount = 128;
    uint8_t tmp8;
    f >> tmp8;
    //m_maxUsedPattern = tmp8 + 1;
    const auto maxUsedPattern = tmp8 + 1;
    for(uint8_t i = 0; i < orderCount; ++i)
    {
        f >> tmp8;
        addOrder(tmp8);
    }
    f.seekrel(128 - orderCount);
    f.seekrel(10);
    int maxi = 0;
    if(header.version == 0x10)
    { // unpacked module
        maxi = maxUsedPattern * 9;
        for(int i = 0; i < 64; i++)
            for(int j = 0; j < 9; ++j)
                setCellColumnMapping(i, j, i * 9 + j + 1);
        int t = 0;
        while(f.pos() != f.size())
        {
            for(int j = 0; j < 64; j++)
                for(int i = t; i < t + 9; i++)
                {
                    uint8_t buf;
                    f >> buf;
                    PatternCell& cell = patternCell(i, j);
                    cell.loNybble = (buf & 127) % 10;
                    cell.hiNybble = (buf & 127) / 10;
                    f >> buf;
                    cell.instrument = buf >> 4;
                    cell.command = convfx[buf & 0x0f];
                    f >> buf;
                    if(buf >> 4) // fix bug in AMD save routine
                        cell.note = ((buf & 14) >> 1) * 12 + (buf >> 4);
                    else
                        cell.note = 0;
                    cell.instrument += (buf & 1) << 4;
                }
            t += 9;
        }
    }
    else
    { // packed module
        for(int i = 0; i < maxUsedPattern; i++)
        {
            for(int j = 0; j < 9; j++)
            {
                uint16_t tmp16;
                f >> tmp16;
                setCellColumnMapping(i, j, tmp16 + 1);
            }
        }
        uint16_t numtrax;
        f >> numtrax;
        for(int k = 0; k < numtrax; k++)
        {
            uint16_t i;
            f >> i;
            if(i > 575)
                i = 575; // fix corrupted modules
            maxi = (i + 1 > maxi ? i + 1 : maxi);
            int j = 0;
            do
            {
                uint8_t buf;
                f >> buf;
                if(buf & 128)
                {
                    for(int t = j; t < j + (buf & 127) && t < 64; t++)
                    {
                        PatternCell& cell = patternCell(i, t);
                        cell.command = Command::None;
                        cell.instrument = 0;
                        cell.note = 0;
                        cell.hiNybble = 0;
                        cell.loNybble = 0;
                    }
                    j += buf & 127;
                    continue;
                }
                PatternCell& cell = patternCell(i, j);
                cell.loNybble = buf % 10;
                cell.hiNybble = buf / 10;
                f >> buf;
                cell.instrument = buf >> 4;
                cell.command = convfx[buf & 0x0f];
                f >> buf;
                if(buf >> 4) // fix bug in AMD save routine
                    cell.note = ((buf & 14) >> 1) * 12 + (buf >> 4);
                else
                    cell.note = 0;
                cell.instrument += (buf & 1) << 4;
                j++;
            } while(j < 64);
        }
    }

    // convert to protracker replay data
    setInitialTempo(50);
    setRestartOrder(0);
    setDecimalValues();
    for(int i = 0; i < maxi; i++)
    { // convert patterns
        for(int j = 0; j < 64; j++)
        {
            PatternCell& cell = patternCell(i, j);
            // extended command
            if(cell.command == Command::Special)
            {
                if(cell.hiNybble == 2)
                {
                    cell.command = Command::SA2VolSlide;
                    cell.hiNybble = cell.loNybble;
                    cell.loNybble = 0;
                }

                if(cell.hiNybble == 3)
                {
                    cell.command = Command::SA2VolSlide;
                    cell.hiNybble = 0;
                }
            }

            // fix volume
            if(cell.command == Command::SetFineVolume2)
            {
                int vol = convvol[cell.hiNybble * 10 + cell.loNybble];

                if(vol > 63)
                    vol = 63;
                cell.hiNybble = vol / 10;
                cell.loNybble = vol % 10;
            }
        }
    }

    rewind(0);
    return true;
}

size_t AmdPlayer::framesUntilUpdate() const
{
    if(currentTempo())
        return SampleRate / currentTempo();
    else
        return static_cast<size_t>(SampleRate / 18.2);
}