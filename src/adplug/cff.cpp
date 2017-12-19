/*
  AdPlug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2008 Simon Peter <dn.tlp@gmx.net>, et al.

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

  cff.cpp - BoomTracker loader by Riven the Mage <riven@ok.ru>
*/
/*
  NOTE: Conversion of slides is not 100% accurate. Original volume slides
  have effect on carrier volume only. Also, original arpeggio, frequency &
  volume
  slides use previous effect data instead of current.
*/

#include <cstdlib>

#include "stream/filestream.h"

#include "cff.h"
#include <stuff/stringutils.h>

/* -------- Public Methods -------------------------------- */

Player* CffPlayer::factory()
{
    return new CffPlayer();
}

bool CffPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }
    const uint8_t conv_inst[11] = {2, 1, 10, 9, 4, 3, 6, 5, 0, 8, 7};
    const std::array<uint16_t, 12> conv_note = {0x16B, 0x181, 0x198, 0x1B0, 0x1CA,
                                                0x1E5, 0x202, 0x220, 0x241, 0x263,
                                                0x287, 0x2AE};

    // '<CUD-FM-File>' - signed ?
    f >> m_header;
    if( memcmp(m_header.id, "<CUD-FM-File>"
                   "\x1A\xDE\xE0",
               16) != 0 )
    {
        return false;
    }

    std::vector<uint8_t> module(0x10000);

    // packed ?
    if( m_header.packed )
    {
        std::unique_ptr<cff_unpacker> unpacker{new cff_unpacker()};

        std::vector<uint8_t> packedModule;
        packedModule.resize(m_header.size + 4, 0);

        f.read(packedModule.data(), m_header.size);
        unpacker->unpack(packedModule, module);

        if( module.empty() )
        {
            return false;
        }

        if( memcmp(&module[0x5E1], "CUD-FM-File - SEND A POSTCARD -", 31) != 0 )
        {
            return false;
        }
    }
    else
    {
        f.read(module.data(), m_header.size);
    }

    // init CmodPlayer
    realloc_patterns(36, 64, 9);
    init_notetable(conv_note);
    init_trackord();

    // load instruments
    for( int i = 0; i < 47; i++ )
    {
        memcpy(&m_instruments[i], &module[i * 32], sizeof(cff_instrument));

        for( int j = 0; j < 11; j++ )
        {
            addInstrument().data[conv_inst[j]] = m_instruments[i].data[j];
        }

        m_instruments[i].name[20] = 0;
    }

    // number of patterns
    const auto patternCount = module[0x5E0];

    // load title & author
    m_title = stringncpy(reinterpret_cast<const char*>(&module[0x614]), 20);
    m_author = stringncpy(reinterpret_cast<const char*>(&module[0x600]), 20);

    // load orders
    {
        static constexpr auto OrderDataOffset = 0x628;
        for( int i = 0; i < 64; i++ )
        {
            if( module[OrderDataOffset + i] & 0x80 )
            {
                break;
            }

            addOrder(module[OrderDataOffset + i]);
        }
    }

    // load tracks
    auto t = 0u;
    for( int i = 0; i < patternCount; i++ )
    {
        uint8_t old_event_byte2[9];

        memset(old_event_byte2, 0, 9);

        for( auto channel = 0u; channel < 9; channel++ )
        {
            for( auto row = 0u; row < 64; row++ )
            {
                const auto* event = reinterpret_cast<const cff_event*>(&module[0x669 + ((i * 64 + row) * 9 + channel) * 3]);
                PatternCell& cell = patternCell(t, row);

                // convert note
                if( event->byte0 == 0x6D )
                {
                    cell.note = 127;
                }
                else if( event->byte0 )
                {
                    cell.note = event->byte0;
                }

                if( event->byte2 )
                {
                    old_event_byte2[channel] = event->byte2;
                }

                // convert effect
                switch( event->byte1 )
                {
                    case 'I': // set instrument
                        cell.instrument = event->byte2 + 1;
                        cell.hiNybble = cell.loNybble = 0;
                        break;

                    case 'H': // set tempo
                        cell.command = Command::SetTempo;
                        if( event->byte2 < 16 )
                        {
                            cell.hiNybble = 0x07;
                            cell.loNybble = 0x0D;
                        }
                        break;

                    case 'A': // set speed
                        cell.command = Command::RADSpeed;
                        cell.hiNybble = event->byte2 >> 4;
                        cell.loNybble = event->byte2 & 15;
                        break;

                    case 'L': // pattern break
                        cell.command = Command::PatternBreak;
                        cell.hiNybble = event->byte2 >> 4;
                        cell.loNybble = event->byte2 & 15;
                        break;

                    case 'K': // order jump
                        cell.command = Command::OrderJump;
                        cell.hiNybble = event->byte2 >> 4;
                        cell.loNybble = event->byte2 & 15;
                        break;

                    case 'M': // set vibrato/tremolo
                        cell.command = Command::OplTremoloVibrato;
                        cell.hiNybble = event->byte2 >> 4;
                        cell.loNybble = event->byte2 & 15;
                        break;

                    case 'C': // set modulator volume
                        cell.command = Command::ModulatorVolume;
                        cell.hiNybble = (0x3F - event->byte2) >> 4;
                        cell.loNybble = (0x3F - event->byte2) & 15;
                        break;

                    case 'G': // set carrier volume
                        cell.command = Command::CarrierVolume;
                        cell.hiNybble = (0x3F - event->byte2) >> 4;
                        cell.loNybble = (0x3F - event->byte2) & 15;
                        break;

                    case 'B': // set carrier waveform
                        cell.command = Command::WaveForm;
                        cell.hiNybble = event->byte2;
                        cell.loNybble = 0x0F;
                        break;

                    case 'E': // fine frequency slide down
                        cell.command = Command::FineSlideDown;
                        cell.hiNybble = old_event_byte2[channel] >> 4;
                        cell.loNybble = old_event_byte2[channel] & 15;
                        break;

                    case 'F': // fine frequency slide up
                        cell.command = Command::FineSlideUp;
                        cell.hiNybble = old_event_byte2[channel] >> 4;
                        cell.loNybble = old_event_byte2[channel] & 15;
                        break;

                    case 'D': // fine volume slide
                        if( old_event_byte2[channel] & 15 )
                        {
                            // slide down
                            cell.command = Command::SFXFineVolumeDown;
                            cell.loNybble = old_event_byte2[channel] & 15;
                        }
                        else
                        {
                            // slide up
                            cell.command = Command::SFXFineVolumeUp;
                            cell.loNybble = old_event_byte2[channel] >> 4;
                        }
                        break;

                    case 'J': // arpeggio
                        cell.hiNybble = old_event_byte2[channel] >> 4;
                        cell.loNybble = old_event_byte2[channel] & 15;
                        break;
                }
            }

            t++;
        }
    }

    // order loop
    setRestartOrder(0);

    // default tempo
    setInitialTempo(0x7D);

    rewind(size_t(0));

    return true;
}

