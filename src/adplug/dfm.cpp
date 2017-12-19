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
 * dfm.cpp - Digital-FM Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstdio>
#include <cstring>

#include "stream/filestream.h"
#include "stuff/stringutils.h"

#include "dfm.h"

Player* DfmPlayer::factory()
{
    return new DfmPlayer();
}

bool DfmPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }
    const Command convfx[8] = {Command::Sentinel,
                               Command::Sentinel,
                               Command::SetFineVolume2,
                               Command::RADSpeed,
                               Command::FineSlideUp,
                               Command::FineSlideDown,
                               Command::Sentinel,
                               Command::PatternBreak};

    // file validation
    char id[4];
    f.read(id, 4);

    uint8_t hiver, lover;
    f >> hiver >> lover;

    if( strncmp(id, "DFM\x1a", 4) != 0 || hiver > 1 )
    {
        return false;
    }

    m_type = stringFmt("Digital-FM %d.%d", int(hiver), int(lover));

    // load
    setInitialTempo(0);
    init_trackord();
    {
        char songinfo[33];
        f.read(songinfo, 33);
        m_songInfo = stringncpy(songinfo, 33);
    }
    uint8_t initSpeed;
    f >> initSpeed;
    setInitialSpeed(initSpeed);
    for( auto i = 0; i < 32; i++ )
    {
        char instname[12];
        f.read(instname, 12);
        m_instName[i] = stringncpy(instname, 12);
    }
    for( auto i = 0; i < 32; i++ )
    {
        ModPlayer::Instrument& inst = addInstrument();
        f >> inst.data[1];
        f >> inst.data[2];
        f >> inst.data[9];
        f >> inst.data[10];
        f >> inst.data[3];
        f >> inst.data[4];
        f >> inst.data[5];
        f >> inst.data[6];
        f >> inst.data[7];
        f >> inst.data[8];
        f >> inst.data[0];
    }

    const auto orderListEnd = f.pos() + 128;
    for( uint8_t order; orderCount() < 128 && f >> order && order != 0x80; )
    {
        addOrder(order);
    }
    setRestartOrder(0);

    f.seek(orderListEnd);
    uint8_t patternCount;
    f >> patternCount;
    for( auto patternIdx = 0; patternIdx < patternCount; patternIdx++ )
    {
        uint8_t pattern;
        f >> pattern;
        for( auto row = 0u; row < 64; row++ )
        {
            for( auto chan = 0u; chan < 9; chan++ )
            {
                uint8_t note;
                f >> note;
                PatternCell& cell = patternCell(pattern * 9 + chan, row);
                if( (note & 0x0f) == 0x0f )
                {
                    cell.note = 0x7f; // key off
                }
                else
                {
                    cell.note = ((note & 0x7f) >> 4) * 12 + (note & 0x0f);
                }
                if( note & 0x80 )
                {
                    // additional effect byte
                    uint8_t fx;
                    f >> fx;
                    if( (fx >> 5) == 1 )
                    {
                        cell.instrument = (fx & 31) + 1;
                    }
                    else
                    {
                        cell.command = convfx[fx >> 5];
                        if( cell.command == Command::SetFineVolume2 )
                        { // set volume
                            auto param = fx & 0x1f;
                            param = 0x3f - param * 2;
                            cell.hiNybble = param >> 4;
                            cell.loNybble = param & 0x0f;
                        }
                        else
                        {
                            cell.hiNybble = (fx & 0x1f) >> 4;
                            cell.loNybble = fx & 0x0f;
                        }
                    }
                }
            }
        }
    }

    rewind(size_t(0));
    return true;
}

std::string DfmPlayer::type() const
{
    return m_type;
}

size_t DfmPlayer::framesUntilUpdate() const
{
    return SampleRate / 125;
}
