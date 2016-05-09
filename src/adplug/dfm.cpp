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

Player* CdfmLoader::factory()
{
    return new CdfmLoader();
}

bool CdfmLoader::load(const std::string& filename)
{
    FileStream f(filename);
    if(!f)
        return false;
    const Command convfx[8] = { Command::Sentinel,
        Command::Sentinel,
        Command::SetFineVolume2,
        Command::RADSpeed,
        Command::FineSlideUp,
        Command::FineSlideDown,
        Command::Sentinel,
        Command::PatternBreak };

    // file validation
    char id[4];
    f.read(id, 4);

    uint8_t hiver, lover;
    f >> hiver >> lover;

    if(strncmp(id, "DFM\x1a", 4) || hiver > 1)
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
        m_songInfo.assign(songinfo, 1, songinfo[0]);
    }
    uint8_t initSpeed;
    f >> initSpeed;
    setInitialSpeed(initSpeed);
    for(auto i = 0; i < 32; i++)
    {
        char instname[12];
        f.read(instname, 12);
        m_instName[i].assign(instname, 1, instname[0]);
    }
    for(auto i = 0; i < 32; i++)
    {
        CmodPlayer::Instrument& inst = addInstrument();
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
    for(uint8_t order; orderCount() < 128 && f >> order && order != 0x80; )
        addOrder(order);
    setRestartOrder(0);

    f.seekrel(128 - orderCount());
    uint8_t npats;
    f >> npats;
    for(auto i = 0; i < npats; i++)
    {
        uint8_t n;
        f >> n;
        for(auto r = 0; r < 64; r++)
        {
            for(auto c = 0; c < 9; c++)
            {
                uint8_t note;
                f >> note;
                PatternCell& cell = patternCell(n * 9 + c, r);
                if((note & 15) == 15)
                    cell.note = 127; // key off
                else
                    cell.note = ((note & 127) >> 4) * 12 + (note & 15);
                if(note & 128)
                { // additional effect byte
                    uint8_t fx;
                    f >> fx;
                    if((fx >> 5) == 1)
                        cell.instrument = (fx & 31) + 1;
                    else
                    {
                        cell.command = convfx[fx >> 5];
                        if(cell.command == Command::SetFineVolume2)
                        { // set volume
                            auto param = fx & 31;
                            param = 63 - param * 2;
                            cell.hiNybble = param >> 4;
                            cell.loNybble = param & 15;
                        }
                        else
                        {
                            cell.hiNybble = (fx & 31) >> 4;
                            cell.loNybble = fx & 15;
                        }
                    }
                }
            }
        }
    }

    rewind(0);
    return true;
}

std::string CdfmLoader::type() const
{
    return m_type;
}

size_t CdfmLoader::framesUntilUpdate() const
{
    return SampleRate / 125;
}