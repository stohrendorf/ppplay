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

class U6mPlayer : public Player
{
    DISABLE_COPY(U6mPlayer)
public:
    static Player *factory();

    U6mPlayer() = default;

    ~U6mPlayer() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(const boost::optional<size_t>& subsong) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override
    {
        return "Ultima 6 Music";
    }

private:
    struct byte_pair
    {
        uint8_t lo;
        uint8_t hi;
    };

    struct subsong_info
    { // information about a subsong
        int continue_pos;
        int subsong_repetitions;
        int subsong_start;
    };

    struct DictionaryEntry
    { // dictionary entry
        uint8_t root;
        uint32_t codeword;
    };

    using DataBlock = std::vector<uint8_t>;

    class MyDict
    {
        DISABLE_COPY(MyDict)
    public:
        static constexpr int MaxCodewordLength = 12; // maximum codeword length in bits
        static constexpr int DefaultDictionarySize = 1 << MaxCodewordLength;
    private:
        // The actual number of dictionary entries allocated
        // is (dictionary_size-256), because there are 256 roots
        // that do not need to be stored.
        size_t m_contains = 2;  // number of entries currently in the dictionary
        std::array<DictionaryEntry, DefaultDictionarySize - 0x100> m_dictionary{ {} };

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
    std::stack<subsong_info> m_subsongStack{};

    int m_instrumentOffsets[9] = { 0 }; // offsets of the adlib instrument data
    // vibrato ("vb")
    uint8_t m_vbCurrentValue[9] = { 0 };
    uint8_t m_vbDoubleAmplitude[9] = { 0 };
    uint8_t m_vbMultiplier[9] = { 0 };
    bool m_vbDirectionFlag[9] = { false };
    // mute factor ("mf") = not(volume)
    uint8_t m_carrierMf[9] = { 0 };
    int8_t m_carrierMfSignedDelta[9] = { 0 };
    uint8_t m_carrierMfModDelayBackup[9] = { 0 };
    uint8_t m_carrierMfModDelay[9] = { 0 };
    // frequency
    byte_pair m_channelFreq[9]{}; // adlib freq settings for each channel
    int8_t m_channelFreqSignedDelta[9] = { 0 };

    // protected functions used by update()
    void command_loop();
    uint8_t read_song_byte();
    int8_t read_signed_song_byte();
    static void dec_clip(int &);
    static byte_pair expand_freq_byte(uint8_t);
    void set_adlib_freq(int channel, byte_pair freq_word);
    void set_adlib_freq_no_update(int channel, byte_pair freq_word);
    void set_carrier_mf(int channel, uint8_t mute_factor);
    void set_modulator_mf(int channel, uint8_t mute_factor);
    void freq_slide(int channel);
    void vibrato(int channel);
    void mf_slide(int channel);

    void command_0(int channel);
    void command_1(int channel);
    void command_2(int channel);
    void command_3(int channel);
    void command_4(int channel);
    void command_5(int channel);
    void command_6(int channel);
    void command_7(int channel);
    void command_81();
    void command_82();
    void command_83();
    void command_85();
    void command_86();
    void command_E();
    void command_F();

    void out_adlib(uint8_t adlib_register, uint8_t adlib_data);
    void out_adlib_opcell(int channel, bool carrier, uint8_t adlib_register, uint8_t out_byte);

    // protected functions used by load()
    static bool lzw_decompress(const DataBlock& source, DataBlock& dest);
    static uint32_t get_next_codeword(long &bits_read, const uint8_t *source, int codeword_size);
    static void output_root(uint8_t root, uint8_t *destination, size_t& position);

    static bool safeOutputRoot(uint8_t c, DataBlock& d, size_t& p)
    {
        if(p >= d.size())
            return false;
        output_root(c, d.data(), p);
        return true;
    }

    static void get_string(uint32_t codeword, MyDict &dictionary, std::stack<uint8_t> &root_stack);
};
