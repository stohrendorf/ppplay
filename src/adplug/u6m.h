#pragma once

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
 * u6m.h - Ultima 6 Music Player by Marc Winterrowd.
 * This code extends the Adlib Winamp plug-in by Simon Peter <dn.tlp@gmx.net>
 */

#include <stack>

#include "player.h"

class U6mPlayer
    : public Player
{
public:
    DISABLE_COPY(U6mPlayer)

    static Player* factory();

    U6mPlayer() = default;

    ~U6mPlayer() override = default;

    bool load(const std::string& filename) override;

    bool update() override;

    void rewind(const boost::optional<size_t>& subsong) override;

    size_t framesUntilUpdate() const override;

    std::string type() const override
    {
        return "Ultima 6 Music";
    }

private:
    struct Nybbles
    {
        uint8_t lo;
        uint8_t hi;
    };

    struct SubsongInfo
    { // information about a subsong
        int continue_pos;
        int repetitions;
        int start;
    };

    struct DictionaryEntry
    { // dictionary entry
        uint8_t root;
        uint32_t codeword;
    };

    using DataBlock = std::vector<uint8_t>;

    class MyDict
    {
    public:
        DISABLE_COPY(MyDict)

        static constexpr int MaxCodewordLength = 12; // maximum codeword length in bits
        static constexpr int DefaultDictionarySize = 1 << MaxCodewordLength;
    private:
        // The actual number of dictionary entries allocated
        // is (dictionary_size-256), because there are 256 roots
        // that do not need to be stored.
        size_t m_contains = 2;  // number of entries currently in the dictionary
        std::array<DictionaryEntry, DefaultDictionarySize - 0x100> m_dictionary{{}};

    public:
        MyDict() = default;    // use dictionary size of 4096
        void reset(); // re-initializes the dictionary
        void add(uint8_t, int);

        uint8_t get_root(uint32_t);

        uint32_t get_codeword(uint32_t);
    };

    std::vector<uint8_t> m_songData{}; // the uncompressed .m file (the "song")
    bool m_driverActive = false;       // flag to prevent reentrancy
    bool m_songEnd = false;             // indicates song end
    int m_songPos = 0;             // current offset within the song
    int m_loopPosition = 0;        // position of the loop point
    int m_readDelay = 0; // delay (in timer ticks) before further song data is read
    std::stack<SubsongInfo> m_subsongStack{};

    int m_instrumentOffsets[9] = {0}; // offsets of the adlib instrument data
    // vibrato ("vb")
    uint8_t m_vbCurrentValue[9] = {0};
    uint8_t m_vbDoubleAmplitude[9] = {0};
    uint8_t m_vbMultiplier[9] = {0};
    bool m_vbDirectionFlag[9] = {false};
    // mute factor ("mf") = not(volume)
    uint8_t m_carrierAttenuation[9] = {0};
    int8_t m_carrierMfSignedDelta[9] = {0};
    uint8_t m_carrierMfModDelayBackup[9] = {0};
    uint8_t m_carrierMfModDelay[9] = {0};
    // frequency
    Nybbles m_channelFreq[9]{}; // adlib freq settings for each channel
    int8_t m_channelFreqSignedDelta[9] = {0};

    // protected functions used by update()
    void command_loop();

    uint8_t nextByte();

    int8_t nextSignedByte();

    static void dec_clip(int&);

    static Nybbles expand_freq_byte(uint8_t);

    void set_adlib_freq(int channel, Nybbles freq_word);

    void set_adlib_freq_no_update(int channel, Nybbles freq_word);

    void setCarrierAttenuation(int channel, uint8_t mute_factor);

    void setModulatorAttenuation(int channel, uint8_t mute_factor);

    void freq_slide(int channel);

    void vibrato(int channel);

    void mf_slide(int channel);

    void stopNote(int channel);

    void restartNote(int channel);

    void playNote(int channel);

    void setCarrierAttenuation(int channel);

    void setModulatorAttenuation(int channel);

    void setPortamentoSpeed(int channel);

    void setVibratoParameters(int channel);

    void setInstrument(int channel);

    void playSubsong();

    void delay();

    void readInstrumentData();

    void setVolSlideUpSpeed();

    void setVolSlideDownSpeed();

    void setLoopStart();

    void returnFromSubsong();

    void writeOperatorRegister(int channel, bool carrier, uint8_t reg, uint8_t val);

    // protected functions used by load()
    static bool lzw_decompress(const DataBlock& source, DataBlock& dest);

    static uint32_t get_next_codeword(long& bits_read, const uint8_t* source, int codeword_size);

    static void output_root(uint8_t root, uint8_t* destination, size_t& position);

    static bool safeOutputRoot(uint8_t c, DataBlock& d, size_t& p)
    {
        if( p >= d.size() )
        {
            return false;
        }
        output_root(c, d.data(), p);
        return true;
    }

    static void get_string(uint32_t codeword, MyDict& dictionary, std::stack<uint8_t>& root_stack);
};