void CffPlayer::rewind(const boost::optional<size_t>& subsong)
{
    ModPlayer::rewind(subsong);

    // default instruments
    for( auto i = 0u; i < 9; i++ )
    {
        channel(i).instrument = i;

        const ModPlayer::Instrument& inst = instrument(i);
        channel(i).carrierVolume = 0x3f - (inst.data[10] & 0x3f);
        channel(i).modulatorVolume = 0x3f - (inst.data[9] & 0x3f);
    }
}

std::string CffPlayer::type() const
{
    if( m_header.packed )
    {
        return "BoomTracker 4, packed";
    }
    else
    {
        return "BoomTracker 4";
    }
}

std::string CffPlayer::title() const
{
    return m_title;
}

std::string CffPlayer::author() const
{
    return m_author;
}

std::string CffPlayer::instrumentTitle(size_t n) const
{
    return std::string(m_instruments[n].name);
}

size_t CffPlayer::instrumentCount() const
{
    return 47;
}

/* -------- Private Methods ------------------------------- */

/*
  Lempel-Ziv-Tyr ;-)
*/
void CffPlayer::cff_unpacker::unpack(const std::vector<uint8_t>& ibuf, std::vector<uint8_t>& obuf)
{
    obuf.clear();

    if( memcmp(ibuf.data(), "YsComp"
                   "\x07"
                   "CUD1997"
                   "\x1A\x04",
               16) != 0 )
    {
        return;
    }

    auto it = std::next(ibuf.begin(), 16);

    std::vector<std::vector<uint8_t>> dictionary;
    cleanup();
    if( !startup(dictionary, obuf, it) )
    {
        return;
    }

    // LZW
    while( auto newCode = get_code(it) )
    {
        // 0x01: end of block
        if( newCode == 1 )
        {
            cleanup();
            dictionary.clear();
            if( !startup(dictionary, obuf, it) )
            {
                return;
            }

            continue;
        }

        // 0x02: expand code length
        if( newCode == 2 )
        {
            m_codeLength++;

            continue;
        }

        // 0x03: RLE
        if( newCode == 3 )
        {
            uint8_t old_code_length = m_codeLength;

            m_codeLength = 2;

            uint8_t repeat_length = get_code(it) + 1;

            m_codeLength = 4 << get_code(it);

            unsigned long repeat_counter = get_code(it);

            if( obuf.size() + repeat_counter * repeat_length )
            {
                obuf.clear();
                return;
            }

            for( auto i = 0; i < repeat_counter * repeat_length; i++ )
            {
                obuf.emplace_back(obuf[obuf.size() - repeat_length]);
            }

            m_codeLength = old_code_length;

            if( !startup(dictionary, obuf, it) )
            {
                return;
            }

            continue;
        }

        if( newCode >= (0x104 + dictionary.size()) )
        {
            // dictionary <- old.code.string + old.code.char
            m_theString[++m_theString[0]] = m_theString[1];
        }
        else
        {
            // dictionary <- old.code.string + new.code.char
            uint8_t temp_string[256];

            translate_code(newCode, temp_string, dictionary);

            m_theString[++m_theString[0]] = temp_string[1];
        }

        expand_dictionary(m_theString, dictionary);

        // output <- new.code.string
        translate_code(newCode, m_theString, dictionary);

        for( int i = 0; i < m_theString[0]; i++ )
        {
            obuf.emplace_back(m_theString[i + 1]);
        }
    }
}

