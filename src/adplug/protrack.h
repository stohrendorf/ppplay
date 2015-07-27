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
 * protrack.h - Generic Protracker Player by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_PROTRACK
#define H_PROTRACK

#include "player.h"
#include "stuff/field.h"

#include <boost/optional.hpp>

class CmodPlayer : public CPlayer {
    DISABLE_COPY(CmodPlayer)
public:
    CmodPlayer();
    virtual ~CmodPlayer() = default;

    bool update();
    void rewind(int);
    size_t framesUntilUpdate() const;

    struct Instrument {
        using Data = std::array<uint8_t, 11>;
        Data data = {{0,0,0,0,0,0,0,0,0,0,0}};
        uint8_t arpeggioStart = 0;
        uint8_t arpeggioSpeed = 0;
        uint8_t arpeggioPos = 0;
        uint8_t misc = 0;
        int8_t slide = 0;
    };

    enum class Command : uint8_t {
        None,
        SlideUp,
        SlideDown,
        Porta,
        Vibrato,
        PortaVolSlide,
        VibVolSlide,
        SetTempo,
        NoteOff,
        SetVolume,
        SA2VolSlide,
        OrderJump,
        SetFineVolume,
        PatternBreak,
        Special,
        SA2Speed,
        AMDVolSlide,
        SetFineVolume2,
        AMDSpeed,
        RADSpeed,
        RADVolSlide,
        ModulatorVolume,
        CarrierVolume,
        FineSlideUp,
        FineSlideDown,
        WaveForm,
        VolSlide,
        OplTremoloVibrato,
        SlideUpDown,
        PatternDelay,


        SFXTremolo,
        SFXVibrato,
        SFXRetrigger,
        SFXFineVolumeUp,
        SFXFineVolumeDown,
        SFXSlideUp,
        SFXSlideDown,
        SFXPatternDelay,

        SFXKeyOff,
        SFXWaveForm,
        SFXVolumeUp,
        SFXVolumeDown,

        Sentinel
    };

    struct PatternCell {
        uint8_t note = 0;
        Command command = Command::None;

        uint8_t instrument = 0;

        uint8_t hiNybble = 0;
        uint8_t loNybble = 0;

        PatternCell() = default;
    };

    const Instrument& instrument(size_t idx) const
    {
        return m_instruments[idx];
    }

protected:
    struct Channel {
        uint16_t frequency = 0;
        uint16_t portaTargetFrequency = 0;
        uint8_t octave = 0;
        uint8_t carrierVolume = 0;
        uint8_t modulatorVolume = 0;
        uint8_t instrument = 0;
        Command fx = Command::None;
        uint8_t hiNybble = 0;
        uint8_t loNybble = 0;
        uint8_t key = 0;
        uint8_t portaTargetOctave = 0;
        uint8_t note = 0;
        uint8_t portaSpeed = 0;
        uint8_t vibratoSpeed = 0;
        uint8_t vibratoDepth = 0;
        uint8_t arppos = 0;
        uint8_t arpspdcnt = 0;
        int8_t trigger = 0;

        void distinctVolumeDown(int amount, const std::vector<Instrument>& instruments)
        {
            carrierVolume = std::max(0, carrierVolume-amount);
            if (instruments[instrument].data[0] & 1) {
                modulatorVolume = std::max(0, modulatorVolume-amount);
            }
        }

        void volumeDown(int amount)
        {
            carrierVolume = std::max(0, carrierVolume-amount);
            modulatorVolume = std::max(0, modulatorVolume-amount);
        }

        void distinctVolumeUp(int amount, const std::vector<Instrument>& instruments)
        {
            carrierVolume = std::min(63, carrierVolume+amount);
            if (instruments[instrument].data[0] & 1) {
                modulatorVolume = std::min(63, modulatorVolume+amount);
            }
        }

        void volumeUp(int amount)
        {
            carrierVolume = std::min(63, carrierVolume+amount);
            modulatorVolume = std::min(63, modulatorVolume+amount);
        }

        void slideDown(int amount)
        {
            frequency -= amount;
            if (frequency <= 342) {
                if (octave) {
                    octave--;
                    frequency <<= 1;
                }
                else {
                    frequency = 342;
                }
            }
        }

