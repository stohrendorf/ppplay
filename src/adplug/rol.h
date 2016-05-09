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
#include <string>

#include <boost/algorithm/string.hpp>

#include "player.h"

class FileStream;

class CrolPlayer : public Player
{
    DISABLE_COPY(CrolPlayer)
public:
    static Player *factory();

    CrolPlayer() = default;

    ~CrolPlayer() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(int) override;           // rewinds to specified subsong
    size_t framesUntilUpdate() const override; // returns needed timer refresh rate

    std::string type() const override
    {
        return "Adlib Visual Composer";
    }

private:
    struct SRolHeader
    {
        uint16_t version_major = 0;
        uint16_t version_minor = 0;
        char UNUSED_0[40] = "";
        uint16_t ticks_per_beat = 0;
        uint16_t beats_per_measure = 0;
        uint16_t edit_scale_y = 0;
        uint16_t edit_scale_x = 0;
        char UNUSED_1 = 0;
        char mode = 0;
        char UNUSED_2[90 + 38 + 15] = "";
        float basic_tempo = 0;
    };

#pragma pack(push,1)
    struct STempoEvent
    {
        int16_t time;
        float multiplier;
    };

    struct SNoteEvent
    {
        int16_t number;
        int16_t duration;
    };

    struct SInstrumentEvent
    {
        int16_t time;
        char name[9];
        int16_t ins_index;
    };

    struct SVolumeEvent
    {
        int16_t time;
        float multiplier;
    };

    struct SPitchEvent
    {
        int16_t time;
        float variation;
    };
#pragma pack(pop)

    typedef std::vector<SNoteEvent> TNoteEvents;
    typedef std::vector<SInstrumentEvent> TInstrumentEvents;
    typedef std::vector<SVolumeEvent> TVolumeEvents;
    typedef std::vector<SPitchEvent> TPitchEvents;

    class CVoiceData
    {
    public:
        enum EEventStatus
        {
            kES_NoteEnd = 1 << 0,
            kES_PitchEnd = 1 << 1,
            kES_InstrEnd = 1 << 2,
            kES_VolumeEnd = 1 << 3,

            kES_None = 0
        };

        explicit CVoiceData() = default;

        void Reset()
        {
            mForceNote = true;
            mEventStatus = kES_None;
            current_note = 0;
            current_note_duration = 0;
            mNoteDuration = 0;
            next_instrument_event = 0;
            next_volume_event = 0;
            next_pitch_event = 0;
        }

        TNoteEvents note_events{};
        TInstrumentEvents instrument_events{};
        TVolumeEvents volume_events{};
        TPitchEvents pitch_events{};

        bool mForceNote = true;
        int mEventStatus = kES_None;
        unsigned int current_note = 0;
        int current_note_duration = 0;
        int mNoteDuration = 0;
        unsigned int next_instrument_event = 0;
        unsigned int next_volume_event = 0;
        unsigned int next_pitch_event = 0;
    };

#pragma pack(push,1)
    struct SInstrumentName
    {
        uint16_t index;
        char record_used;
        char name[9];
    };
#pragma pack(pop)

    typedef std::vector<SInstrumentName> TInstrumentNames;

    struct SBnkHeader
    {
        char version_major = 0;
        char version_minor = 0;
        char signature[6] = "";
        uint16_t number_of_list_entries_used = 0;
        uint16_t total_number_of_list_entries = 0;
        int32_t abs_offset_of_name_list = 0;
        int32_t abs_offset_of_data = 0;

        TInstrumentNames ins_name_list{};
    };

#pragma pack(push,1)
    struct SFMOperator
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

    struct SOPL2Op
    {
        uint8_t ammulti = 0;
        uint8_t ksltl = 0;
        uint8_t ardr = 0;
        uint8_t slrr = 0;
        uint8_t fbc = 0;
        uint8_t waveform = 0;
    };

    struct SRolInstrument
    {
        char mode = 0;
        char voice_number = 0;
        SOPL2Op modulator{};
        SOPL2Op carrier{};
    };

    struct SUsedList
    {
        std::string name{};
        SRolInstrument instrument{};
    };

    void load_tempo_events(FileStream& f);
    bool load_voice_data(FileStream& f, std::string const &bnk_filename);
    void load_note_events(FileStream& f, CVoiceData &voice);
    void load_instrument_events(FileStream& f, CVoiceData &voice,
                                FileStream& bnk_file,
                                SBnkHeader const &bnk_header);
    static void load_volume_events(FileStream& f, CVoiceData &voice);
    static void load_pitch_events(FileStream& f, CVoiceData &voice);

    static bool load_bnk_info(FileStream& f, SBnkHeader &header);
    int load_rol_instrument(FileStream& f, SBnkHeader const &header,
                            std::string &name);
    static void read_rol_instrument(FileStream& f, SRolInstrument &ins);
    static void read_fm_operator(FileStream& f, SOPL2Op &opl2_op);
    int get_ins_index(std::string const &name) const;

    void UpdateVoice(int const voice, CVoiceData &voiceData);
    void SetNote(int const voice, int const note);
    void SetNoteMelodic(int const voice, int const note);
    void SetNotePercussive(int const voice, int const note);
    void SetFreq(int const voice, int const note, bool const keyOn = false);
    void SetPitch(int const voice, float const variation);
    void SetVolume(int const voice, int const volume);
    void SetRefresh(float const multiplier);
    void send_ins_data_to_chip(int const voice, int const ins_index);
    void send_operator(int const voice, SOPL2Op const &modulator,
                       SOPL2Op const &carrier);

    class StringCompare
    {
    public:
        bool operator()(SInstrumentName const &lhs,
                        SInstrumentName const &rhs) const
        {
            return keyLess(lhs.name, rhs.name);
        }

        bool operator()(SInstrumentName const &lhs, std::string const &rhs) const
        {
            return keyLess(lhs.name, rhs.c_str());
        }

        bool operator()(std::string const &lhs, SInstrumentName const &rhs) const
        {
            return keyLess(lhs.c_str(), rhs.name);
        }

    private:
        bool keyLess(const char *const lhs, const char *const rhs) const
        {
            return boost::algorithm::ilexicographical_compare(lhs, rhs);
        }
    };

    typedef std::vector<CVoiceData> TVoiceData;

    SRolHeader m_rolHeader{};
    std::vector<STempoEvent> m_tempoEvents{};
    TVoiceData m_voiceData{};
    std::vector<SUsedList> m_instrumentList{};

    static constexpr float kDefaultUpdateTme = 18.2f;

    uint32_t m_nextTempoEvent = 0;
    int m_currTick = 0;
    int m_timeOfLastNote = 0;
    float m_refresh = kDefaultUpdateTme;
    uint8_t m_bdRegister = 0;
    uint8_t m_bxRegister[9] = { 0,0,0,0,0,0,0,0,0 };
    uint8_t m_volumeCache[11] = { 0,0,0,0,0,0,0,0,0,0,0 };
    uint16_t m_freqCache[11] = { 0,0,0,0,0,0,0,0,0,0,0 };
    float m_pitchCache[11] = { 1,1,1,1,1,1,1,1,1,1,1 };
};
