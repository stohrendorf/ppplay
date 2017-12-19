/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2007 Simon Peter <dn.tlp@gmx.net>, et al.

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

  fmc.cpp - FMC Loader by Riven the Mage <riven@ok.ru>
*/

#include "stream/filestream.h"

#include "fmc.h"

/* -------- Public Methods -------------------------------- */

Player* FmcPlayer::factory()
{
    return new FmcPlayer();
}

bool FmcPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }
    const Command conv_fx[16] = {Command::None,
                                 Command::SlideUp,
                                 Command::SlideDown,
                                 Command::Porta,
                                 Command::Vibrato,
                                 Command::NoteOff,
                                 Command::Sentinel,
                                 Command::Sentinel,
                                 Command::Sentinel,
                                 Command::Sentinel,
                                 Command::VolSlide,
                                 Command::OrderJump,
                                 Command::SetFineVolume,
                                 Command::PatternBreak,
                                 Command::Special,
                                 Command::SA2Speed};

    // read header
    f.read(header.id, 4);
    f.read(header.title, 21);
    f >> header.numchan;

    // 'FMC!' - signed ?
    if( strncmp(header.id, "FMC!", 4) != 0 )
    {
        return false;
    }

    // init CmodPlayer
    realloc_patterns(64, 64, header.numchan);
    init_trackord();

    // load order
    for( uint8_t order; orderCount() < 256 && f >> order && order < 0xFE; )
    {
        addOrder(order);
    }
    f.seekrel(256 - orderCount());

    f.seekrel(2);

    // load instruments
    for( auto& instrument : instruments )
    {
        f >> instrument;
    }

    // load tracks
    auto t = 0u;
    for( auto i = 0u; i < 64 && f.pos() < f.size(); i++ )
    {
        for( auto j = 0u; j < header.numchan; j++ )
        {
            for( auto k = 0u; k < 64; k++ )
            {
                fmc_event event;

                // read event
                f >> event;

                PatternCell& cell = patternCell(t, k);
                // convert event
                cell.note = event.byte0 & 0x7F;
                cell.instrument = ((event.byte0 & 0x80) >> 3) + (event.byte1 >> 4) + 1;
                cell.command = conv_fx[event.byte1 & 0x0F];
                cell.hiNybble = event.byte2 >> 4;
                cell.loNybble = event.byte2 & 0x0F;

                // fix effects
                if( cell.command == Command::Special )
                { // 0x0E (14): Retrig
                    cell.command = Command::SFXRetrigger;
                }
                if( cell.command == Command::VolSlide )
                { // 0x1A (26): Volume Slide
                    if( cell.hiNybble > cell.loNybble )
                    {
                        cell.hiNybble -= cell.loNybble;
                        cell.loNybble = 0;
                    }
                    else
                    {
                        cell.loNybble -= cell.hiNybble;
                        cell.hiNybble = 0;
                    }
                }
            }

            t++;
        }
    }

    // convert instruments
    for( int i = 0; i < 31; i++ )
    {
        addInstrument(instruments[i]);
    }

    // data for Protracker
    BOOST_ASSERT(header.numchan <= 32);
    disableAllChannels();
    for( uint8_t i = 0; i < header.numchan; ++i )
    {
        enableChannel(i);
    }
    //m_maxUsedPattern = t / header.numchan;
    setRestartOrder(0);

    // flags
    setFaust();

    rewind(size_t(0));

    return true;
}

size_t FmcPlayer::framesUntilUpdate() const
{
    return SampleRate / 50;
}

std::string FmcPlayer::type() const
{
    return std::string("Faust Music Creator");
}

std::string FmcPlayer::title() const
{
    return std::string(header.title);
}

std::string FmcPlayer::instrumentTitle(size_t n) const
{
    return instruments[n].name;
}

size_t FmcPlayer::instrumentCount() const
{
    return 32;
}

/* -------- Private Methods ------------------------------- */

void FmcPlayer::addInstrument(const Instrument& instrument)
{
    ModPlayer::Instrument& inst = addInstrument();
    inst.data[0] = ((instrument.synthesis & 1) ^ 1);
    inst.data[0] |= ((instrument.feedback & 7) << 1);

    inst.data[3] = ((instrument.mod_attack & 15) << 4);
    inst.data[3] |= (instrument.mod_decay & 15);
    inst.data[5] = ((15 - (instrument.mod_sustain & 15)) << 4);
    inst.data[5] |= (instrument.mod_release & 15);
    inst.data[9] = (63 - (instrument.mod_volume & 63));
    inst.data[9] |= ((instrument.mod_ksl & 3) << 6);
    inst.data[1] = (instrument.mod_freq_multi & 15);
    inst.data[7] = (instrument.mod_waveform & 3);
    inst.data[1] |= ((instrument.mod_sustain_sound & 1) << 5);
    inst.data[1] |= ((instrument.mod_ksr & 1) << 4);
    inst.data[1] |= ((instrument.mod_vibrato & 1) << 6);
    inst.data[1] |= ((instrument.mod_tremolo & 1) << 7);

    inst.data[4] = ((instrument.car_attack & 15) << 4);
    inst.data[4] |= (instrument.car_decay & 15);
    inst.data[6] = ((15 - (instrument.car_sustain & 15)) << 4);
    inst.data[6] |= (instrument.car_release & 15);
    inst.data[10] = (63 - (instrument.car_volume & 63));
    inst.data[10] |= ((instrument.car_ksl & 3) << 6);
    inst.data[2] = (instrument.car_freq_multi & 15);
    inst.data[8] = (instrument.car_waveform & 3);
    inst.data[2] |= ((instrument.car_sustain_sound & 1) << 5);
    inst.data[2] |= ((instrument.car_ksr & 1) << 4);
    inst.data[2] |= ((instrument.car_vibrato & 1) << 6);
    inst.data[2] |= ((instrument.car_tremolo & 1) << 7);

    inst.slide = instrument.pitch_shift;
}