        void slideUp(int amount)
        {
            frequency += amount;
            if (frequency >= 686) {
                if (octave < 7) {
                    octave++;
                    frequency >>= 1;
                }
                else {
                    frequency = 686;
                }
            }
        }

        void porta(uint8_t speed)
        {
            if (frequency + (octave << 10) < portaTargetFrequency + (portaTargetOctave << 10)) {
                slideUp(speed);
                if (frequency + (octave << 10) > portaTargetFrequency + (portaTargetOctave << 10)) {
                    frequency = portaTargetFrequency;
                    octave = portaTargetOctave;
                }
            }
            if (frequency + (octave << 10) > portaTargetFrequency + (portaTargetOctave << 10)) {
                slideDown(speed);
                if (frequency + (octave << 10) < portaTargetFrequency + (portaTargetOctave << 10)) {
                    frequency = portaTargetFrequency;
                    octave = portaTargetOctave;
                }
            }
        }
    };

    void init_trackord();
    void init_notetable(const std::array<uint16_t,12> &newnotetable);
    void realloc_patterns(size_t pats, size_t rows, size_t chans);

    Instrument& addInstrument()
    {
        m_instruments.emplace_back();
        return m_instruments.back();
    }

    const PatternCell& patternCell(size_t channel, size_t row) const
    {
        return m_patternCells.at(channel, row);
    }

    PatternCell& patternCell(size_t channel, size_t row)
    {
        return m_patternCells.at(channel, row);
    }

    Channel& channel(size_t idx)
    {
        return m_channels[idx];
    }

    void setRestartOrder(size_t order)
    {
        BOOST_ASSERT(order < orderCount());
        m_restartOrder = order;
    }

    void enableChannel(uint8_t index)
    {
        BOOST_ASSERT(index < 32);
        m_activechan |= (1<<index);
    }

    void disableChannel(uint8_t index)
    {
        BOOST_ASSERT(index < 32);
        m_activechan &= ~(1<<index);
    }

    void disableAllChannels()
    {
        m_activechan = 0;
    }

    void setOpl3Mode()
    {
        m_opl3Mode = true;
    }

    void setTremolo()
    {
        m_tremolo = true;
    }

    void setVibrato()
    {
        m_vibrato = true;
    }

    void setNoKeyOn()
    {
        m_noKeyOn = true;
    }

    void setDecimalValues()
    {
        m_decimalValues = true;
    }

    void setFaust()
    {
        m_faust = true;
    }

    void setCellColumnMapping(size_t pattern, size_t channel, uint16_t column)
    {
        m_cellColumnMapping.at(pattern, channel) = column;
    }

    using ArpeggioData = std::array<uint8_t,256>;

    void setArpeggioList(const ArpeggioData& data)
    {
        m_arpeggioList = data;
    }

    void setArpeggioCommands(const ArpeggioData& data)
    {
        m_arpeggioCommands = data;
    }

private:
    uint8_t m_patternDelay = 0;
    bool m_songEnd = false;
    uint8_t m_oplBdRegister = 0;
    std::array<uint16_t,12> m_noteTable{{0,0,0,0,0,0,0,0,0,0,0,0}};
    std::vector<Instrument> m_instruments{};
    Field<PatternCell> m_patternCells{};
    std::vector<Channel> m_channels{};
    size_t m_restartOrder = 0;
    uint32_t m_activechan = 0xffffffff;
    bool m_decimalValues = false;
    bool m_faust = false;
    bool m_noKeyOn = false;
    bool m_opl3Mode = false;
    bool m_tremolo = false;
    bool m_vibrato = false;
    bool m_percussion = false;
    Field<uint16_t> m_cellColumnMapping{};
    boost::optional<ArpeggioData> m_arpeggioList{};
    boost::optional<ArpeggioData> m_arpeggioCommands{};


    void setVolume(uint8_t chan);
    void setAverageVolume(uint8_t chan);
    void setFreq(uint8_t chan);
    void playNote(uint8_t chan);
    void setNote(uint8_t chan, int note);
    void tonePortamento(uint8_t chan, uint8_t info);
    void vibrato(uint8_t chan, uint8_t speed, uint8_t depth);

    void deallocPatterns();
    bool resolveOrder();
    uint8_t setOplChip(uint8_t chan);
};

#endif
