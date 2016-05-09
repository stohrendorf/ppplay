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
 * protrack.cpp - Generic Protracker Player
 *
 * NOTES:
 * This is a generic Protracker-based formats player. It offers all Protracker
 * features, plus a good set of extensions to be compatible to other Protracker
 * derivatives. It is derived from the former SA2 player. If you got a
 * Protracker-like format, this is most certainly the player you want to use.
 */

#include "protrack.h"

namespace
{
constexpr auto JUMPMARKER = 0x80;   // Orderlist jump marker

// SA2 compatible adlib note table
const std::array<uint16_t,12> sa2_notetable = { 340, 363, 385, 408, 432,
                                                458, 485, 514, 544, 577,
                                                611, 647 };

// SA2 compatible vibrato rate table
const std::array<uint8_t,32> vibratotab = { 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                             10, 11, 12, 13, 14, 15, 16,
                                             16, 15, 14, 13, 12, 11, 10,
                                             9, 8, 7, 6, 5, 4, 3, 2, 1 };
}

/*** public methods *************************************/

CmodPlayer::CmodPlayer() {
    realloc_patterns(64, 64, 9);
    //m_instruments.resize(250);
    init_notetable(sa2_notetable);
}

bool CmodPlayer::update() {
    bool pattbreak = false;

    if (currentSpeed() == 0) // song full stop
        return !m_songEnd;

    // effect handling (timer dependant)
    for(size_t channelIdx = 0; channelIdx < m_channels.size(); ++channelIdx) {
        auto oplchan = setOplChip(channelIdx);
        Channel& chan = m_channels[channelIdx];

        if (m_arpeggioList.is_initialized() && m_arpeggioCommands.is_initialized() && m_instruments[chan.instrument].arpeggioStart) { // special arpeggio
            auto command = (*m_arpeggioCommands)[chan.arppos];
            auto data = (*m_arpeggioList)[chan.arppos];
            static constexpr auto DoNothing = 0;
            static constexpr auto SetVolume = 252;
            static constexpr auto ReleaseNote = 253;
            static constexpr auto SetPosition = 254;
            static constexpr auto NoCommand = 255;
            if (chan.arpspdcnt) {
                chan.arpspdcnt--;
            }
            else if (command != NoCommand) {
                switch (command) {
                case DoNothing:
                    break;
                case SetVolume:
                    chan.carrierVolume = std::min<uint8_t>(63, data); // set volume
                    chan.modulatorVolume = chan.carrierVolume;
                    setVolume(channelIdx);
                    break;
                case ReleaseNote:
                    chan.key = 0;
                    setFreq(channelIdx);
                    break; // release sustaining note
                case SetPosition:
                    chan.arppos = data;
                    data = (*m_arpeggioList)[chan.arppos];
                    command = (*m_arpeggioCommands)[chan.arppos];
                    break; // arpeggio loop
                default:
                    if (command / 10)
                        getOpl()->writeReg(0xe3 + s_opTable[oplchan], command / 10 - 1);
                    if (command % 10)
                        getOpl()->writeReg(0xe0 + s_opTable[oplchan], (command % 10) - 1);
                    if (command < 10) // ?????
                        getOpl()->writeReg(0xe0 + s_opTable[oplchan], command - 1);
                }

                if (command != SetVolume) {
                    if (data <= 96)
                        setNote(channelIdx, chan.note + data);
                    else if (data >= 100)
                        setNote(channelIdx, data - 100);
                }
                else {
                    setNote(channelIdx, chan.note);
                }

                setFreq(channelIdx);
                if (command != NoCommand)
                    chan.arppos++;
                chan.arpspdcnt = m_instruments[chan.instrument].arpeggioSpeed - 1;
            }
        }

        const auto hiNybble = chan.hiNybble;
        const auto loNybble = chan.loNybble;
        int info;
        if(m_decimalValues)
            info = chan.hiNybble * 10 + chan.loNybble;
        else
            info = (chan.hiNybble << 4) + chan.loNybble;

        switch (chan.fx) {
        case Command::None:
            if (info) { // arpeggio
                if (chan.trigger < 2)
                    chan.trigger++;
                else
                    chan.trigger = 0;
                switch (chan.trigger) {
                case 0:
                    setNote(channelIdx, chan.note);
                    break;
                case 1:
                    setNote(channelIdx, chan.note + hiNybble);
                    break;
                case 2:
                    setNote(channelIdx, chan.note + loNybble);
                }
                setFreq(channelIdx);
            }
            break;
        case Command::SlideUp:
            chan.slideUp(info);
            setFreq(channelIdx);
            break; // slide up
        case Command::SlideDown:
            chan.slideDown(info);
            setFreq(channelIdx);
            break; // slide down
        case Command::Porta:
            tonePortamento(channelIdx, chan.portaSpeed);
            break; // tone portamento
        case Command::Vibrato:
            vibrato(channelIdx, chan.vibratoSpeed, chan.vibratoDepth);
            break; // vibrato
        case Command::PortaVolSlide:  // tone portamento & volume slide
        case Command::VibVolSlide:
            if (chan.fx == Command::PortaVolSlide) // vibrato & volume slide
                tonePortamento(channelIdx, chan.portaSpeed);
            else
                vibrato(channelIdx, chan.vibratoSpeed, chan.vibratoDepth);
        case Command::SA2VolSlide:
            if (m_patternDelay % 4) // SA2 volume slide
                break;
            if (hiNybble)
                chan.volumeUp(hiNybble);
            else
                chan.volumeDown(loNybble);
            setVolume(channelIdx);
            break;
        case Command::SFXRetrigger:
            if (!(m_patternDelay % (loNybble + 1)))
                playNote(channelIdx);
            break;
        case Command::AMDVolSlide:
            if (m_patternDelay % 4) // AMD volume slide
                break;
            if (hiNybble)
                chan.distinctVolumeUp(hiNybble, m_instruments);
            else
                chan.distinctVolumeDown(loNybble, m_instruments);
            setVolume(channelIdx);
            break;
        case Command::RADVolSlide: // RAD volume slide
            if (info < 50)
                chan.distinctVolumeDown(info, m_instruments);
            else
                chan.distinctVolumeUp(info - 50, m_instruments);
            setVolume(channelIdx);
            break;
        case Command::VolSlide: // volume slide
            if (hiNybble)
                chan.volumeUp(hiNybble);
            else
                chan.volumeDown(loNybble);
            setVolume(channelIdx);
            break;
        case Command::SlideUpDown:
            if (hiNybble) {
                chan.slideUp(1);
                chan.hiNybble--;
            }
            if (loNybble) {
                chan.slideDown(1);
                chan.loNybble--;
            }
            setFreq(channelIdx);
            break;
        }
    }

    if (m_patternDelay) { // speed compensation
        m_patternDelay--;
        return !m_songEnd;
    }

    // arrangement handling
    if (!resolveOrder()) {
        return !m_songEnd;
    }
    size_t pattnr = currentPattern();

    // play row
    auto pattern_delay = 0;
    auto row = currentRow();
    for(size_t channelIdx = 0; channelIdx < m_channels.size(); ++channelIdx) {
        auto oplchan = setOplChip(channelIdx);
        Channel& chan = m_channels[channelIdx];

        if( (m_activechan & (1<<channelIdx)) == 0 ) { // channel active?
            continue;
        }
        auto track = m_cellColumnMapping.at(pattnr,channelIdx);
        if(track == 0) { // resolve track
            continue;
        }
        else {
            track--;
        }

        bool donote = false;
        if (m_patternCells.at(track,row).instrument != 0) {
            chan.instrument = m_patternCells.at(track,row).instrument - 1;
            if (!m_faust) {
                chan.carrierVolume = 63 - (m_instruments[chan.instrument].data[10] & 63);
                chan.modulatorVolume = 63 - (m_instruments[chan.instrument].data[9] & 63);
                setVolume(channelIdx);
            }
        }

        if (m_patternCells.at(track,row).note && m_patternCells.at(track,row).command != Command::Porta) { // no tone portamento
            chan.note = m_patternCells.at(track,row).note;
            setNote(channelIdx, m_patternCells.at(track,row).note);
            chan.portaTargetFrequency = chan.frequency;
            chan.portaTargetOctave = chan.octave;
            chan.arppos = m_instruments[chan.instrument].arpeggioStart;
            chan.arpspdcnt = 0;
            if (m_patternCells.at(track,row).note != 127) // handle key off
                donote = true;
        }
        chan.fx = m_patternCells.at(track,row).command;
        chan.hiNybble = m_patternCells.at(track,row).hiNybble;
        chan.loNybble = m_patternCells.at(track,row).loNybble;

        if (donote) {
            playNote(channelIdx);
        }

        // command handling (row dependant)
        const auto hiNybble = chan.hiNybble;
        const auto loNybble = chan.loNybble;
        uint32_t info;
        if(m_decimalValues)
            info = chan.hiNybble * 10 + chan.loNybble;
        else
            info = (chan.hiNybble << 4) + chan.loNybble;
        switch (chan.fx) {
        case Command::Porta: // tone portamento
            if (m_patternCells.at(track,row).note) {
                if (m_patternCells.at(track,row).note < 13)
                    chan.portaTargetFrequency = m_noteTable[m_patternCells.at(track,row).note - 1];
                else if (m_patternCells.at(track,row).note % 12 > 0)
                    chan.portaTargetFrequency = m_noteTable[(m_patternCells.at(track,row).note % 12) - 1];
                else
                    chan.portaTargetFrequency = m_noteTable[11];
                chan.portaTargetOctave = (m_patternCells.at(track,row).note - 1) / 12;
                if (m_patternCells.at(track,row).note == 127) { // handle key off
                    chan.portaTargetFrequency = chan.frequency;
                    chan.portaTargetOctave = chan.octave;
                }
            }
            if (info) // remember vars
                chan.portaSpeed = info;
            break;

        case Command::Vibrato: // vibrato (remember vars)
            if (info) {
                chan.vibratoSpeed = hiNybble;
                chan.vibratoDepth = loNybble;
            }
            break;

        case Command::SetTempo:
            setCurrentTempo(info);
            break; // set tempo

        case Command::NoteOff:
            chan.key = 0;
            setFreq(channelIdx);
            break; // release sustaining note

        case Command::SetVolume: // set carrier/modulator volume
            if (hiNybble)
                chan.carrierVolume = hiNybble * 7;
            else
                chan.modulatorVolume = loNybble * 7;
            setVolume(channelIdx);
            break;

        case Command::OrderJump: // position jump
            pattbreak = true;
            setCurrentRow(0);
            if (info < currentOrder())
                m_songEnd = true;
            setCurrentOrder(info);
            break;

        case Command::SetFineVolume: // set volume
            chan.carrierVolume = info;
            chan.modulatorVolume = info;
            if (chan.carrierVolume > 63)
                chan.carrierVolume = 63;
            if (chan.modulatorVolume > 63)
                chan.modulatorVolume = 63;
            setVolume(channelIdx);
            break;

        case Command::PatternBreak: // pattern break
            if (!pattbreak) {
                pattbreak = true;
                setCurrentRow(info);
                setCurrentOrder(currentOrder()+1);
            }
            break;

        case Command::SFXTremolo: // define cell-tremolo
            if (loNybble)
                m_oplBdRegister |= 128;
            else
                m_oplBdRegister &= 127;
            getOpl()->writeReg(0xbd, m_oplBdRegister);
            break;

        case Command::SFXVibrato: // define cell-vibrato
            if (loNybble)
                m_oplBdRegister |= 64;
            else
                m_oplBdRegister &= 191;
            getOpl()->writeReg(0xbd, m_oplBdRegister);
            break;

        case Command::SFXFineVolumeUp: // increase volume fine
            chan.distinctVolumeUp(loNybble, m_instruments);
            setVolume(channelIdx);
            break;

        case Command::SFXFineVolumeDown: // decrease volume fine
            chan.distinctVolumeDown(loNybble, m_instruments);
            setVolume(channelIdx);
            break;

        case Command::SFXSlideUp: // manual slide up
            chan.slideUp(loNybble);
            setFreq(channelIdx);
            break;

        case Command::SFXSlideDown: // manual slide down
            chan.slideDown(loNybble);
            setFreq(channelIdx);
            break;

        case Command::SFXPatternDelay: // pattern delay (rows)
            pattern_delay = loNybble * currentSpeed();
            break;

        case Command::SA2Speed: // SA2 set speed
            if (info <= 0x1f)
                setCurrentSpeed(info);
            else if (info >= 0x32)
                setCurrentTempo(info);
            if (!info)
                m_songEnd = true;
            break;

        case Command::SetFineVolume2: // alternate set volume
            chan.carrierVolume = info;
            if (chan.carrierVolume > 63)
                chan.carrierVolume = 63;
            if (m_instruments[chan.instrument].data[0] & 1) {
                chan.modulatorVolume = info;
                if (chan.modulatorVolume > 63)
                    chan.modulatorVolume = 63;
            }

            setVolume(channelIdx);
            break;

        case Command::AMDSpeed: // AMD set speed
            if (info <= 0x1f && info > 0)
                setCurrentSpeed(info);
            else if (info > 0x1f || !info)
                setCurrentTempo(info);
            break;

        case Command::RADSpeed: // RAD/A2M set speed
            setCurrentSpeed(std::max(1u, info));
            break;

        case Command::ModulatorVolume: // set modulator volume
            if (info <= 63)
                chan.modulatorVolume = info;
            else
                chan.modulatorVolume = 63;
            setVolume(channelIdx);
            break;

        case Command::CarrierVolume: // set carrier volume
            if (info <= 63)
                chan.carrierVolume = info;
            else
                chan.carrierVolume = 63;
            setVolume(channelIdx);
            break;

        case Command::FineSlideUp: // fine frequency slide up
            chan.slideUp(info);
            setFreq(channelIdx);
            break;

        case Command::FineSlideDown: // fine frequency slide down
            chan.slideDown(info);
            setFreq(channelIdx);
            break;

        case Command::WaveForm: // set carrier/modulator waveform
            if (hiNybble != 0x0f)
                getOpl()->writeReg(0xe3 + s_opTable[oplchan], hiNybble);
            if (loNybble != 0x0f)
                getOpl()->writeReg(0xe0 + s_opTable[oplchan], loNybble);
            break;

        case Command::OplTremoloVibrato: // set chip tremolo/vibrato
            if (hiNybble)
                m_oplBdRegister |= 128;
            else
                m_oplBdRegister &= 127;
            if (loNybble)
                m_oplBdRegister |= 64;
            else
                m_oplBdRegister &= 191;
            getOpl()->writeReg(0xbd, m_oplBdRegister);
            break;

        case Command::PatternDelay: // pattern delay (frames)
            pattern_delay = info;
            break;
        }
    }

    // speed compensation
    m_patternDelay = currentSpeed() - 1 + pattern_delay;

    if (!pattbreak) { // next row (only if no manual advance)
        setCurrentRow(currentRow()+1);
        if (currentRow() >= m_patternCells.height()) {
            setCurrentRow(0);
            setCurrentOrder(currentOrder()+1);
        }
    }

    resolveOrder(); // so we can report songend right away
    return !m_songEnd;
}

