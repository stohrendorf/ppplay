/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.
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
#include <cstring>
#include <algorithm>

#include "stream/filestream.h"

#include "rol.h"
#include "debug.h"

int const CrolPlayer::kSizeofDataRecord = 30;
int const CrolPlayer::kMaxTickBeat = 60;
int const CrolPlayer::kSilenceNote = -12;
int const CrolPlayer::kNumMelodicVoices = 9;
int const CrolPlayer::kNumPercussiveVoices = 11;
int const CrolPlayer::kBassDrumChannel = 6;
int const CrolPlayer::kSnareDrumChannel = 7;
int const CrolPlayer::kTomtomChannel = 8;
int const CrolPlayer::kTomtomFreq = 2;    //4;
int const CrolPlayer::kSnareDrumFreq = 2; //kTomtomFreq + 7;
float const CrolPlayer::kDefaultUpdateTme = 18.2f;
float const CrolPlayer::kPitchFactor = 400.0f;

static const unsigned char drum_table[4] = { 0x14, 0x12, 0x15, 0x11 };

CrolPlayer::uint16_t const CrolPlayer::kNoteTable[12] = { 340, // C
                                                          363, // C#
                                                          385, // D
                                                          408, // D#
                                                          432, // E
                                                          458, // F
                                                          485, // F#
                                                          514, // G
                                                          544, // G#
                                                          577, // A
                                                          611, // A#
                                                          647  // B
                                                        };

/*** public methods **************************************/

