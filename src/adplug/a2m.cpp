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

#include "compression/sixpack.h"

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
    static const Command basicCommands[16] = {
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
    static const Command specialCommands[16] = {
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
    static const Command newCommands[] = {
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
    uint16_t lengths[9];
    uint32_t channelCount;
    if( version < 5 )
    {
        f.read(lengths, 5);
        channelCount = 9;
    }
    else
    { // version >= 5
        f.read(lengths, 9);
        channelCount = 18;
    }

    // block 0
    readHeader(f, version, lengths);

    // blocks 1-4 or 1-8

    std::vector<uint8_t> data;
    auto decompressAppend = [&data, &f](size_t len) {
        const auto input = f.readVector<uint16_t>(len / 2u);
        const auto tmp = compression::SixPack(input).get();
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(data));
    };
    
    if( version == 1 || version == 5 )
    {
        decompressAppend(lengths[1]);

        if( version == 1 )
        {
            if( numpats > 16 )
            {
                decompressAppend(lengths[2]);
            }
            if( numpats > 32 )
            {
                decompressAppend(lengths[3]);
            }
            if( numpats > 48 )
            {
                decompressAppend(lengths[4]);
            }
        }
        else
        {
            if( numpats > 8 )
            {
                decompressAppend(lengths[2]);
            }
            if( numpats > 16 )
            {
                decompressAppend(lengths[3]);
            }
            if( numpats > 24 )
            {
                decompressAppend(lengths[4]);
            }
            if( numpats > 32 )
            {
                decompressAppend(lengths[5]);
            }
            if( numpats > 40 )
            {
                decompressAppend(lengths[6]);
            }
            if( numpats > 48 )
            {
                decompressAppend(lengths[7]);
            }
            if( numpats > 56 )
            {
                decompressAppend(lengths[8]);
            }
        }
    }
    else
    {
        size_t total = lengths[1];
        for( int i = 0; i < (version < 5 ? numpats / 16 : numpats / 8); i++ )
        {
            total += lengths[i + 2];
        }

        data = f.readVector<uint8_t>(total);
    }

    realloc_patterns(numpats, 64, channelCount);

    if( version < 5 )
    {
        for( auto pattern = 0u; pattern < numpats; pattern++ )
        {
            for( auto row = 0u; row < 64; row++ )
            {
                for( auto channel = 0u; channel < channelCount; channel++ )
                {
                    PatternCell& cell = patternCell(pattern * channelCount + channel, row);
                    const uint8_t* o = &data[pattern * 64 * channelCount * 4 + row * channelCount * 4 + channel * 4];

                    cell.note = o[0] == 255 ? PatternCell::KeyOff : o[0];
                    cell.instrument = o[1];
                    cell.command = basicCommands[o[2]];
                    cell.loNybble = o[3] & 0x0f;
                    if( cell.command != Command::Special )
                    {
                        cell.hiNybble = o[3] >> 4;
                    }
                    else
                    {
                        cell.command = specialCommands[o[3] >> 4];
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
        for( auto pattern = 0u; pattern < numpats; pattern++ )
        {
            for( auto channel = 0u; channel < channelCount; channel++ )
            {
                for( auto row = 0u; row < 64; row++ )
                {
                    PatternCell& cell = patternCell(pattern * channelCount + channel, row);
                    const uint8_t* o = &data[pattern * 64 * channelCount * 4 + channel * 64 * 4 + row * 4];

                    cell.note = o[0] == 255 ? PatternCell::KeyOff : o[0];
                    cell.instrument = o[1];
                    cell.command = newCommands[o[2]];
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

void A2mPlayer::readHeader(FileStream& f, uint8_t version, uint16_t* lengths)
{
    std::vector<uint16_t> compressed;
    compressed.resize(lengths[0] / 2u);
    std::vector<uint8_t> data;
    size_t dataPos = 0;
    if( version == 1 || version == 5 )
    {
        f.read(compressed.data(), lengths[0] / 2u);
        data = compression::SixPack(compressed).get();
    }
    else
    {
        f.read(data.data(), lengths[0]);
    }
    m_songname.assign(&data[dataPos], &data[dataPos] + 42);
    dataPos += 43;
    m_author.assign(&data[dataPos], &data[dataPos] + 42);
    dataPos += 43;
    for( int i = 0; i < 250; ++i )
    {
        dataPos += 33;
    }

    for( int i = 0; i < 250; i++ )
    { // instruments
        ModPlayer::Instrument& inst = addInstrument();
        inst.data[0] = data[dataPos + 10];
        inst.data[1] = data[dataPos + 0];
        inst.data[2] = data[dataPos + 1];
        inst.data[3] = data[dataPos + 4];
        inst.data[4] = data[dataPos + 5];
        inst.data[5] = data[dataPos + 6];
        inst.data[6] = data[dataPos + 7];
        inst.data[7] = data[dataPos + 8];
        inst.data[8] = data[dataPos + 9];
        inst.data[9] = data[dataPos + 2];
        inst.data[10] = data[dataPos + 3];

        if( version < 5 )
        {
            inst.misc = data[dataPos + 11];
        }
        else
        { // version >= 5 -> OPL3 format
            int pan = data[dataPos + 11];

            if( pan )
            {
                inst.data[0] |= (pan & 3) << 4; // set pan
            }
            else
            {
                inst.data[0] |= 48;
            } // enable both speakers
        }

        inst.slide = data[dataPos + 12];
        dataPos += 13;
    }

    for( int i = 0; i < 128; ++i )
    {
        addOrder(data[dataPos++]);
    }
    setRestartOrder(0);
    setInitialTempo(data[dataPos++]);
    setInitialSpeed(data[dataPos++]);

    if( version >= 5 )
    {
        const auto flags = data[dataPos];

        setOpl3Mode(); // All versions >= 5 are OPL3
        if( flags & 8 )
        {
            setTremolo();
        }
        if( flags & 16 )
        {
            setVibrato();
        }
    }
}