uint8_t CmodPlayer::setOplChip(uint8_t chan) /*
                         * Sets OPL chip according to channel number. Channels
                         * 0-8 are on first chip,
                         * channels 9-17 are on second chip. Returns
                         * corresponding OPL channel
                         * number.
                         */
{
    return chan % 9;
}

bool CmodPlayer::resolveOrder() /*
                     * Resolves current orderlist entry, checking
                     * for jumps and loops.
                     *
                     * Returns true on correct processing, false if
                     * immediate recursive loop
                     * has been detected.
                     */
{
    if (currentOrder() < orderCount()) {
        while (currentPattern() >= JUMPMARKER) { // jump to order
            const int neword = currentPattern() - JUMPMARKER;

            if (neword <= static_cast<int>(currentOrder()))
                m_songEnd = true;
            else if (neword == static_cast<int>(currentOrder()))
                return false;
            setCurrentOrder(neword);
        }
    }
    else {
        m_songEnd = true;
        setCurrentOrder(m_restartOrder);
    }

    return true;
}

void CmodPlayer::rewind(int) {
    // Reset playing variables
    m_songEnd = false;
    m_patternDelay = m_oplBdRegister = 0;
    setCurrentOrder(0);
    setCurrentRow(0);
    setCurrentSpeed(initialSpeed());
    setCurrentTempo(initialTempo());

    // Reset channel data
    {
        auto nchans = m_channels.size();
        m_channels.clear();
        m_channels.resize(nchans);
    }

    getOpl()->writeReg(1, 32); // Go to ym3812 mode

    // Enable OPL3 extensions if flagged
    if (m_opl3Mode) {
        //FIXME sto opl->setchip(1);
        //FIXME sto opl->writeReg(1, 32);
        //FIXME sto opl->writeReg(5, 1);
        //FIXME sto opl->setchip(0);
    }

    // Enable tremolo/vibrato depth if flagged
    if (m_tremolo)
        m_oplBdRegister |= 128;
    if (m_vibrato)
        m_oplBdRegister |= 64;
    if (m_oplBdRegister)
        getOpl()->writeReg(0xbd, m_oplBdRegister);
}

