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
 * rad.cpp - RAD Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * BUGS:
 * some volumes are dropped out
 */

#include <cstring>

#include "stream/filestream.h"

#include "rad.h"

CPlayer *CradLoader::factory() { return new CradLoader(); }

bool CradLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;

    // file validation section
    char id[16];
    f.read(id, 16);
    uint8_t version;
    f >> version;
    if (strncmp(id, "RAD by REALiTY!!", 16) || version != 0x10) {
        return false;
    }

    // load section
    uint8_t flags;
    f >> flags;
    if (flags & 0x80) { // description
        m_description.clear();
        uint8_t buf;
        while(f>>buf && buf) {
            if (buf == 1)
                m_description += "\n";
            else if (buf >= 2 && buf <= 0x1f)
                for (int i = 0; i < buf; i++)
                    m_description += " ";
            else {
                m_description += char(buf);
            }
        }
    }
    {
        uint8_t buf;
        std::vector<CmodPlayer::Instrument::Data> instruments;
        while(f>>buf && buf) {
            buf--;
            if(buf >= instruments.size())
                instruments.resize(buf+1);
            auto& inst = instruments[buf];
            f >> inst[2];
            f >> inst[1];
            f >> inst[10];
            f >> inst[9];
            f >> inst[4];
            f >> inst[3];
            f >> inst[6];
            f >> inst[5];
            f >> inst[0];
            f >> inst[8];
            f >> inst[7];
        }
        for(const auto& inst : instruments)
            addInstrument().data = inst;
    }
    {
        uint8_t length;
        f >> length;
        for(uint8_t order; orderCount()<length && f>>order; )
            addOrder(order);
    }
    uint16_t patofs[32];
    f.read(patofs, 32);
    init_trackord();             // patterns
    for (int i = 0; i < 32; i++) {
        if (patofs[i]) {
            f.seek(patofs[i]);
            while (true) {
                uint8_t buf;
                f >> buf;
                uint8_t b = buf & 127;
                while (true) {
                    uint8_t ch;
                    f >> ch;
                    uint8_t c = ch & 127;
                    uint8_t inp;
                    f >> inp;
                    PatternCell& cell = patternCell(i*9+c, b);
                    cell.note = inp & 127;
                    cell.instrument = (inp & 128) >> 3;
                    f >> inp;
                    cell.instrument += inp >> 4;
                    static constexpr Command convfx[16] = { Command::Sentinel,
                                                            Command::SlideUp,
                                                            Command::SlideDown,
                                                            Command::Porta,
                                                            Command::Sentinel,
                                                            Command::PortaVolSlide,
                                                            Command::Sentinel,
                                                            Command::Sentinel,
                                                            Command::Sentinel,
                                                            Command::Sentinel,
                                                            Command::RADVolSlide,
                                                            Command::Sentinel,
                                                            Command::SetFineVolume2,
                                                            Command::PatternBreak,
                                                            Command::Sentinel,
                                                            Command::RADSpeed };
                    cell.command = convfx[inp & 15];
                    if (inp & 15) {
                        f >> inp;
                        cell.hiNybble = inp / 10;
                        cell.loNybble = inp % 10;
                    }
                    if (ch & 0x80)
                        break;
                }
                if (buf & 0x80)
                    break;
            }
        }
        else {
            for(int j=0; j<9; ++j)
                setCellColumnMapping(i,j,0);
        }
    }

    // convert replay data
    for (int i = 0; i < 32 * 9; i++) // convert patterns
        for (int j = 0; j < 64; j++) {
            PatternCell& cell = patternCell(i,j);
            if (cell.note == 15)
                cell.note = 127;
            if (cell.note > 16 && cell.note < 127)
                cell.note -= 4 * (cell.note >> 4);
            if (cell.note && cell.note < 126)
                cell.note++;
        }
    setRestartOrder(0);
    setInitialSpeed(flags & 0x1f);
    setInitialTempo(flags & 0x40 ? 0 : 50);
    setDecimalValues();

    rewind(0);
    return true;
}

size_t CradLoader::framesUntilUpdate() const {
    if (currentTempo() != 0)
        return SampleRate / currentTempo();
    else
        return SampleRate / 18.2;
}