CPlayer *CrolPlayer::factory() { return new CrolPlayer(); }
//---------------------------------------------------------
bool CrolPlayer::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;

    char *fn = new char[filename.length() + 12];
    int i;
    std::string bnk_filename;

    AdPlug_LogWrite("*** CrolPlayer::load(f, \"%s\") ***\n", filename.c_str());
    strcpy(fn, filename.data());
    for (i = strlen(fn) - 1; i >= 0; i--)
        if (fn[i] == '/' || fn[i] == '\\')
            break;
    strcpy(fn + i + 1, "standard.bnk");
    bnk_filename = fn;
    delete[] fn;
    AdPlug_LogWrite("bnk_filename = \"%s\"\n", bnk_filename.c_str());

    f >> rol_header.version_major;
    f >> rol_header.version_minor;

    // Version check
    if (rol_header.version_major != 0 || rol_header.version_minor != 4) {
        AdPlug_LogWrite("Unsupported file version %d.%d or not a ROL file!\n",
                        rol_header.version_major, rol_header.version_minor);
        AdPlug_LogWrite("--- CrolPlayer::load ---\n");
        return false;
    }

    f.seekrel(40);

    f >> rol_header.ticks_per_beat;
    f >> rol_header.beats_per_measure;
    f >> rol_header.edit_scale_y;
    f >> rol_header.edit_scale_x;

    f.seekrel(1);

    f >> rol_header.mode;

    f.seekrel(90 + 38 + 15);

    f >> rol_header.basic_tempo;

    load_tempo_events(f);

    mTimeOfLastNote = 0;

    if (load_voice_data(f, bnk_filename) != true) {
        AdPlug_LogWrite("CrolPlayer::load_voice_data(f) failed!\n");
        AdPlug_LogWrite("--- CrolPlayer::load ---\n");

        return false;
    }

    rewind(0);
    AdPlug_LogWrite("--- CrolPlayer::load ---\n");
    return true;
}
//---------------------------------------------------------
bool CrolPlayer::update() {
    if (mNextTempoEvent < mTempoEvents.size() &&
            mTempoEvents[mNextTempoEvent].time == mCurrTick) {
        SetRefresh(mTempoEvents[mNextTempoEvent].multiplier);
        ++mNextTempoEvent;
    }

    TVoiceData::iterator curr = voice_data.begin();
    TVoiceData::iterator end = voice_data.end();
    int voice = 0;

    while (curr != end) {
        UpdateVoice(voice, *curr);
        ++curr;
        ++voice;
    }

    ++mCurrTick;

    if (mCurrTick > mTimeOfLastNote) {
        return false;
    }

    return true;
    //return ( mCurrTick > mTimeOfLastNote ) ? false : true;
}
//---------------------------------------------------------
void CrolPlayer::rewind(int) {
    TVoiceData::iterator curr = voice_data.begin();
    TVoiceData::iterator end = voice_data.end();

    while (curr != end) {
        CVoiceData &voice = *curr;

        voice.Reset();
        ++curr;
    }

    memset(bxRegister, 0, sizeof(bxRegister));
    memset(volumeCache, 0, sizeof(volumeCache));

    bdRegister = 0;

    getOpl()->writeReg(1, 0x20); // Enable waveform select (bit 5)

    if (rol_header.mode == 0) {
        getOpl()->writeReg(0xbd, 0x20); // select rhythm mode (bit 5)
        bdRegister = 0x20;

        SetFreq(kTomtomChannel, 24);
        SetFreq(kSnareDrumChannel, 31);
    }

    mNextTempoEvent = 0;
    mCurrTick = 0;

    SetRefresh(1.0f);
}
//---------------------------------------------------------
void CrolPlayer::SetRefresh(float const multiplier) {
    float const tickBeat = fmin(kMaxTickBeat, rol_header.ticks_per_beat);

    mRefresh = (tickBeat * rol_header.basic_tempo * multiplier) / 60.0f;
}
//---------------------------------------------------------
size_t CrolPlayer::framesUntilUpdate() { return SampleRate / mRefresh; }
//---------------------------------------------------------
void CrolPlayer::UpdateVoice(int const voice, CVoiceData &voiceData) {
    TNoteEvents const &nEvents = voiceData.note_events;

    if (nEvents.empty() || voiceData.mEventStatus & CVoiceData::kES_NoteEnd) {
        return; // no note data to process, don't bother doing anything.
    }

    TInstrumentEvents &iEvents = voiceData.instrument_events;
    TVolumeEvents &vEvents = voiceData.volume_events;
    TPitchEvents &pEvents = voiceData.pitch_events;

    if (!(voiceData.mEventStatus & CVoiceData::kES_InstrEnd) &&
            iEvents[voiceData.next_instrument_event].time == mCurrTick) {
        if (voiceData.next_instrument_event < iEvents.size()) {
            send_ins_data_to_chip(voice,
                                  iEvents[voiceData.next_instrument_event].ins_index);
            ++voiceData.next_instrument_event;
        } else {
            voiceData.mEventStatus |= CVoiceData::kES_InstrEnd;
        }
    }

    if (!(voiceData.mEventStatus & CVoiceData::kES_VolumeEnd) &&
            vEvents[voiceData.next_volume_event].time == mCurrTick) {
        SVolumeEvent const &volumeEvent = vEvents[voiceData.next_volume_event];

        if (voiceData.next_volume_event < vEvents.size()) {
            int const volume = static_cast<int>(63.0f * (1.0f - volumeEvent.multiplier));

            SetVolume(voice, volume);

            ++voiceData.next_volume_event; // move to next volume event
        } else {
            voiceData.mEventStatus |= CVoiceData::kES_VolumeEnd;
        }
    }

    if (voiceData.mForceNote ||
            voiceData.current_note_duration > voiceData.mNoteDuration - 1) {
        if (mCurrTick != 0) {
            ++voiceData.current_note;
        }

        if (voiceData.current_note < nEvents.size()) {
            SNoteEvent const &noteEvent = nEvents[voiceData.current_note];

            SetNote(voice, noteEvent.number);
            voiceData.current_note_duration = 0;
            voiceData.mNoteDuration = noteEvent.duration;
            voiceData.mForceNote = false;
        } else {
            SetNote(voice, kSilenceNote);
            voiceData.mEventStatus |= CVoiceData::kES_NoteEnd;
            return;
        }
    }

    if (!(voiceData.mEventStatus & CVoiceData::kES_PitchEnd) &&
            pEvents[voiceData.next_pitch_event].time == mCurrTick) {
        if (voiceData.next_pitch_event < pEvents.size()) {
            SetPitch(voice, pEvents[voiceData.next_pitch_event].variation);
            ++voiceData.next_pitch_event;
        } else {
            voiceData.mEventStatus |= CVoiceData::kES_PitchEnd;
        }
    }

    ++voiceData.current_note_duration;
}
//---------------------------------------------------------
void CrolPlayer::SetNote(int const voice, int const note) {
    if (voice < kBassDrumChannel || rol_header.mode) {
        SetNoteMelodic(voice, note);
    } else {
        SetNotePercussive(voice, note);
    }
}
//---------------------------------------------------------
void CrolPlayer::SetNotePercussive(int const voice, int const note) {
    int const bit_pos = 4 - voice + kBassDrumChannel;

    bdRegister &= ~(1 << bit_pos);
    getOpl()->writeReg(0xbd, bdRegister);

    if (note != kSilenceNote) {
        switch (voice) {
        case kTomtomChannel:
            SetFreq(kSnareDrumChannel, note + 7);
        case kBassDrumChannel:
            SetFreq(voice, note);
            break;
        }

        bdRegister |= 1 << bit_pos;
        getOpl()->writeReg(0xbd, bdRegister);
    }
}
//---------------------------------------------------------
void CrolPlayer::SetNoteMelodic(int const voice, int const note) {
    getOpl()->writeReg(0xb0 + voice, bxRegister[voice] & ~0x20);

    if (note != kSilenceNote) {
        SetFreq(voice, note, true);
    }
}
//---------------------------------------------------------
void CrolPlayer::SetPitch(int const voice, float const variation) {
    pitchCache[voice] = variation;
    freqCache[voice] += static_cast<uint16_t>( (freqCache[voice] * (variation - 1.0f)) / kPitchFactor );

    getOpl()->writeReg(0xa0 + voice, freqCache[voice] & 0xff);
}
//---------------------------------------------------------
void CrolPlayer::SetFreq(int const voice, int const note, bool const keyOn) {
    uint16_t freq = kNoteTable[note % 12] + ((note / 12) << 10);
    freq += static_cast<uint16_t>((freq * (pitchCache[voice] - 1.0f)) / kPitchFactor);

    freqCache[voice] = freq;
    bxRegister[voice] = ((freq >> 8) & 0x1f);

    getOpl()->writeReg(0xa0 + voice, freq & 0xff);
    getOpl()->writeReg(0xb0 + voice, bxRegister[voice] | (keyOn ? 0x20 : 0x0));
}
//---------------------------------------------------------
void CrolPlayer::SetVolume(int const voice, int const volume) {
    volumeCache[voice] = (volumeCache[voice] & 0xc0) | volume;

    int const op_offset = (voice < kSnareDrumChannel || rol_header.mode)
            ? s_opTable[voice] + 3
            : drum_table[voice - kSnareDrumChannel];

    getOpl()->writeReg(0x40 + op_offset, volumeCache[voice]);
}
//---------------------------------------------------------
void CrolPlayer::send_ins_data_to_chip(int const voice, int const ins_index) {
    SRolInstrument &instrument = ins_list[ins_index].instrument;

    send_operator(voice, instrument.modulator, instrument.carrier);
}
//---------------------------------------------------------
void CrolPlayer::send_operator(int const voice, SOPL2Op const &modulator,
                               SOPL2Op const &carrier) {
    if (voice < kSnareDrumChannel || rol_header.mode) {
        int const op_offset = s_opTable[voice];

        getOpl()->writeReg(0x20 + op_offset, modulator.ammulti);
        getOpl()->writeReg(0x40 + op_offset, modulator.ksltl);
        getOpl()->writeReg(0x60 + op_offset, modulator.ardr);
        getOpl()->writeReg(0x80 + op_offset, modulator.slrr);
        getOpl()->writeReg(0xc0 + voice, modulator.fbc);
        getOpl()->writeReg(0xe0 + op_offset, modulator.waveform);

        volumeCache[voice] = (carrier.ksltl & 0xc0) | (volumeCache[voice] & 0x3f);

        getOpl()->writeReg(0x23 + op_offset, carrier.ammulti);
        getOpl()->writeReg(0x43 + op_offset, volumeCache[voice]);
        getOpl()->writeReg(0x63 + op_offset, carrier.ardr);
        getOpl()->writeReg(0x83 + op_offset, carrier.slrr);
        //        opl->writeReg( 0xc3+voice    , carrier.fbc      ); <- don't bother
        // writing this.
        getOpl()->writeReg(0xe3 + op_offset, carrier.waveform);
    } else {
        int const op_offset = drum_table[voice - kSnareDrumChannel];

        volumeCache[voice] = (modulator.ksltl & 0xc0) | (volumeCache[voice] & 0x3f);

        getOpl()->writeReg(0x20 + op_offset, modulator.ammulti);
        getOpl()->writeReg(0x40 + op_offset, volumeCache[voice]);
        getOpl()->writeReg(0x60 + op_offset, modulator.ardr);
        getOpl()->writeReg(0x80 + op_offset, modulator.slrr);
        getOpl()->writeReg(0xc0 + voice, modulator.fbc);
        getOpl()->writeReg(0xe0 + op_offset, modulator.waveform);
    }
}
//---------------------------------------------------------
void CrolPlayer::load_tempo_events(FileStream& f) {
    int16_t num_tempo_events;
    f >> num_tempo_events;

    mTempoEvents.reserve(num_tempo_events);

    for (int i = 0; i < num_tempo_events; ++i) {
        STempoEvent event;
        f >> event;
        mTempoEvents.push_back(event);
    }
}
//---------------------------------------------------------
bool CrolPlayer::load_voice_data(FileStream& f, std::string const &bnk_filename) {
    SBnkHeader bnk_header;
    FileStream bnk_file(bnk_filename);

    if (bnk_file) {
        load_bnk_info(bnk_file, bnk_header);

        int const numVoices = rol_header.mode ? kNumMelodicVoices : kNumPercussiveVoices;

        voice_data.reserve(numVoices);
        for (int i = 0; i < numVoices; ++i) {
            CVoiceData voice;

            load_note_events(f, voice);
            load_instrument_events(f, voice, bnk_file, bnk_header);
            load_volume_events(f, voice);
            load_pitch_events(f, voice);

            voice_data.push_back(voice);
        }

        return true;
    }

    return false;
}
//---------------------------------------------------------
void CrolPlayer::load_note_events(FileStream& f, CVoiceData &voice) {
    f.seekrel(15);

    int16_t time_of_last_note;
    f >> time_of_last_note;

    if (time_of_last_note != 0) {
        TNoteEvents &note_events = voice.note_events;
        int16_t total_duration = 0;

        do {
            SNoteEvent event;
            f >> event;

            event.number += kSilenceNote; // adding -12

            note_events.push_back(event);

            total_duration += event.duration;
        } while (total_duration < time_of_last_note);

        if (time_of_last_note > mTimeOfLastNote) {
            mTimeOfLastNote = time_of_last_note;
        }
    }

    f.seekrel(15);
}
//---------------------------------------------------------
void CrolPlayer::load_instrument_events(FileStream& f, CVoiceData &voice,
                                        FileStream& bnk_file,
                                        SBnkHeader const &bnk_header) {
    int16_t number_of_instrument_events;
    f >> number_of_instrument_events;

    TInstrumentEvents &instrument_events = voice.instrument_events;

    instrument_events.reserve(number_of_instrument_events);

    for (int i = 0; i < number_of_instrument_events; ++i) {
        SInstrumentEvent event;
        f >> event.time;
        f.read(event.name, 9);

        std::string event_name = event.name;
        event.ins_index = load_rol_instrument(bnk_file, bnk_header, event_name);

        instrument_events.push_back(event);

        f.seekrel(1 + 2);
    }

    f.seekrel(15);
}
//---------------------------------------------------------
void CrolPlayer::load_volume_events(FileStream& f, CVoiceData &voice) {
    int16_t number_of_volume_events;
    f >> number_of_volume_events;

    TVolumeEvents &volume_events = voice.volume_events;

    volume_events.reserve(number_of_volume_events);

    for (int i = 0; i < number_of_volume_events; ++i) {
        SVolumeEvent event;
        f >> event;
        volume_events.push_back(event);
    }

    f.seekrel(15);
}
//---------------------------------------------------------
void CrolPlayer::load_pitch_events(FileStream& f, CVoiceData &voice) {
    int16_t number_of_pitch_events;
    f >> number_of_pitch_events;

    TPitchEvents &pitch_events = voice.pitch_events;

    pitch_events.reserve(number_of_pitch_events);

    for (int i = 0; i < number_of_pitch_events; ++i) {
        SPitchEvent event;
        f >> event;

        pitch_events.push_back(event);
    }
}
//---------------------------------------------------------
bool CrolPlayer::load_bnk_info(FileStream& f, SBnkHeader &header) {
    f >> header.version_major;
    f >> header.version_minor;
    f.read(header.signature, 6);

    f >> header.number_of_list_entries_used;
    f >> header.total_number_of_list_entries;

    f >> header.abs_offset_of_name_list;
    f >> header.abs_offset_of_data;

    f.seek(header.abs_offset_of_name_list);

    TInstrumentNames &ins_name_list = header.ins_name_list;
    ins_name_list.reserve(header.number_of_list_entries_used);

    for (int i = 0; i < header.number_of_list_entries_used; ++i) {
        SInstrumentName instrument;
        f >> instrument;

        // printf("%s = #%d\n", instrument.name, i );

        ins_name_list.push_back(instrument);
    }

    //std::sort( ins_name_list.begin(), ins_name_list.end(), StringCompare() );

    return true;
}
//---------------------------------------------------------
int CrolPlayer::load_rol_instrument(FileStream& f, SBnkHeader const &header,
                                    std::string &name) {
    TInstrumentNames const &ins_name_list = header.ins_name_list;

    int const ins_index = get_ins_index(name);

    if (ins_index != -1) {
        return ins_index;
    }

    typedef TInstrumentNames::const_iterator TInsIter;
    typedef std::pair<TInsIter, TInsIter> TInsIterPair;

    TInsIterPair range = std::equal_range(
                ins_name_list.begin(), ins_name_list.end(), name, StringCompare());

    if (range.first != range.second) {
        int const seekOffs = header.abs_offset_of_data + (range.first->index * kSizeofDataRecord);
        f.seek(seekOffs);
    }

    SUsedList usedIns;
    usedIns.name = name;

    if (range.first != range.second) {
        read_rol_instrument(f, usedIns.instrument);
    } else {
        // set up default instrument data here
        memset(&usedIns.instrument, 0, sizeof(SRolInstrument));
    }
    ins_list.push_back(usedIns);

    return ins_list.size() - 1;
}
//---------------------------------------------------------
int CrolPlayer::get_ins_index(std::string const &name) const {
    for (unsigned int i = 0; i < ins_list.size(); ++i) {
        if (strcasecmp(ins_list[i].name.c_str(), name.c_str()) == 0) {
            return i;
        }
    }

    return -1;
}
//---------------------------------------------------------
void CrolPlayer::read_rol_instrument(FileStream& f, SRolInstrument &ins) {
    f >> ins.mode;
    f >> ins.voice_number;

    read_fm_operator(f, ins.modulator);
    read_fm_operator(f, ins.carrier);

    f >> ins.modulator.waveform;
    f >> ins.carrier.waveform;
}
//---------------------------------------------------------
void CrolPlayer::read_fm_operator(FileStream& f, SOPL2Op &opl2_op) {
    SFMOperator fm_op;
    f >> fm_op;

    opl2_op.ammulti = fm_op.amplitude_vibrato << 7 |
                                                 fm_op.frequency_vibrato << 6 | fm_op.sustaining_sound << 5 |
                                                 fm_op.envelope_scaling << 4 | fm_op.freq_multiplier;
    opl2_op.ksltl = fm_op.key_scale_level << 6 | fm_op.output_level;
    opl2_op.ardr = fm_op.attack_rate << 4 | fm_op.decay_rate;
    opl2_op.slrr = fm_op.sustain_level << 4 | fm_op.release_rate;
    opl2_op.fbc = fm_op.feed_back << 1 | (fm_op.fm_type ^ 1);
}