size_t CmodPlayer::framesUntilUpdate() const
{
    return SampleRate * 2.5 / currentTempo();
}

void CmodPlayer::init_trackord() {
    m_cellColumnMapping.reset(m_cellColumnMapping.width(), m_channels.size());
    for(size_t i = 0; i<m_cellColumnMapping.width(); ++i)
        for(size_t j=0; j<m_channels.size(); ++j)
            m_cellColumnMapping.at(i,j) = i*m_channels.size() + j + 1;
}

void CmodPlayer::init_notetable(const std::array<uint16_t,12>& newnotetable) {
    m_noteTable = newnotetable;
}

void CmodPlayer::realloc_patterns(size_t pats, size_t rows, size_t chans) {
    deallocPatterns();

    m_channels.clear();
    m_channels.resize(chans);

    // alloc new patterns
    m_patternCells.reset(pats*chans, rows);
    m_cellColumnMapping.reset(pats, chans, 0);
}

void CmodPlayer::deallocPatterns() {
    m_patternCells.clear();
    m_cellColumnMapping.clear();
    m_channels.clear();
}

/*** private methods *************************************/

void CmodPlayer::setVolume(uint8_t chan) {
    const auto oplchan = setOplChip(chan);

    if (m_faust) {
        setAverageVolume(chan);
    }
    else {
        getOpl()->writeReg(0x40 + s_opTable[oplchan],
                           63 - m_channels[chan].modulatorVolume +
                           (m_instruments[m_channels[chan].instrument].data[9] & 0xc0));
        getOpl()->writeReg(0x43 + s_opTable[oplchan],
                           63 - m_channels[chan].carrierVolume +
                           (m_instruments[m_channels[chan].instrument].data[10] & 0xc0));
    }
}

