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
 * mid.h - LAA, SCI, MID & CMF Player by Philip Hassey <philhassey@hotmail.com>
 */

#include "player.h"
#include "mid/almidi.h"

class MidPlayer
    : public Player
{
public:
    DISABLE_COPY(MidPlayer)

    MidPlayer() = default;

    static Player* factory();

    bool load(const std::string& filename) override;

    bool update() override;

    void rewind(const boost::optional<size_t>& subsong) override;

    size_t framesUntilUpdate() const override;

    std::string type() const override;

    std::string title() const override
    {
        return m_title;
    }

    std::string author() const override
    {
        return m_author;
    }

    std::string description() const override
    {
        return m_remarks;
    }

    size_t instrumentCount() const override
    {
        return m_tins;
    }

    size_t subSongCount() const override
    {
        return m_subsongs;
    }

private:
    static const unsigned char adlib_opadd[];
    static const int percussion_map[];

    struct midi_channel
    {
        int inum = 0;
        unsigned char ins[11]{0};
        int vol = 0;
        int nshift = 0;
        int on = 0;
    };

    struct midi_track
    {
        size_t tend = 0;
        size_t spos = 0;
        size_t pos = 0;
        unsigned long iwait = 0;
        int on = 0;
        unsigned char pv = 0;
    };

    std::string m_author{};
    std::string m_title{};
    std::string m_remarks{};
    size_t m_dataPos = 0;
    size_t m_sierraPos = 0; //sierras gotta be special.. :>
    size_t m_subsongs = 0;
    std::vector<uint8_t> m_data{};

    int m_adlibStyle = 0;
    bool m_melodicMode = true;
    using InsBank = std::array<std::array<uint8_t, 14>, 128>;
    InsBank m_myInsBank{{}}, m_sMyInsBank{{}};
    midi_channel m_ch[16]{};
    int m_chp[18][3]{{0}};

    long m_deltas = 0;
    long m_msqtr = 0;

    midi_track m_tracks[16]{};
    unsigned int m_currentTrack = 0;

    float m_fwait = 0;
    unsigned long m_iwait = 0;
    bool m_doing = false;

    enum class FileType
    {
        Unknown, Lucas, Midi, Cmf, Sierra, AdvSierra, OldLucas
    };

    FileType m_type = FileType::Unknown;
    int m_stins = 0;
    size_t m_tins = 0;

    bool load_sierra_ins(const std::string& fname);

    uint8_t datalook(size_t m_dataPos) const;

    uint32_t getnexti(size_t num);

    uint32_t getnext(size_t num);

    uint32_t getval();

    void sierra_next_section();

    void midi_fm_instrument(int voice, unsigned char* inst);

    void midi_fm_percussion(int m_ch, unsigned char* inst);

    void midi_fm_volume(int voice, int volume);

    void midi_fm_playnote(int voice, int note, int volume);

    void midi_fm_endnote(int voice);

    void midi_fm_reset();
};

class DukePlayer
    : Player
{
private:
    std::unique_ptr<ppp::EMidi> m_emidi = nullptr;

public:
    static Player* factory()
    {
        return new DukePlayer();
    }

    bool load(const std::string& filename) override;

    bool update() override
    {
        return m_emidi->serviceRoutine();
    }

    void rewind(const boost::optional<size_t>&) override
    {
    }

    size_t framesUntilUpdate() const override
    {
        return SampleRate / m_emidi->ticksPerSecond();
    }

    std::string type() const override
    {
        std::string fmt = m_emidi->shortFormatName();
        return "Duke MIDI (" + fmt + ")";
    }

    std::string title() const override
    {
        return std::string();
    }

    std::string author() const override
    {
        return std::string();
    }

    std::string description() const override
    {
        return std::string();
    }

    size_t instrumentCount() const override
    {
        return 0;
    }

    size_t subSongCount() const override
    {
        return 1;
    }

    void read(std::array<int16_t, 4>* data) override
    {
        m_emidi->read(data);
    }
};
