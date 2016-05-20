#pragma once

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * rol.h - ROL Player by OPLx <oplx@yahoo.com>
 *
 * Visit:  http://tenacity.hispeed.com/aomit/oplx/
 */

#include <vector>

#include <boost/algorithm/string.hpp>

#include "player.h"

class FileStream;

class RolPlayer : public Player
{
    DISABLE_COPY(RolPlayer)
public:
    static Player *factory();

    RolPlayer() = default;

    ~RolPlayer() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(const boost::optional<size_t>& subsong) override;           // rewinds to specified subsong
    size_t framesUntilUpdate() const override; // returns needed timer refresh rate

    std::string type() const override
    {
        return "Adlib Visual Composer";
    }

private:
    struct Header
    {
        uint16_t version_major = 0;
        uint16_t version_minor = 0;
        uint8_t UNUSED_0[40] = "";
        uint16_t ticks_per_beat = 0;
        uint16_t beats_per_measure = 0;
        uint16_t edit_scale_y = 0;
        uint16_t edit_scale_x = 0;
        uint8_t UNUSED_1 = 0;
        uint8_t mode = 0;
        uint8_t UNUSED_2[90 + 38 + 15] = "";
        float basic_tempo = 0;
    };

#pragma pack(push,1)
    struct TempoEvent
    {
        int16_t time;
        float multiplier;
    };

    struct NoteEvent
    {
        int16_t number;
        int16_t duration;
    };

    struct InstrumentEvent
    {
        int16_t time;
        char name[9];
        int16_t ins_index;
    };

    struct VolumeEvent
    {
        int16_t time;
        float multiplier;
    };

    struct PitchEvent
    {
        int16_t time;
        float variation;
    };
#pragma pack(pop)

    using NoteEvents = std::vector<NoteEvent>;
    using InstrumentEvents = std::vector<InstrumentEvent>;
    using VolumeEvents = std::vector<VolumeEvent>;
    using PitchEvents = std::vector<PitchEvent>;

    class VoiceData
    {
    public:
        enum EventStatus : uint8_t
        {
            None = 0,
            NoteEnd = 1 << 0,
            PitchEnd = 1 << 1,
            InstrEnd = 1 << 2,
            VolumeEnd = 1 << 3
        };

        explicit VoiceData() = default;

        void Reset()
        {
            mForceNote = true;
            mEventStatus = EventStatus::None;
            current_note = 0;
            current_note_duration = 0;
            mNoteDuration = 0;
            next_instrument_event = 0;
            next_volume_event = 0;
            next_pitch_event = 0;
        }

        NoteEvents note_events{};
        InstrumentEvents instrument_events{};
        VolumeEvents volume_events{};
        PitchEvents pitch_events{};

        bool mForceNote = true;
        uint8_t mEventStatus = EventStatus::None;
        unsigned int current_note = 0;
        int current_note_duration = 0;
        int mNoteDuration = 0;
        unsigned int next_instrument_event = 0;
        unsigned int next_volume_event = 0;
        unsigned int next_pitch_event = 0;
    };

#pragma pack(push,1)
    struct InstrumentName
    {
        uint16_t index;
        char record_used;
        char name[9];
    };
#pragma pack(pop)

    typedef std::vector<InstrumentName> InstrumentNames;

    struct BnkHeader
    {
        char version_major = 0;
        char version_minor = 0;
        char signature[6] = "";
        uint16_t number_of_list_entries_used = 0;
        uint16_t total_number_of_list_entries = 0;
        int32_t abs_offset_of_name_list = 0;
        int32_t abs_offset_of_data = 0;

        InstrumentNames ins_name_list{};
    };

#pragma pack(push,1)
    struct FMOperator
    {
        uint8_t key_scale_level;
        uint8_t freq_multiplier;
        uint8_t feed_back;
        uint8_t attack_rate;
        uint8_t sustain_level;
        uint8_t sustaining_sound;
        uint8_t decay_rate;
        uint8_t release_rate;
        uint8_t output_level;
        uint8_t amplitude_vibrato;
        uint8_t frequency_vibrato;
        uint8_t envelope_scaling;
        uint8_t fm_type;
    };
#pragma pack(pop)

    struct OPL2Op
    {
        uint8_t ammulti = 0;
        uint8_t ksltl = 0;
        uint8_t ardr = 0;
        uint8_t slrr = 0;
        uint8_t fbc = 0;
        uint8_t waveform = 0;
    };

    struct RolInstrument
    {
        char mode = 0;
        char voice_number = 0;
        OPL2Op modulator{};
        OPL2Op carrier{};
    };

    struct UsedList
    {
        std::string name{};
        RolInstrument instrument{};
    };

    void load_tempo_events(FileStream& f);
    bool load_voice_data(FileStream& f, const std::string &bnk_filename);
    void load_note_events(FileStream& f, VoiceData &voice);
    void load_instrument_events(FileStream& f, VoiceData &voice,
                                FileStream& bnk_file,
                                const BnkHeader &bnk_header);
    static void load_volume_events(FileStream& f, VoiceData &voice);
    static void load_pitch_events(FileStream& f, VoiceData &voice);

    static bool load_bnk_info(FileStream& f, BnkHeader &header);
    int load_rol_instrument(FileStream& f, const BnkHeader &header,
                            std::string &name);
    static void read_rol_instrument(FileStream& f, RolInstrument &ins);
    static void read_fm_operator(FileStream& f, OPL2Op &opl2_op);
    int get_ins_index(const std::string &name) const;

    void UpdateVoice(int voice, VoiceData &voiceData);
    void SetNote(int voice, int note);
    void SetNoteMelodic(int voice, int note);
    void SetNotePercussive(int voice, int note);
    void SetFreq(int voice, int note, bool keyOn = false);
    void SetPitch(int voice, float variation);
    void SetVolume(int voice, int volume);
    void SetRefresh(float multiplier);
    void send_ins_data_to_chip(int voice, int ins_index);
    void send_operator(int voice, const OPL2Op &modulator,
                       const OPL2Op &carrier);

    class StringCompare
    {
    public:
        bool operator()(const InstrumentName &lhs,
                        const InstrumentName &rhs) const
        {
            return keyLess(lhs.name, rhs.name);
        }

        bool operator()(const InstrumentName &lhs, const std::string &rhs) const
        {
            return keyLess(lhs.name, rhs.c_str());
        }

        bool operator()(const std::string &lhs, const InstrumentName &rhs) const
        {
            return keyLess(lhs.c_str(), rhs.name);
        }

    private:
        static bool keyLess(const char* lhs, const char* rhs)
        {
            return boost::algorithm::ilexicographical_compare(lhs, rhs);
        }
    };

    Header m_rolHeader{};
    std::vector<TempoEvent> m_tempoEvents{};
    std::vector<VoiceData> m_voiceData{};
    std::vector<UsedList> m_instrumentList{};

    static constexpr float DefaultUpdateTme = 18.2f;

    uint32_t m_nextTempoEvent = 0;
    int m_currTick = 0;
    int m_timeOfLastNote = 0;
    float m_refresh = DefaultUpdateTme;
    uint8_t m_bdRegister = 0;
    uint8_t m_bxRegister[9] = { 0,0,0,0,0,0,0,0,0 };
    uint8_t m_volumeCache[11] = { 0,0,0,0,0,0,0,0,0,0,0 };
    uint16_t m_freqCache[11] = { 0,0,0,0,0,0,0,0,0,0,0 };
    float m_pitchCache[11] = { 1,1,1,1,1,1,1,1,1,1,1 };
};