void CmodPlayer::setAverageVolume(uint8_t chan) {
    const auto oplchan = setOplChip(chan);

    const auto instrVolumeModulator = m_instruments[m_channels[chan].instrument].data[9] & 0x3f;
    const auto modulatorVolume = 63 - (m_channels[chan].modulatorVolume & 0x3f);
    const auto instrFlagsModulator = m_instruments[m_channels[chan].instrument].data[9] & 0xc0;
    getOpl()->writeReg(0x40 + s_opTable[oplchan],
                       ((modulatorVolume + instrVolumeModulator) >> 1) +
                       instrFlagsModulator);

    const auto instrVolumeCarrier = m_instruments[m_channels[chan].instrument].data[10] & 0x3f;
    const auto carrierVolume = 63 - (m_channels[chan].carrierVolume & 0x3f);
    const auto instrFlagsCarrier = m_instruments[m_channels[chan].instrument].data[10] & 0xc0;
    getOpl()->writeReg(0x43 + s_opTable[oplchan],
                       ((carrierVolume + instrVolumeCarrier) >> 1) +
                       instrFlagsCarrier);
}

void CmodPlayer::setFreq(uint8_t chan) {
    unsigned char oplchan = setOplChip(chan);

    getOpl()->writeReg(0xa0 + oplchan, m_channels[chan].frequency & 0xff);
    if (m_channels[chan].key)
        getOpl()->writeReg(0xb0 + oplchan, ((m_channels[chan].frequency & 0x300) >> 8) | (m_channels[chan].octave << 2) | 0x20);
    else
        getOpl()->writeReg(0xb0 + oplchan, ((m_channels[chan].frequency & 0x300) >> 8) | (m_channels[chan].octave << 2));
}