uint32_t CffPlayer::cff_unpacker::get_code(std::vector<uint8_t>::const_iterator& it)
{
    while( m_bitsLeft < m_codeLength )
    {
        m_bitsBuffer |= ((*it++) << m_bitsLeft);
        m_bitsLeft += 8;
    }

    uint32_t code = m_bitsBuffer & ((1 << m_codeLength) - 1);

    m_bitsBuffer >>= m_codeLength;
    m_bitsLeft -= m_codeLength;

    return code;
}

void CffPlayer::cff_unpacker::translate_code(unsigned long code, uint8_t* string, const std::vector<std::vector<uint8_t>>& dictionary)
{
    uint8_t translated_string[256];

    if( code >= 0x104 )
    {
        std::copy_n(dictionary[code - 0x104].begin(), dictionary[code - 0x104].size(), translated_string);
    }
    else
    {
        translated_string[0] = 1;
        translated_string[1] = (code - 4) & 0xFF;
    }

    memcpy(string, translated_string, 256);
}

void CffPlayer::cff_unpacker::cleanup()
{
    m_codeLength = 9;

    m_bitsBuffer = 0;
    m_bitsLeft = 0;
}

bool CffPlayer::cff_unpacker::startup(const std::vector<std::vector<uint8_t>>& dictionary, std::vector<uint8_t>& obuf, std::vector<uint8_t>::const_iterator& it)
{
    auto oldCode = get_code(it);

    translate_code(oldCode, m_theString, dictionary);

    for( int i = 0; i < m_theString[0]; i++ )
    {
        obuf.emplace_back(m_theString[i + 1]);
    }

    return true;
}

void CffPlayer::cff_unpacker::expand_dictionary(uint8_t* string, std::vector<std::vector<uint8_t>>& dictionary)
{
    if( string[0] >= 0xF0 )
    {
        return;
    }

    std::vector<uint8_t> stringData;
    stringData.assign(string, string + string[0] + 1);

    dictionary.emplace_back(std::move(stringData));
}
