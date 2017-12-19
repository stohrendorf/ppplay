/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  dtm.cpp - DTM loader by Riven the Mage <riven@ok.ru>
*/
/*
  NOTE: Panning (Ex) effect is ignored.
*/

#include "stream/filestream.h"

#include "dtm.h"
#include <stuff/stringutils.h>

namespace
{
#pragma pack(push, 1)
struct Event
{
    uint8_t byte0;
    uint8_t byte1;
};
#pragma pack(pop)
}

Player* DtmPlayer::factory()
{
    return new DtmPlayer();
}

bool DtmPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }
    const std::array<uint16_t, 12> conv_note{{0x16B, 0x181, 0x198, 0x1B0, 0x1CA,
                                                 0x1E5, 0x202, 0x220, 0x241, 0x263,
                                                 0x287, 0x2AE}};
    // read header
    f.read(m_header.id, 12);
    f >> m_header.version;
    f.read(m_header.title, 20);
    f.read(m_header.author, 20);
    f >> m_header.numpat >> m_header.numinst;

    // signature exists ? good version ?
    if( memcmp(m_header.id, "DeFy DTM ", 9) != 0 || m_header.version != 0x10 )
    {
        return false;
    }

    m_header.numinst++;

    // load description

    for( int i = 0; i < 16; i++ )
    {
        // get line length
        uint8_t bufstr_length;
        f >> bufstr_length;

        if( bufstr_length > 80 )
        {
            return false;
        }

        // read line
        if( bufstr_length )
        {
            char bufstr[81];
            f.read(bufstr, bufstr_length);

            for( int j = 0; j < bufstr_length; j++ )
            {
                if( !bufstr[j] )
                {
                    bufstr[j] = 0x20;
                }
            }

            bufstr[bufstr_length] = 0;

            m_description += bufstr;
        }

        m_description += "\n";
    }

    // init CmodPlayer
    realloc_patterns(m_header.numpat, 64, 9);
    init_notetable(conv_note);
    init_trackord();

    // load instruments
    for( int i = 0; i < m_header.numinst; i++ )
    {
        uint8_t name_length;
        f >> name_length;

        BOOST_ASSERT(name_length <= sizeof(Instrument::name));
        if( name_length )
        {
            f.read(m_instruments[i].name, name_length);
        }

        m_instruments[i].name[name_length] = 0;

        f.read(m_instruments[i].data, 12);

        static const uint8_t conv_inst[11] = {2, 1, 10, 9, 4, 3, 6, 5, 0, 8, 7};

        auto& instr = addInstrument();
        for( int j = 0; j < 11; j++ )
        {
            instr.data[conv_inst[j]] = m_instruments[i].data[j];
        }
    }

    // load orders
    {
        uint8_t orders[100];
        f.read(orders, 100);
        for( uint8_t order : orders )
        {
            if( order < 0x80 )
            {
                addOrder(order);
                continue;
            }

            if( order == 0xFF )
            {
                setRestartOrder(0);
            }
            else
            {
                setRestartOrder(order - 0x80u);
            }

            break;
        }
    }

    //m_maxUsedPattern = header.numpat;

    // load tracks
    auto channel = 0u;
    for( int i = 0; i < m_header.numpat; i++ )
    {
        uint16_t packed_length;
        f >> packed_length;

        std::vector<uint8_t> packed_pattern(packed_length);
        f.read(packed_pattern.data(), packed_length);

        std::vector<uint8_t> pattern = unpack_pattern(packed_pattern);

        if( pattern.empty() || pattern.size() < 0x480 )
        {
            return false;
        }
        pattern.resize(0x480);

        // convert pattern
        for( auto j = 0u; j < 9; j++ )
        {
            for( auto row = 0u; row < 64; row++ )
            {
                const auto* event = reinterpret_cast<const Event*>(&pattern[(row * 9 + j) * 2]);
                PatternCell& cell = patternCell(channel, row);

                if( event->byte0 == 0x80 )
                {
                    // instrument
                    if( event->byte1 <= 0x80 )
                    {
                        cell.instrument = event->byte1 + 1;
                    }
                }
                else
                {
                    // note + effect
                    cell.note = event->byte0;

                    if( (event->byte0 != 0) && (event->byte0 != 127) )
                    {
                        cell.note++;
                    }

                    // convert effects
                    switch( event->byte1 >> 4 )
                    {
                        case 0x0: // pattern break
                            if( (event->byte1 & 15) == 1 )
                            {
                                cell.command = Command::PatternBreak;
                            }
                            break;

                        case 0x1: // freq. slide up
                            cell.command = Command::SlideUpDown;
                            cell.hiNybble = event->byte1 & 15;
                            break;

                        case 0x2: // freq. slide down
                            cell.command = Command::SlideUpDown;
                            cell.loNybble = event->byte1 & 15;
                            break;

                        case 0xA: // set carrier volume
                        case 0xC: // set instrument volume
                            cell.command = Command::CarrierVolume;
                            cell.hiNybble = (0x3F - (event->byte1 & 15)) >> 4;
                            cell.loNybble = (0x3F - (event->byte1 & 15)) & 15;
                            break;

                        case 0xB: // set modulator volume
                            cell.command = Command::ModulatorVolume;
                            cell.hiNybble = (0x3F - (event->byte1 & 15)) >> 4;
                            cell.loNybble = (0x3F - (event->byte1 & 15)) & 15;
                            break;

                        case 0xE: // set panning
                            break;

                        case 0xF: // set speed
                            cell.command = Command::SA2Speed; // was: PatternBreak
                            cell.loNybble = event->byte1 & 15;
                            break;
                    }
                }
            }

            channel++;
        }
    }

    // initial speed
    setInitialSpeed(2);

    rewind(size_t(0));

    return true;
}

void DtmPlayer::rewind(const boost::optional<size_t>& subsong)
{
    ModPlayer::rewind(subsong);

    // default instruments
    for( auto i = 0u; i < 9; i++ )
    {
        channel(i).instrument = i;

        const ModPlayer::Instrument& inst = instrument(i);
        channel(i).carrierVolume = 63 - (inst.data[10] & 63);
        channel(i).modulatorVolume = 63 - (inst.data[9] & 63);
    }
}

size_t DtmPlayer::framesUntilUpdate() const
{
    return static_cast<size_t>(SampleRate / 18.2);
}

std::string DtmPlayer::type() const
{
    return "DeFy Adlib Tracker";
}

std::string DtmPlayer::title() const
{
    return stringncpy(m_header.title, 20);
}

std::string DtmPlayer::author() const
{
    return stringncpy(m_header.author, 20);
}

std::string DtmPlayer::description() const
{
    return m_description;
}

std::string DtmPlayer::instrumentTitle(size_t n) const
{
    return stringncpy(m_instruments[n].name, 13);
}

size_t DtmPlayer::instrumentCount() const
{
    return m_header.numinst;
}

/* -------- Private Methods ------------------------------- */

std::vector<uint8_t> DtmPlayer::unpack_pattern(const std::vector<uint8_t>& input)
{
    std::vector<uint8_t> result;

    // RLE
    for( auto inp = input.begin(); inp < input.end(); )
    {
        auto repeat_byte = *inp++;

        uint8_t repeat_counter = 1;
        if( (repeat_byte & 0xF0) == 0xD0 )
        {
            repeat_counter = repeat_byte & 15;
            repeat_byte = *inp++;
        }

        for( int i = 0; i < repeat_counter; i++ )
        {
            result.emplace_back(repeat_byte);
        }
    }

    return result;
}