void CmodPlayer::playNote(uint8_t chan) {
    unsigned char oplchan = setOplChip(chan);

    if (!m_noKeyOn)
        getOpl()->writeReg(0xb0 + oplchan, 0); // stop old note

    // set instrument data
    const auto op = s_opTable[oplchan];
    const auto insnr = m_channels[chan].instrument;
    getOpl()->writeReg(0x20 + op, m_instruments[insnr].data[1]);
    getOpl()->writeReg(0x23 + op, m_instruments[insnr].data[2]);
    getOpl()->writeReg(0x60 + op, m_instruments[insnr].data[3]);
    getOpl()->writeReg(0x63 + op, m_instruments[insnr].data[4]);
    getOpl()->writeReg(0x80 + op, m_instruments[insnr].data[5]);
    getOpl()->writeReg(0x83 + op, m_instruments[insnr].data[6]);
    getOpl()->writeReg(0xe0 + op, m_instruments[insnr].data[7]);
    getOpl()->writeReg(0xe3 + op, m_instruments[insnr].data[8]);
    getOpl()->writeReg(0xc0 + oplchan, m_instruments[insnr].data[0]);
    getOpl()->writeReg(0xbd, m_instruments[insnr].misc); // set misc. register

    // set frequency, volume & play
    m_channels[chan].key = 1;
    setFreq(chan);

    if (m_faust) {
        m_channels[chan].modulatorVolume = 63;
        m_channels[chan].carrierVolume = 63;
    }
    setVolume(chan);
}

void CmodPlayer::setNote(uint8_t chan, int note) {
    if (note > 96) {
        if (note == 127) { // key off
            m_channels[chan].key = 0;
            setFreq(chan);
            return;
        }
        else
            note = 96;
    }

    if (note < 13) {
        BOOST_ASSERT(note > 0);
        m_channels[chan].frequency = m_noteTable[note - 1];
    }
    else if (note % 12 > 0)
        m_channels[chan].frequency = m_noteTable[(note % 12) - 1];
    else
        m_channels[chan].frequency = m_noteTable[11];
    m_channels[chan].octave = (note - 1) / 12;
    m_channels[chan].frequency += m_instruments[m_channels[chan].instrument].slide; // apply pre-slide
}

void CmodPlayer::tonePortamento(uint8_t chan, uint8_t speed) {
    m_channels[chan].porta(speed);
    setFreq(chan);
}

void CmodPlayer::vibrato(uint8_t chan, uint8_t speed, uint8_t depth) {
    if (!speed || !depth)
        return;

    if (depth > 14)
        depth = 14;
    depth = 16-depth;

    for (int i = 0; i < speed; i++) {
        m_channels[chan].trigger = (m_channels[chan].trigger+1) % 64;
        const auto trigger = m_channels[chan].trigger;
        if (trigger >= 16 && trigger < 48)
            m_channels[chan].slideDown(vibratotab[trigger - 16] / depth);
        else if (trigger < 16)
            m_channels[chan].slideUp(vibratotab[trigger + 16] / depth);
        else if (trigger >= 48)
            m_channels[chan].slideUp(vibratotab[trigger - 48] / depth);
    }
    setFreq(chan);
}
