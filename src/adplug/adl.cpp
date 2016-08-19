/*
 * adl.cpp - ADL player adaption by Simon Peter <dn.tlp@gmx.net>
 *
 * Original ADL player by Torbjorn Andersson and Johannes Schickel
 * 'lordhoto' <lordhoto at scummvm dot org> of the ScummVM project.
 */

/* ScummVM - Scumm Interpreter
 *
 * This file is licensed under both GPL and LGPL
 * Copyright (C) 2006 The ScummVM project
 * Copyright (C) 2006 Torbjorn Andersson and Johannes Schickel
 *
 * GPL License
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 * LPGL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 *
 * $URL:
 * https://svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/kyra/sound_adlib.cpp
 * $
 * $Id: adl.cpp,v 1.11 2008/02/11 20:18:27 dynamite Exp $
 *
 */

#include "stream/filestream.h"

#include "adl.h"

#include <algorithm>

namespace
{
    // This table holds the register offset for operator 1 for each of the nine
    // channels. To get the register offset for operator 2, simply add 3.

    const uint8_t operatorRegisterOffsets[] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12};

    // Given the size of this table, and the range of its values, it's probably the
    // F-Numbers (10 bits) for the notes of the 12-tone scale. However, it does not
    // match the table in the Adlib documentation I've seen.

    const uint16_t _unkTable[] = {0x0134, 0x0147, 0x015A, 0x016F,
        0x0184, 0x019C, 0x01B4, 0x01CE,
        0x01E9, 0x0207, 0x0225, 0x0246};

    // These tables are currently only used by updateCallback46(), which only ever
    // uses the first element of one of the sub-tables.

    const uint8_t _unkTable2_1[] = {
        0x50, 0x50, 0x4F, 0x4F, 0x4E, 0x4E, 0x4D, 0x4D, 0x4C, 0x4C, 0x4B, 0x4B, 0x4A,
        0x4A, 0x49, 0x49, 0x48, 0x48, 0x47, 0x47, 0x46, 0x46, 0x45, 0x45, 0x44, 0x44,
        0x43, 0x43, 0x42, 0x42, 0x41, 0x41, 0x40, 0x40, 0x3F, 0x3F, 0x3E, 0x3E, 0x3D,
        0x3D, 0x3C, 0x3C, 0x3B, 0x3B, 0x3A, 0x3A, 0x39, 0x39, 0x38, 0x38, 0x37, 0x37,
        0x36, 0x36, 0x35, 0x35, 0x34, 0x34, 0x33, 0x33, 0x32, 0x32, 0x31, 0x31, 0x30,
        0x30, 0x2F, 0x2F, 0x2E, 0x2E, 0x2D, 0x2D, 0x2C, 0x2C, 0x2B, 0x2B, 0x2A, 0x2A,
        0x29, 0x29, 0x28, 0x28, 0x27, 0x27, 0x26, 0x26, 0x25, 0x25, 0x24, 0x24, 0x23,
        0x23, 0x22, 0x22, 0x21, 0x21, 0x20, 0x20, 0x1F, 0x1F, 0x1E, 0x1E, 0x1D, 0x1D,
        0x1C, 0x1C, 0x1B, 0x1B, 0x1A, 0x1A, 0x19, 0x19, 0x18, 0x18, 0x17, 0x17, 0x16,
        0x16, 0x15, 0x15, 0x14, 0x14, 0x13, 0x13, 0x12, 0x12, 0x11, 0x11, 0x10, 0x10
    };

    // no don't ask me WHY this table exsits!
    const uint8_t _unkTable2_2[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
        0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
        0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D,
        0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
        0x5B, 0x5C, 0x5D, 0x5E, 0x6F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
        0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F
    };

    const uint8_t _unkTable2_3[] = {
        0x40, 0x40, 0x40, 0x3F, 0x3F, 0x3F, 0x3E, 0x3E, 0x3E, 0x3D, 0x3D, 0x3D, 0x3C,
        0x3C, 0x3C, 0x3B, 0x3B, 0x3B, 0x3A, 0x3A, 0x3A, 0x39, 0x39, 0x39, 0x38, 0x38,
        0x38, 0x37, 0x37, 0x37, 0x36, 0x36, 0x36, 0x35, 0x35, 0x35, 0x34, 0x34, 0x34,
        0x33, 0x33, 0x33, 0x32, 0x32, 0x32, 0x31, 0x31, 0x31, 0x30, 0x30, 0x30, 0x2F,
        0x2F, 0x2F, 0x2E, 0x2E, 0x2E, 0x2D, 0x2D, 0x2D, 0x2C, 0x2C, 0x2C, 0x2B, 0x2B,
        0x2B, 0x2A, 0x2A, 0x2A, 0x29, 0x29, 0x29, 0x28, 0x28, 0x28, 0x27, 0x27, 0x27,
        0x26, 0x26, 0x26, 0x25, 0x25, 0x25, 0x24, 0x24, 0x24, 0x23, 0x23, 0x23, 0x22,
        0x22, 0x22, 0x21, 0x21, 0x21, 0x20, 0x20, 0x20, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E,
        0x1E, 0x1D, 0x1D, 0x1D, 0x1C, 0x1C, 0x1C, 0x1B, 0x1B, 0x1B, 0x1A, 0x1A, 0x1A,
        0x19, 0x19, 0x19, 0x18, 0x18, 0x18, 0x17, 0x17, 0x17, 0x16, 0x16, 0x16, 0x15
    };

    // This table is used to modify the frequency of the notes, depending on the
    // note value and unk16. In theory, we could very well try to access memory
    // outside this table, but in reality that probably won't happen.
    //
    // This could be some sort of pitch bend, but I have yet to see it used for
    // anything so it's hard to say.

    const std::array<std::array<uint8_t, 32>, 14> _unkTables{{
        // 0
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
            0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x19,
            0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21},
        // 1
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
            0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x1A,
            0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x22, 0x24},
        // 2
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x09, 0x0A, 0x0C, 0x0D, 0x0E,
            0x0F, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1D,
            0x1E, 0x1F, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26},
        // 3
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
            0x0F, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x1A, 0x1C, 0x1D,
            0x1E, 0x1F, 0x20, 0x21, 0x23, 0x25, 0x27, 0x28},
        // 4
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
            0x0F, 0x11, 0x13, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1B, 0x1D, 0x1F, 0x20,
            0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x2A},
        // 5
        {0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x13, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1B, 0x1D, 0x1F, 0x20,
            0x21, 0x22, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D},
        // 6
        {0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x13, 0x15, 0x16, 0x17, 0x18, 0x1A, 0x1C, 0x1E, 0x21, 0x24,
            0x25, 0x26, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x30},
        // 7
        {0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
            0x11, 0x13, 0x15, 0x18, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x21, 0x23, 0x25,
            0x26, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x30, 0x32},
        // 8
        {0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
            0x12, 0x14, 0x17, 0x1A, 0x19, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x25, 0x28,
            0x29, 0x2A, 0x2B, 0x2D, 0x2F, 0x31, 0x33, 0x35},
        // 9
        {0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0E, 0x0F, 0x10, 0x12, 0x14,
            0x16, 0x18, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x20, 0x22, 0x24, 0x26, 0x29,
            0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x39},
        // 10
        {0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0E, 0x0F, 0x10, 0x12, 0x14,
            0x16, 0x19, 0x1B, 0x1E, 0x1F, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D,
            0x2E, 0x2F, 0x31, 0x32, 0x34, 0x36, 0x39, 0x3C},
        // 11
        {0x00, 0x01, 0x03, 0x05, 0x07, 0x0A, 0x0C, 0x0F, 0x10, 0x11, 0x13, 0x15,
            0x17, 0x19, 0x1B, 0x1E, 0x1F, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2B, 0x2E,
            0x2F, 0x30, 0x32, 0x34, 0x36, 0x39, 0x3C, 0x3F},
        // 12
        {0x00, 0x02, 0x04, 0x06, 0x08, 0x0B, 0x0D, 0x10, 0x11, 0x12, 0x14, 0x16,
            0x18, 0x1B, 0x1E, 0x21, 0x22, 0x23, 0x25, 0x27, 0x29, 0x2C, 0x2F, 0x32,
            0x33, 0x34, 0x36, 0x38, 0x3B, 0x34, 0x41, 0x44},
        // 13
        {0x00, 0x02, 0x04, 0x06, 0x08, 0x0B, 0x0D, 0x11, 0x12, 0x13, 0x15, 0x17,
            0x1A, 0x1D, 0x20, 0x23, 0x24, 0x25, 0x27, 0x29, 0x2C, 0x2F, 0x32, 0x35,
            0x36, 0x37, 0x39, 0x3B, 0x3E, 0x41, 0x44, 0x47}
    }};

    const uint8_t* _unkTable2[] = {
        _unkTable2_1, _unkTable2_2,
        _unkTable2_1, _unkTable2_2,
        _unkTable2_3, _unkTable2_2
    };
}

// Basic Adlib Programming:
// http://www.gamedev.net/reference/articles/article446.asp

namespace
{
    inline uint16_t READ_LE_UINT16(const uint8_t* b)
    {
        return (b[1] << 8) + b[0];
    }

    inline uint16_t READ_BE_UINT16(const uint8_t* b)
    {
        return (b[0] << 8) + b[1];
    }
}

class AdlibDriver
{
    DISABLE_COPY(AdlibDriver)
public:
    AdlibDriver() = default;
    ~AdlibDriver() = default;

    void callback();

    void snd_initDriver();
    void snd_setSoundData(const uint8_t* data);
    void snd_startSong(int songId);
    void snd_unkOpcode3(int a);
    uint8_t snd_readByte(int progId, int ofs) const;
    void snd_writeByte(int progId, int ofs, uint8_t val);
    uint8_t snd_getSoundTrigger() const;
    void snd_setFlag(int flg);

    // These variables have not yet been named, but some of them are partly
    // known nevertheless:
    //
    // unk16 - Sound-related. Possibly some sort of pitch bend.
    // unk18 - Sound-effect. Used for secondaryEffect1()
    // unk19 - Sound-effect. Used for secondaryEffect1()
    // unk20 - Sound-effect. Used for secondaryEffect1()
    // unk21 - Sound-effect. Used for secondaryEffect1()
    // unk22 - Sound-effect. Used for secondaryEffect1()
    // unk29 - Sound-effect. Used for primaryEffect1()
    // unk30 - Sound-effect. Used for primaryEffect1()
    // unk31 - Sound-effect. Used for primaryEffect1()
    // unk32 - Sound-effect. Used for primaryEffect2()
    // unk33 - Sound-effect. Used for primaryEffect2()
    // unk34 - Sound-effect. Used for primaryEffect2()
    // unk35 - Sound-effect. Used for primaryEffect2()
    // unk36 - Sound-effect. Used for primaryEffect2()
    // unk37 - Sound-effect. Used for primaryEffect2()
    // unk38 - Sound-effect. Used for primaryEffect2()
    // unk39 - Currently unused, except for updateCallback56()
    // unk40 - Currently unused, except for updateCallback56()
    // unk41 - Sound-effect. Used for primaryEffect2()

    struct Channel
    {
        uint8_t opExtraLevel2 = 0;
        const uint8_t* dataptr = nullptr;
        uint8_t duration = 0;
        uint8_t repeatCounter = 0;
        int8_t baseOctave = 0;
        uint8_t priority = 0;
        uint8_t dataptrStackPos = 0;
        const uint8_t* dataptrStack[4] = {nullptr, nullptr, nullptr, nullptr};
        int8_t baseNote = 0;
        uint8_t unk29 = 0;
        uint8_t unk31 = 0;
        uint16_t unk30 = 0;
        int16_t vibratoSpeed = 0;
        uint8_t unk33 = 0;
        uint8_t vibratoCountdown = 0;
        uint8_t vibratoPeriod = 0;
        uint8_t unk36 = 0;
        uint8_t unk32 = 0;
        uint8_t unk41 = 0;
        uint8_t unk38 = 0;
        uint8_t opExtraLevel1 = 0;
        uint8_t spacing2 = 0;
        uint8_t baseFreq = 0;
        uint8_t tempo = 0;
        uint8_t position = 0;
        uint8_t fnumL = 0;
        uint8_t konBlockFnumH = 0;

        typedef void (AdlibDriver::*Callback)(Channel&);

        Callback primaryEffect = nullptr;
        Callback secondaryEffect = nullptr;
        uint8_t fractionalSpacing = 0;
        uint8_t opLevel1 = 0;
        uint8_t opLevel2 = 0;
        uint8_t opExtraLevel3 = 0;
        bool cnt = false;
        uint8_t unk39 = 0;
        uint8_t unk40 = 0;
        uint8_t spacing1 = 0;
        uint8_t durationRandomness = 0;
        uint8_t unk19 = 0;
        uint8_t unk18 = 0;
        int8_t unk20 = 0;
        int8_t rawRegisterDataOffset = 0;
        uint8_t rawRegisterOffset = 0;
        uint16_t offset = 0;
        uint8_t tempoReset = 0;
        uint8_t rawNote = 0;
        int8_t finetune = 0;
    };

    void primaryEffect1(Channel& channel);
    void primaryEffect2(Channel& channel);
    void secondaryEffect1(Channel& channel);

    void resetAdlibState();
    void writeOPL(uint8_t reg, uint8_t val);
    static void initChannel(Channel& channel);
    void noteOff(Channel& channel);
    void unkOutput2(uint8_t num);

    uint16_t getRandomNr();
    void setupDuration(uint8_t duration, Channel& channel);

    void setupNote(uint8_t rawNote, Channel& channel, bool flag = false);
    void setupInstrument(uint8_t regOffset, const uint8_t* dataptr, Channel& channel);
    void noteOn(Channel& channel);

    void adjustVolume(Channel& channel);

    static uint8_t calculateOpLevel1(Channel& channel);
    static uint8_t calculateOpLevel2(Channel& channel);

    static uint8_t clampTotalLevel(int16_t val)
    {
        if( val < 0 )
            return 0;
        else if( val > 0x3F )
            return 0x3F;
        return static_cast<uint8_t>(val);
    }

    // The sound data has at least two lookup tables:
    //
    // * One for programs, starting at offset 0.
    // * One for instruments, starting at offset 500.

    const uint8_t* getProgram(int progId) const
    {
        return m_soundData + READ_LE_UINT16(m_soundData + 2 * progId);
    }

    const uint8_t* getInstrument(int instrumentId) const
    {
        return m_soundData + READ_LE_UINT16(m_soundData + 500 + 2 * instrumentId);
    }

    void setupPrograms();
    void executePrograms();

    struct ParserOpcode
    {
        typedef int (AdlibDriver::*POpcode)(const uint8_t*& dataptr, Channel& channel,
                                            uint8_t value);
        POpcode function;
        const char* name;
    };

    int update_setRepeat(const uint8_t*&, Channel& channel, uint8_t value);
    int update_checkRepeat(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int update_setupProgram(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setNoteSpacing(const uint8_t*&, Channel& channel, uint8_t value);
    int update_jump(const uint8_t*& dataptr, Channel&, uint8_t);
    int update_jumpToSubroutine(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int update_returnFromSubroutine(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int update_setBaseOctave(const uint8_t*&, Channel& channel, uint8_t value);
    int update_stopChannel(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int update_playRest(const uint8_t*&, Channel& channel, uint8_t value);
    int update_writeAdlib(const uint8_t*& dataptr, Channel&, uint8_t value);
    int update_setupNoteAndDuration(const uint8_t*& dataptr, Channel& channel,
                                    uint8_t value);
    int update_setBaseNote(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setupSecondaryEffect1(const uint8_t*& dataptr, Channel& channel,
                                     uint8_t value);
    int update_stopOtherChannel(const uint8_t*&, Channel& channel, uint8_t value);
    int update_waitForEndOfProgram(const uint8_t*& dataptr, Channel&, uint8_t value);
    int update_setupInstrument(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setupPrimaryEffect1(const uint8_t*& dataptr, Channel& channel,
                                   uint8_t value);
    int update_removePrimaryEffect1(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int update_setBaseFreq(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setupPrimaryEffect2(const uint8_t*& dataptr, Channel& channel,
                                   uint8_t value);
    int update_setPriority(const uint8_t*&, Channel& channel, uint8_t value);
    int updateCallback23(const uint8_t*&, Channel&, uint8_t value);
    int updateCallback24(const uint8_t*& dataptr, Channel& channel, uint8_t value);
    int update_setExtraLevel1(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setupDuration(const uint8_t*&, Channel& channel, uint8_t value);
    int update_playNote(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setFractionalNoteSpacing(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setTempo(const uint8_t*&, Channel&, uint8_t value);
    int update_removeSecondaryEffect1(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int update_setChannelTempo(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setExtraLevel3(const uint8_t*&, Channel& channel, uint8_t value);
    int update_setExtraLevel2(const uint8_t*& dataptr, Channel&, uint8_t value);
    int update_changeExtraLevel2(const uint8_t*& dataptr, Channel&, uint8_t value);
    int update_setAMDepth(const uint8_t*&, Channel&, uint8_t value);
    int update_setVibratoDepth(const uint8_t*&, Channel&, uint8_t value);
    int update_changeExtraLevel1(const uint8_t*&, Channel& channel, uint8_t value);
    int updateCallback38(const uint8_t*&, Channel&, uint8_t value);
    int updateCallback39(const uint8_t*& dataptr, Channel& channel, uint8_t value);
    int update_removePrimaryEffect2(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int setFinetune(const uint8_t*&, Channel& channel, uint8_t value);
    int update_resetToGlobalTempo(const uint8_t*& dataptr, Channel& channel, uint8_t);
    int update_nop1(const uint8_t*& dataptr, Channel&, uint8_t);
    int update_setDurationRandomness(const uint8_t*&, Channel& channel, uint8_t value);
    int update_changeChannelTempo(const uint8_t*&, Channel& channel, uint8_t value);
    int updateCallback46(const uint8_t*& dataptr, Channel&, uint8_t value);
    int update_nop2(const uint8_t*& dataptr, Channel&, uint8_t);
    int update_setupRhythmSection(const uint8_t*& dataptr, Channel& channel, uint8_t value);
    int update_playRhythmSection(const uint8_t*&, Channel&, uint8_t value);
    int update_removeRhythmSection(const uint8_t*& dataptr, Channel&, uint8_t value);
    int updateCallback51(const uint8_t*& dataptr, Channel&, uint8_t value);
    int updateCallback52(const uint8_t*& dataptr, Channel&, uint8_t value);
    int updateCallback53(const uint8_t*& dataptr, Channel&, uint8_t value);
    int update_setSoundTrigger(const uint8_t*&, Channel&, uint8_t value);
    int update_setTempoReset(const uint8_t*&, Channel& channel, uint8_t value);
    int updateCallback56(const uint8_t*& dataptr, Channel& channel, uint8_t value);

    // These variables have not yet been named, but some of them are partly
    // known nevertheless:
    //
    // _unkValue1      - Unknown. Used for updating _unkValue2
    // _unkValue2      - Unknown. Used for updating _unkValue4
    // _unkValue3      - Unknown. Used for updating _unkValue2
    // _unkValue4      - Unknown. Used for updating _unkValue5
    // _unkValue5      - Unknown. Used for controlling updateCallback24().
    // _unkValue6      - Unknown. Rhythm section volume?
    // _unkValue7      - Unknown. Rhythm section volume?
    // _unkValue8      - Unknown. Rhythm section volume?
    // _unkValue9      - Unknown. Rhythm section volume?
    // _unkValue10     - Unknown. Rhythm section volume?
    // _unkValue11     - Unknown. Rhythm section volume?
    // _unkValue12     - Unknown. Rhythm section volume?
    // _unkValue13     - Unknown. Rhythm section volume?
    // _unkValue14     - Unknown. Rhythm section volume?
    // _unkValue15     - Unknown. Rhythm section volume?
    // _unkValue16     - Unknown. Rhythm section volume?
    // _unkValue17     - Unknown. Rhythm section volume?
    // _unkValue18     - Unknown. Rhythm section volume?
    // _unkValue19     - Unknown. Rhythm section volume?
    // _unkValue20     - Unknown. Rhythm section volume?
    // _unkTable[]     - Probably frequences for the 12-tone scale.
    // _unkTable2[]    - Unknown. Currently only used by updateCallback46()
    // _unkTable2_1[]  - One of the tables in _unkTable2[]
    // _unkTable2_2[]  - One of the tables in _unkTable2[]
    // _unkTable2_3[]  - One of the tables in _unkTable2[]

    int32_t _samplesPerCallback = 0;
    int32_t _samplesPerCallbackRemainder = 0;
    int32_t _samplesTillCallback = 0;
    int32_t _samplesTillCallbackRemainder = 0;

    int _lastProcessed = 0;
    int8_t m_flagTrigger = 0;
    int m_oplChannel = 0;
    uint8_t _soundTrigger = 0;
    int _soundsPlaying = 0;

    uint16_t _rnd = 0x1234;

    uint8_t _unkValue1 = 0;
    uint8_t _unkValue2 = 0;
    uint8_t _unkValue3 = 0xFF;
    uint8_t _unkValue4 = 0;
    uint8_t _unkValue5 = 0;
    uint8_t _unkValue6 = 0;
    uint8_t _unkValue7 = 0;
    uint8_t _unkValue8 = 0;
    uint8_t _unkValue9 = 0;
    uint8_t _unkValue10 = 0;
    uint8_t _unkValue11 = 0;
    uint8_t _unkValue12 = 0;
    uint8_t _unkValue13 = 0;
    uint8_t _unkValue14 = 0;
    uint8_t _unkValue15 = 0;
    uint8_t _unkValue16 = 0;
    uint8_t _unkValue17 = 0;
    uint8_t _unkValue18 = 0;
    uint8_t _unkValue19 = 0;
    uint8_t _unkValue20 = 0;

    int m_flags = 0;

    const uint8_t* m_soundData = nullptr;

    uint8_t _soundIdTable[0x10];
    std::array<Channel, 10> m_channels{{}};

    uint8_t m_vibratoAndAMDepthBits = 0;
    uint8_t _rhythmSectionBits = 0;

    uint8_t m_operatorRegisterOffset = 0;
    uint8_t m_tempo = 0;

    const uint8_t* _tablePtr1 = nullptr;
    const uint8_t* _tablePtr2 = nullptr;

    opl::Opl3 m_opl{};
};

// Opcodes

void AdlibDriver::snd_initDriver()
{
    _lastProcessed = _soundsPlaying = 0;
    resetAdlibState();
}

void AdlibDriver::snd_setSoundData(const uint8_t* data)
{
    m_soundData = data;
}

void AdlibDriver::snd_startSong(int songId)
{
    m_flags |= 8;
    m_flagTrigger = 1;

    const uint8_t* ptr = getProgram(songId);
    uint8_t chan = *ptr;

    if( (songId << 1) != 0 )
    {
        if( chan == 9 )
        {
            if( m_flags & 2 )
                return;
        }
        else
        {
            if( m_flags & 1 )
                return;
        }
    }

    _soundIdTable[_soundsPlaying++] = songId;
    _soundsPlaying &= 0x0F;
}

void AdlibDriver::snd_unkOpcode3(int value)
{
    int loop = value;
    if( value < 0 )
    {
        value = 0;
        loop = 9;
    }
    loop -= value;
    ++loop;

    while( loop-- )
    {
        m_oplChannel = value;
        Channel& channel = m_channels[m_oplChannel];
        channel.priority = 0;
        channel.dataptr = nullptr;
        if( value != 9 )
        {
            noteOff(channel);
        }
        ++value;
    }
}

uint8_t AdlibDriver::snd_readByte(int progId, int ofs) const
{
    return getProgram(progId)[ofs];
}

void AdlibDriver::snd_writeByte(int progId, int ofs, uint8_t val)
{
    const_cast<uint8_t&>(getProgram(progId)[ofs]) = val;
}

uint8_t AdlibDriver::snd_getSoundTrigger() const
{
    return _soundTrigger;
}

void AdlibDriver::snd_setFlag(int flg)
{
    m_flags |= flg;
}

// timer callback

void AdlibDriver::callback()
{
    // 	lock();
    --m_flagTrigger;
    if( m_flagTrigger < 0 )
        m_flags &= ~8;
    setupPrograms();
    executePrograms();

    uint8_t temp = _unkValue3;
    _unkValue3 += m_tempo;
    if( _unkValue3 < temp )
    {
        if( !(--_unkValue2) )
        {
            _unkValue2 = _unkValue1;
            ++_unkValue4;
        }
    }
    // 	unlock();
}

void AdlibDriver::setupPrograms()
{
    while( _lastProcessed != _soundsPlaying )
    {
        const uint8_t* ptr = getProgram(_soundIdTable[_lastProcessed]);
        uint8_t chan = *ptr++;
        uint8_t priority = *ptr++;

        // Only start this sound if its priority is higher than the one
        // already playing.

        Channel& channel = m_channels[chan];

        if( priority >= channel.priority )
        {
            initChannel(channel);
            channel.priority = priority;
            channel.dataptr = ptr;
            channel.tempo = 0xFF;
            channel.position = 0xFF;
            channel.duration = 1;
            unkOutput2(chan);
        }

        ++_lastProcessed;
        _lastProcessed &= 0x0F;
    }
}

// A few words on opcode parsing and timing:
//
// First of all, We simulate a timer callback 72 times per second. Each timeout
// we update each channel that has something to play.
//
// Each channel has its own individual tempo, which is added to its position.
// This will frequently cause the position to "wrap around" but that is
// intentional. In fact, it's the signal to go ahead and do more stuff with
// that channel.
//
// Each channel also has a duration, indicating how much time is left on the
// its current task. This duration is decreased by one. As long as it still has
// not reached zero, the only thing that can happen is that the note is turned
// off depending on manual or automatic note spacing. Once the duration reaches
// zero, a new set of musical opcodes are executed.
//
// An opcode is one byte, followed by a variable number of parameters. Since
// most opcodes have at least one one-byte parameter, we read that as well. Any
// opcode that doesn't have that one parameter is responsible for moving the
// data pointer back again.
//
// If the most significant bit of the opcode is 1, it's a function; call it.
// The opcode functions return either 0 (continue), 1 (stop) or 2 (stop, and do
// not run the effects callbacks).
//
// If the most significant bit of the opcode is 0, it's a note, and the first
// parameter is its duration. (There are cases where the duration is modified
// but that's an exception.) The note opcode is assumed to return 1, and is the
// last opcode unless its duration is zero.
//
// Finally, most of the times that the callback is called, it will invoke the
// effects callbacks. The final opcode in a set can prevent this, if it's a
// function and it returns anything other than 1.

void AdlibDriver::executePrograms()
{
    // Each channel runs its own program. There are ten channels: One for
    // each Adlib channel (0-8), plus one "control channel" (9) which is
    // the one that tells the other channels what to do.

    for( m_oplChannel = 9; m_oplChannel >= 0; --m_oplChannel )
    {
        int result = 1;

        if( !m_channels[m_oplChannel].dataptr )
        {
            continue;
        }

        Channel& channel = m_channels[m_oplChannel];
        m_operatorRegisterOffset = operatorRegisterOffsets[m_oplChannel];

        if( channel.tempoReset )
        {
            channel.tempo = m_tempo;
        }

        uint8_t backup = channel.position;
        channel.position += channel.tempo;
        if( channel.position < backup )
        {
            if( --channel.duration )
            {
                if( channel.duration == channel.spacing2 )
                    noteOff(channel);
                if( channel.duration == channel.spacing1 && m_oplChannel != 9 )
                    noteOff(channel);
            }
            else
            {
#define COMMAND(x)                                                             \
                { &AdlibDriver::x, #x }
                static const std::array<ParserOpcode, 75> parserOpcodeTable{
                    {
                        // 0
                        COMMAND(update_setRepeat), COMMAND(update_checkRepeat),
                        COMMAND(update_setupProgram), COMMAND(update_setNoteSpacing),

                        // 4
                        COMMAND(update_jump), COMMAND(update_jumpToSubroutine),
                        COMMAND(update_returnFromSubroutine),
                        COMMAND(update_setBaseOctave),

                        // 8
                        COMMAND(update_stopChannel), COMMAND(update_playRest),
                        COMMAND(update_writeAdlib),
                        COMMAND(update_setupNoteAndDuration),

                        // 12
                        COMMAND(update_setBaseNote),
                        COMMAND(update_setupSecondaryEffect1),
                        COMMAND(update_stopOtherChannel),
                        COMMAND(update_waitForEndOfProgram),

                        // 16
                        COMMAND(update_setupInstrument),
                        COMMAND(update_setupPrimaryEffect1),
                        COMMAND(update_removePrimaryEffect1),
                        COMMAND(update_setBaseFreq),

                        // 20
                        COMMAND(update_stopChannel),
                        COMMAND(update_setupPrimaryEffect2),
                        COMMAND(update_stopChannel), COMMAND(update_stopChannel),

                        // 24
                        COMMAND(update_stopChannel), COMMAND(update_stopChannel),
                        COMMAND(update_setPriority), COMMAND(update_stopChannel),

                        // 28
                        COMMAND(updateCallback23), COMMAND(updateCallback24),
                        COMMAND(update_setExtraLevel1), COMMAND(update_stopChannel),

                        // 32
                        COMMAND(update_setupDuration), COMMAND(update_playNote),
                        COMMAND(update_stopChannel), COMMAND(update_stopChannel),

                        // 36
                        COMMAND(update_setFractionalNoteSpacing),
                        COMMAND(update_stopChannel), COMMAND(update_setTempo),
                        COMMAND(update_removeSecondaryEffect1),

                        // 40
                        COMMAND(update_stopChannel), COMMAND(update_setChannelTempo),
                        COMMAND(update_stopChannel), COMMAND(update_setExtraLevel3),

                        // 44
                        COMMAND(update_setExtraLevel2),
                        COMMAND(update_changeExtraLevel2), COMMAND(update_setAMDepth),
                        COMMAND(update_setVibratoDepth),

                        // 48
                        COMMAND(update_changeExtraLevel1), COMMAND(update_stopChannel),
                        COMMAND(update_stopChannel), COMMAND(updateCallback38),

                        // 52
                        COMMAND(update_stopChannel), COMMAND(updateCallback39),
                        COMMAND(update_removePrimaryEffect2),
                        COMMAND(update_stopChannel),

                        // 56
                        COMMAND(update_stopChannel), COMMAND(setFinetune),
                        COMMAND(update_resetToGlobalTempo), COMMAND(update_nop1),

                        // 60
                        COMMAND(update_setDurationRandomness),
                        COMMAND(update_changeChannelTempo), COMMAND(update_stopChannel),
                        COMMAND(updateCallback46),

                        // 64
                        COMMAND(update_nop2), COMMAND(update_setupRhythmSection),
                        COMMAND(update_playRhythmSection),
                        COMMAND(update_removeRhythmSection),

                        // 68
                        COMMAND(updateCallback51), COMMAND(updateCallback52),
                        COMMAND(updateCallback53), COMMAND(update_setSoundTrigger),

                        // 72
                        COMMAND(update_setTempoReset), COMMAND(updateCallback56),
                        COMMAND(update_stopChannel)
#undef COMMAND
                    }
                };

                // An opcode is not allowed to modify its own
                // data pointer except through the 'dataptr'
                // parameter. To enforce that, we have to work
                // on a copy of the data pointer.
                //
                // This fixes a subtle music bug where the
                // wrong music would play when getting the
                // quill in Kyra 1.
                const uint8_t* dataptr = channel.dataptr;
                while( dataptr )
                {
                    uint8_t opcode = *dataptr++;
                    uint8_t param = *dataptr++;

                    if( opcode & 0x80 )
                    {
                        opcode &= 0x7F;
                        if( opcode >= parserOpcodeTable.size() )
                            opcode = static_cast<uint8_t>(parserOpcodeTable.size() - 1);
                        result = (this ->* (parserOpcodeTable[opcode].function))(dataptr, channel, param);
                        channel.dataptr = dataptr;
                        if( result )
                            break;
                    }
                    else
                    {
                        setupNote(opcode, channel);
                        noteOn(channel);
                        setupDuration(param, channel);
                        if( param )
                        {
                            channel.dataptr = dataptr;
                            break;
                        }
                    }
                }
            }
        }

        if( result == 1 )
        {
            if( channel.primaryEffect )
                (this ->* (channel.primaryEffect))(channel);
            if( channel.secondaryEffect )
                (this ->* (channel.secondaryEffect))(channel);
        }
    }
}

//

void AdlibDriver::resetAdlibState()
{
    _rnd = 0x1234;

    // Authorize the control of the waveforms
    writeOPL(0x01, 0x20);

    // Select FM music mode
    writeOPL(0x08, 0x00);

    // I would guess the main purpose of this is to turn off the rhythm,
    // thus allowing us to use 9 melodic voices instead of 6.
    writeOPL(0xBD, 0x00);

    int loop = 10;
    while( loop-- )
    {
        if( loop != 9 )
        {
            // Silence the channel
            writeOPL(0x40 + operatorRegisterOffsets[loop], 0x3F);
            writeOPL(0x43 + operatorRegisterOffsets[loop], 0x3F);
        }
        initChannel(m_channels[loop]);
    }
}

// Old calling style: output0x388(0xABCD)
// New calling style: writeOPL(0xAB, 0xCD)

void AdlibDriver::writeOPL(uint8_t reg, uint8_t val)
{
    m_opl.writeReg(reg, val);
}

void AdlibDriver::initChannel(Channel& channel)
{
    memset(&channel.dataptr, 0,
           sizeof(Channel) - (reinterpret_cast<char*>(&channel.dataptr) - reinterpret_cast<char *>(&channel)));

    channel.tempo = 0xFF;
    channel.priority = 0;
    // normally here are nullfuncs but we set 0 for now
    channel.primaryEffect = nullptr;
    channel.secondaryEffect = nullptr;
    channel.spacing1 = 1;
}

void AdlibDriver::noteOff(Channel& channel)
{
    // The control channel has no corresponding Adlib channel

    if( m_oplChannel >= 9 )
        return;

    // When the rhythm section is enabled, channels 6, 7 and 8 are special.

    if( _rhythmSectionBits && m_oplChannel >= 6 )
        return;

    // This means the "Key On" bit will always be 0
    channel.konBlockFnumH &= 0xDF;

    // Octave / F-Number / Key-On
    writeOPL(0xB0 + m_oplChannel, channel.konBlockFnumH);
}

void AdlibDriver::unkOutput2(uint8_t chan)
{
    // The control channel has no corresponding Adlib channel

    if( chan >= 9 )
        return;

    // I believe this has to do with channels 6, 7, and 8 being special
    // when Adlib's rhythm section is enabled.

    if( _rhythmSectionBits && chan >= 6 )
        return;

    uint8_t offset = operatorRegisterOffsets[chan];

    // The channel is cleared: First the attack/delay rate, then the
    // sustain level/release rate, and finally the note is turned off.

    writeOPL(0x60 + offset, 0xFF);
    writeOPL(0x63 + offset, 0xFF);

    writeOPL(0x80 + offset, 0xFF);
    writeOPL(0x83 + offset, 0xFF);

    writeOPL(0xB0 + chan, 0x00);

    // ...and then the note is turned on again, with whatever value is
    // still lurking in the A0 + chan register, but everything else -
    // including the two most significant frequency bit, and the octave -
    // set to zero.
    //
    // This is very strange behaviour, and causes problems with the ancient
    // FMOPL code we borrowed from AdPlug. I've added a workaround. See
    // fmopl.cpp for more details.
    //
    // More recent versions of the MAME FMOPL don't seem to have this
    // problem, but cannot currently be used because of licensing and
    // performance issues.
    //
    // Ken Silverman's Adlib emulator (which can be found on his Web page -
    // http://www.advsys.net/ken - and as part of AdPlug) also seems to be
    // immune, but is apparently not as feature complete as MAME's.

    writeOPL(0xB0 + chan, 0x20);
}

// I believe this is a random number generator. It actually does seem to
// generate an even distribution of almost all numbers from 0 through 65535,
// though in my tests some numbers were never generated.

uint16_t AdlibDriver::getRandomNr()
{
    _rnd += 0x9248;
    uint16_t lowBits = _rnd & 7;
    _rnd >>= 3;
    _rnd |= (lowBits << 13);
    return _rnd;
}

void AdlibDriver::setupDuration(uint8_t duration, Channel& channel)
{
    if( channel.durationRandomness )
    {
        channel.duration = duration + (getRandomNr() & channel.durationRandomness);
        return;
    }
    if( channel.fractionalSpacing )
    {
        channel.spacing2 = (duration >> 3) * channel.fractionalSpacing;
    }
    channel.duration = duration;
}

// This function may or may not play the note. It's usually followed by a call
// to noteOn(), which will always play the current note.

void AdlibDriver::setupNote(uint8_t rawNote, Channel& channel, bool flag)
{
    channel.rawNote = rawNote;

    int8_t note = (rawNote & 0x0F) + channel.baseNote;
    int8_t octave = ((rawNote + channel.baseOctave) >> 4) & 0x0F;

    // There are only twelve notes. If we go outside that, we have to
    // adjust the note and octave.

    if( note >= 12 )
    {
        note -= 12;
        octave++;
    }
    else if( note < 0 )
    {
        note += 12;
        octave--;
    }

    // The calculation of frequency looks quite different from the original
    // disassembly at a first glance, but when you consider that the
    // largest possible value would be 0x0246 + 0xFF + 0x47 (and that's if
    // baseFreq is unsigned), freq is still a 10-bit value, just as it
    // should be to fit in the Ax and Bx registers.
    //
    // If it were larger than that, it could have overflowed into the
    // octave bits, and that could possibly have been used in some sound.
    // But as it is now, I can't see any way it would happen.

    uint16_t freq = _unkTable[note] + channel.baseFreq;

    // When called from callback 41, the behaviour is slightly different:
    // We adjust the frequency, even when channel.unk16 is 0.

    if( channel.finetune != 0 || flag )
    {
        if( channel.finetune >= 0 )
        {
            freq += _unkTables[(channel.rawNote & 0x0F) + 2][channel.finetune];
        }
        else
        {
            freq -= _unkTables[channel.rawNote & 0x0F][-channel.finetune];
        }
    }

    channel.fnumL = freq & 0xFF;
    channel.konBlockFnumH = (channel.konBlockFnumH & 0x20) | (octave << 2) | ((freq >> 8) & 0x03);

    // Keep the note on or off
    writeOPL(0xA0 + m_oplChannel, channel.fnumL);
    writeOPL(0xB0 + m_oplChannel, channel.konBlockFnumH);
}

void AdlibDriver::setupInstrument(uint8_t regOffset, const uint8_t* dataptr,
                                  Channel& channel)
{
    // Amplitude Modulation / Vibrato / Envelope Generator Type /
    // Keyboard Scaling Rate / Modulator Frequency Multiple
    writeOPL(0x20 + regOffset, *dataptr++);
    writeOPL(0x23 + regOffset, *dataptr++);

    uint8_t temp = *dataptr++;

    // Feedback / Algorithm

    // It is very likely that _curChannel really does refer to the same
    // channel as regOffset, but there's only one Cx register per channel.

    writeOPL(0xC0 + m_oplChannel, temp);

    // The algorithm bit. I don't pretend to understand this fully, but
    // "If set to 0, operator 1 modulates operator 2. In this case,
    // operator 2 is the only one producing sound. If set to 1, both
    // operators produce sound directly. Complex sounds are more easily
    // created if the algorithm is set to 0."

    channel.cnt = (temp & 1) != 0;

    // Waveform Select
    writeOPL(0xE0 + regOffset, *dataptr++);
    writeOPL(0xE3 + regOffset, *dataptr++);

    channel.opLevel1 = *dataptr++;
    channel.opLevel2 = *dataptr++;

    // Level Key Scaling / Total Level
    writeOPL(0x40 + regOffset, calculateOpLevel1(channel));
    writeOPL(0x43 + regOffset, calculateOpLevel2(channel));

    // Attack Rate / Decay Rate
    writeOPL(0x60 + regOffset, *dataptr++);
    writeOPL(0x63 + regOffset, *dataptr++);

    // Sustain Level / Release Rate
    writeOPL(0x80 + regOffset, *dataptr++);
    writeOPL(0x83 + regOffset, *dataptr++);
}

// Apart from playing the note, this function also updates the variables for
// primary effect 2.

void AdlibDriver::noteOn(Channel& channel)
{
    // The "note on" bit is set, and the current note is played.

    channel.konBlockFnumH |= 0x20;
    writeOPL(0xB0 + m_oplChannel, channel.konBlockFnumH);

    int8_t shift = 9 - channel.unk33;
    uint16_t temp = channel.fnumL | (channel.konBlockFnumH << 8);
    channel.vibratoSpeed = ((temp & 0x3FF) >> shift) & 0xFF;
    channel.unk38 = channel.unk36;
}

void AdlibDriver::adjustVolume(Channel& channel)
{
    // Level Key Scaling / Total Level

    writeOPL(0x43 + operatorRegisterOffsets[m_oplChannel], calculateOpLevel2(channel));
    if( channel.cnt )
        writeOPL(0x40 + operatorRegisterOffsets[m_oplChannel], calculateOpLevel1(channel));
}

// This is presumably only used for some sound effects, e.g. Malcolm blowing up
// the trees in the intro (but not the effect where he "booby-traps" the big
// tree) and turning Kallak to stone. Related functions and variables:
//
// update_setupPrimaryEffect1()
//    - Initialises unk29, unk30 and unk31
//    - unk29 is not further modified
//    - unk30 is not further modified, except by update_removePrimaryEffect1()
//
// update_removePrimaryEffect1()
//    - Deinitialises unk30
//
// unk29 - determines how often the notes are played
// unk30 - modifies the frequency
// unk31 - determines how often the notes are played

void AdlibDriver::primaryEffect1(Channel& channel)
{
    uint8_t temp = channel.unk31;
    channel.unk31 += channel.unk29;
    if( channel.unk31 >= temp )
        return;

    uint16_t frq = ((channel.konBlockFnumH & 3) << 8) | channel.fnumL;

    // This is presumably to shift the "note on" bit so far to the left
    // that it won't be affected by any of the calculations below.
    uint16_t konFnumH = ((channel.konBlockFnumH & 0x20) << 8) | (channel.konBlockFnumH & 0x1C);

    int16_t unk3 = static_cast<int16_t>(channel.unk30);

    frq += unk3;
    if( unk3 >= 0 )
    {
        if( frq >= 734 )
        {
            // The new frequency is too high. Shift it down and go
            // up one octave.
            frq >>= 1;
            if( (frq & 0x3FF) == 0 )
                ++frq;
            konFnumH = (konFnumH & 0xFF00) | ((konFnumH + 4) & 0xFF);
            konFnumH &= 0xFF1C;
        }
    }
    else
    {
        if( frq < 388 )
        {
            // The new frequency is too low. Shift it up and go
            // down one octave.
            frq <<= 1;
            if( (frq & 0x3FF) == 0 )
                --frq;
            konFnumH = (konFnumH & 0xFF00) | ((konFnumH - 4) & 0xFF);
            konFnumH &= 0xFF1C;
        }
    }

    // Make sure that the new frequency is still a 10-bit value.
    frq &= 0x3FF;

    writeOPL(0xA0 + m_oplChannel, frq & 0xFF);
    channel.fnumL = frq & 0xFF;

    // Shift down the "note on" bit again.
    uint8_t value = frq >> 8;
    value |= (konFnumH >> 8) & 0xFF;
    value |= konFnumH & 0xFF;

    writeOPL(0xB0 + m_oplChannel, value);
    channel.konBlockFnumH = value;
}

// This is presumably only used for some sound effects, e.g. Malcolm entering
// and leaving Kallak's hut. Related functions and variables:
//
// update_setupPrimaryEffect2()
//    - Initialises unk32, unk33, unk34, unk35 and unk36
//    - unk32 is not further modified
//    - unk33 is not further modified
//    - unk34 is a countdown that gets reinitialised to unk35 on zero
//    - unk35 is based on unk34 and not further modified
//    - unk36 is not further modified
//
// noteOn()
//    - Plays the current note
//    - Updates unk37 with a new (lower?) frequency
//    - Copies unk36 to unk38. The unk38 variable is a countdown.
//
// unk32 - determines how often the notes are played
// unk33 - modifies the frequency
// unk34 - countdown, updates frequency on zero
// unk35 - initialiser for unk34 countdown
// unk36 - initialiser for unk38 countdown
// unk37 - frequency
// unk38 - countdown, begins playing on zero
// unk41 - determines how often the notes are played
//
// Note that unk41 is never initialised. Not that it should matter much, but it
// is a bit sloppy.

void AdlibDriver::primaryEffect2(Channel& channel)
{
    if( channel.unk38 )
    {
        --channel.unk38;
        return;
    }

    uint8_t temp = channel.unk41;
    channel.unk41 += channel.unk32;
    if( channel.unk41 >= temp )
        return;

    if( --channel.vibratoCountdown == 0 )
    {
        channel.vibratoSpeed = -channel.vibratoSpeed;
        channel.vibratoCountdown = channel.vibratoPeriod;
    }

    uint16_t frq = (channel.fnumL | (channel.konBlockFnumH << 8)) & 0x3FF;
    frq += channel.vibratoSpeed;

    channel.fnumL = frq & 0xFF;
    channel.konBlockFnumH = (channel.konBlockFnumH & 0xFC) | (frq >> 8);

    // Octave / F-Number / Key-On
    writeOPL(0xA0 + m_oplChannel, channel.fnumL);
    writeOPL(0xB0 + m_oplChannel, channel.konBlockFnumH);
}

// I don't know where this is used. The same operation is performed several
// times on the current channel, using a chunk of the _soundData[] buffer for
// parameters. The parameters are used starting at the end of the chunk.
//
// Since we use _curRegOffset to specify the final register, it's quite
// unlikely that this function is ever used to play notes. It's probably only
// used to modify the sound. Another thing that supports this idea is that it
// can be combined with any of the effects callbacks above.
//
// Related functions and variables:
//
// update_setupSecondaryEffect1()
//    - Initialies unk18, unk19, unk20, unk21, unk22 and offset
//    - unk19 is not further modified
//    - unk20 is not further modified
//    - unk22 is not further modified
//    - offset is not further modified
//
// unk18 -  determines how often the operation is performed
// unk19 -  determines how often the operation is performed
// unk20 -  the start index into the data chunk
// unk21 -  the current index into the data chunk
// unk22 -  the operation to perform
// offset - the offset to the data chunk

void AdlibDriver::secondaryEffect1(Channel& channel)
{
    uint8_t temp = channel.unk18;
    channel.unk18 += channel.unk19;
    if( channel.unk18 >= temp )
        return;

    if( --channel.rawRegisterDataOffset < 0 )
    {
        channel.rawRegisterDataOffset = channel.unk20;
    }
    writeOPL(channel.rawRegisterOffset + m_operatorRegisterOffset,
             m_soundData[channel.offset + channel.rawRegisterDataOffset]);
}

uint8_t AdlibDriver::calculateOpLevel1(Channel& channel)
{
    int8_t value = channel.opLevel1 & 0x3F;

    if( channel.cnt )
    {
        value += channel.opExtraLevel1;
        value += channel.opExtraLevel2;
        value += channel.opExtraLevel3;
    }

    // Preserve the scaling level bits from opLevel1

    return clampTotalLevel(value) | (channel.opLevel1 & 0xC0);
}

uint8_t AdlibDriver::calculateOpLevel2(Channel& channel)
{
    int8_t value = channel.opLevel2 & 0x3F;

    value += channel.opExtraLevel1;
    value += channel.opExtraLevel2;
    value += channel.opExtraLevel3;

    // Preserve the scaling level bits from opLevel2

    return clampTotalLevel(value) | (channel.opLevel2 & 0xC0);
}

// parser opcodes

int AdlibDriver::update_setRepeat(const uint8_t*&, Channel& channel, uint8_t value)
{
    channel.repeatCounter = value;
    return 0;
}

int AdlibDriver::update_checkRepeat(const uint8_t*& dataptr, Channel& channel, uint8_t)
{
    ++dataptr;
    if( --channel.repeatCounter )
    {
        int16_t add = READ_LE_UINT16(dataptr - 2);
        dataptr += add;
    }
    return 0;
}

int AdlibDriver::update_setupProgram(const uint8_t*&, Channel&, uint8_t value)
{
    if( value == 0xFF )
        return 0;

    const uint8_t* ptr = getProgram(value);
    uint8_t chan = *ptr++;
    uint8_t priority = *ptr++;

    Channel& channel2 = m_channels[chan];

    if( priority >= channel2.priority )
    {
        m_flagTrigger = 1;
        m_flags |= 8;
        initChannel(channel2);
        channel2.priority = priority;
        channel2.dataptr = ptr;
        channel2.tempo = 0xFF;
        channel2.position = 0xFF;
        channel2.duration = 1;
        unkOutput2(chan);
    }

    return 0;
}

int AdlibDriver::update_setNoteSpacing(const uint8_t*&, Channel& channel,
                                       uint8_t value)
{
    channel.spacing1 = value;
    return 0;
}

int AdlibDriver::update_jump(const uint8_t*& dataptr, Channel&, uint8_t)
{
    --dataptr;
    int16_t add = READ_LE_UINT16(dataptr);
    dataptr += 2;
    dataptr += add;
    return 0;
}

int AdlibDriver::update_jumpToSubroutine(const uint8_t*& dataptr, Channel& channel,
                                         uint8_t)
{
    --dataptr;
    int16_t add = READ_LE_UINT16(dataptr);
    dataptr += 2;
    channel.dataptrStack[channel.dataptrStackPos++] = dataptr;
    dataptr += add;
    return 0;
}

int AdlibDriver::update_returnFromSubroutine(const uint8_t*& dataptr, Channel& channel,
                                             uint8_t)
{
    dataptr = channel.dataptrStack[--channel.dataptrStackPos];
    return 0;
}

int AdlibDriver::update_setBaseOctave(const uint8_t*&, Channel& channel, uint8_t value)
{
    channel.baseOctave = value;
    return 0;
}

int AdlibDriver::update_stopChannel(const uint8_t*& dataptr, Channel& channel, uint8_t)
{
    channel.priority = 0;
    if( m_oplChannel != 9 )
    {
        noteOff(channel);
    }
    dataptr = nullptr;
    return 2;
}

int AdlibDriver::update_playRest(const uint8_t*&, Channel& channel, uint8_t value)
{
    setupDuration(value, channel);
    noteOff(channel);
    return (value != 0);
}

int AdlibDriver::update_writeAdlib(const uint8_t*& dataptr, Channel&, uint8_t value)
{
    writeOPL(value, *dataptr++);
    return 0;
}

int AdlibDriver::update_setupNoteAndDuration(const uint8_t*& dataptr, Channel& channel,
                                             uint8_t value)
{
    setupNote(value, channel);
    value = *dataptr++;
    setupDuration(value, channel);
    return (value != 0);
}

int AdlibDriver::update_setBaseNote(const uint8_t*&, Channel& channel, uint8_t value)
{
    channel.baseNote = value;
    return 0;
}

int AdlibDriver::update_setupSecondaryEffect1(const uint8_t*& dataptr, Channel& channel,
                                              uint8_t value)
{
    channel.unk18 = value;
    channel.unk19 = value;
    channel.unk20 = channel.rawRegisterDataOffset = *dataptr++;
    channel.rawRegisterOffset = *dataptr++;
    channel.offset = READ_LE_UINT16(dataptr);
    dataptr += 2;
    channel.secondaryEffect = &AdlibDriver::secondaryEffect1;
    return 0;
}

int AdlibDriver::update_stopOtherChannel(const uint8_t*&, Channel&, uint8_t value)
{
    Channel& channel2 = m_channels[value];
    channel2.duration = 0;
    channel2.priority = 0;
    channel2.dataptr = nullptr;
    return 0;
}

int AdlibDriver::update_waitForEndOfProgram(const uint8_t*& dataptr, Channel&,
                                            uint8_t value)
{
    const uint8_t* ptr = getProgram(value);
    uint8_t chan = *ptr;

    if( !m_channels[chan].dataptr )
    {
        return 0;
    }

    dataptr -= 2;
    return 2;
}

int AdlibDriver::update_setupInstrument(const uint8_t*&, Channel& channel,
                                        uint8_t value)
{
    setupInstrument(m_operatorRegisterOffset, getInstrument(value), channel);
    return 0;
}

int AdlibDriver::update_setupPrimaryEffect1(const uint8_t*& dataptr, Channel& channel,
                                            uint8_t value)
{
    channel.unk29 = value;
    channel.unk30 = READ_BE_UINT16(dataptr);
    dataptr += 2;
    channel.primaryEffect = &AdlibDriver::primaryEffect1;
    channel.unk31 = 0xFF;
    return 0;
}

int AdlibDriver::update_removePrimaryEffect1(const uint8_t*& dataptr, Channel& channel,
                                             uint8_t)
{
    --dataptr;
    channel.primaryEffect = nullptr;
    channel.unk30 = 0;
    return 0;
}

int AdlibDriver::update_setBaseFreq(const uint8_t*&, Channel& channel, uint8_t value)
{
    channel.baseFreq = value;
    return 0;
}

int AdlibDriver::update_setupPrimaryEffect2(const uint8_t*& dataptr, Channel& channel,
                                            uint8_t value)
{
    channel.unk32 = value;
    channel.unk33 = *dataptr++;
    uint8_t temp = *dataptr++;
    channel.vibratoCountdown = temp + 1;
    channel.vibratoPeriod = temp << 1;
    channel.unk36 = *dataptr++;
    channel.primaryEffect = &AdlibDriver::primaryEffect2;
    return 0;
}

int AdlibDriver::update_setPriority(const uint8_t*&, Channel& channel, uint8_t value)
{
    channel.priority = value;
    return 0;
}

int AdlibDriver::updateCallback23(const uint8_t*&, Channel&, uint8_t value)
{
    value >>= 1;
    _unkValue1 = _unkValue2 = value;
    _unkValue3 = 0xFF;
    _unkValue4 = _unkValue5 = 0;
    return 0;
}

int AdlibDriver::updateCallback24(const uint8_t*& dataptr, Channel& channel,
                                  uint8_t value)
{
    if( _unkValue5 )
    {
        if( _unkValue4 & value )
        {
            _unkValue5 = 0;
            return 0;
        }
    }

    if( (value & _unkValue4) == 0 )
    {
        ++_unkValue5;
    }

    dataptr -= 2;
    channel.duration = 1;
    return 2;
}

int AdlibDriver::update_setExtraLevel1(const uint8_t*&, Channel& channel,
                                       uint8_t value)
{
    channel.opExtraLevel1 = value;
    adjustVolume(channel);
    return 0;
}

int AdlibDriver::update_setupDuration(const uint8_t*&, Channel& channel, uint8_t value)
{
    setupDuration(value, channel);
    return (value != 0);
}

int AdlibDriver::update_playNote(const uint8_t*&, Channel& channel, uint8_t value)
{
    setupDuration(value, channel);
    noteOn(channel);
    return (value != 0);
}

int AdlibDriver::update_setFractionalNoteSpacing(const uint8_t*&, Channel& channel,
                                                 uint8_t value)
{
    channel.fractionalSpacing = value & 7;
    return 0;
}

int AdlibDriver::update_setTempo(const uint8_t*&, Channel&, uint8_t value)
{
    m_tempo = value;
    return 0;
}

int AdlibDriver::update_removeSecondaryEffect1(const uint8_t*& dataptr,
                                               Channel& channel, uint8_t)
{
    --dataptr;
    channel.secondaryEffect = nullptr;
    return 0;
}

int AdlibDriver::update_setChannelTempo(const uint8_t*&, Channel& channel,
                                        uint8_t value)
{
    channel.tempo = value;
    return 0;
}

int AdlibDriver::update_setExtraLevel3(const uint8_t*&, Channel& channel,
                                       uint8_t value)
{
    channel.opExtraLevel3 = value;
    return 0;
}

int AdlibDriver::update_setExtraLevel2(const uint8_t*& dataptr, Channel&,
                                       uint8_t value)
{
    const auto channelBackUp = m_oplChannel;

    m_oplChannel = value;
    Channel& channel2 = m_channels[value];
    channel2.opExtraLevel2 = *dataptr++;
    adjustVolume(channel2);

    m_oplChannel = channelBackUp;
    return 0;
}

int AdlibDriver::update_changeExtraLevel2(const uint8_t*& dataptr, Channel&,
                                          uint8_t value)
{
    const auto channelBackUp = m_oplChannel;

    m_oplChannel = value;
    Channel& channel2 = m_channels[value];
    channel2.opExtraLevel2 += *dataptr++;
    adjustVolume(channel2);

    m_oplChannel = channelBackUp;
    return 0;
}

// Apart from initialising to zero, these two functions are the only ones that
// modify _vibratoAndAMDepthBits.

int AdlibDriver::update_setAMDepth(const uint8_t*&, Channel&, uint8_t value)
{
    if( value & 1 )
        m_vibratoAndAMDepthBits |= 0x80;
    else
        m_vibratoAndAMDepthBits &= 0x7F;

    writeOPL(0xBD, m_vibratoAndAMDepthBits);
    return 0;
}

int AdlibDriver::update_setVibratoDepth(const uint8_t*&, Channel&, uint8_t value)
{
    if( value & 1 )
        m_vibratoAndAMDepthBits |= 0x40;
    else
        m_vibratoAndAMDepthBits &= 0xBF;

    writeOPL(0xBD, m_vibratoAndAMDepthBits);
    return 0;
}

int AdlibDriver::update_changeExtraLevel1(const uint8_t*&, Channel& channel,
                                          uint8_t value)
{
    channel.opExtraLevel1 += value;
    adjustVolume(channel);
    return 0;
}

int AdlibDriver::updateCallback38(const uint8_t*&, Channel&, uint8_t value)
{
    const auto channelBackUp = m_oplChannel;

    m_oplChannel = value;
    Channel& channel2 = m_channels[value];
    channel2.duration = channel2.priority = 0;
    channel2.dataptr = nullptr;
    channel2.opExtraLevel2 = 0;

    if( value != 9 )
    {
        uint8_t outValue = operatorRegisterOffsets[value];

        // Feedback strength / Connection type
        writeOPL(0xC0 + m_oplChannel, 0x00);

        // Key scaling level / Operator output level
        writeOPL(0x43 + outValue, 0x3F);

        // Sustain Level / Release Rate
        writeOPL(0x83 + outValue, 0xFF);

        // Key On / Octave / Frequency
        writeOPL(0xB0 + m_oplChannel, 0x00);
    }

    m_oplChannel = channelBackUp;
    return 0;
}

int AdlibDriver::updateCallback39(const uint8_t*& dataptr, Channel& channel,
                                  uint8_t value)
{
    uint16_t unk = *dataptr++;
    unk |= value << 8;
    unk &= getRandomNr();

    uint16_t unk2 = ((channel.konBlockFnumH & 0x1F) << 8) | channel.fnumL;
    unk2 += unk;
    unk2 |= ((channel.konBlockFnumH & 0x20) << 8);

    // Frequency
    writeOPL(0xA0 + m_oplChannel, unk2 & 0xFF);

    // Key On / Octave / Frequency
    writeOPL(0xB0 + m_oplChannel, (unk2 & 0xFF00) >> 8);

    return 0;
}

int AdlibDriver::update_removePrimaryEffect2(const uint8_t*& dataptr, Channel& channel,
                                             uint8_t)
{
    --dataptr;
    channel.primaryEffect = nullptr;
    return 0;
}

int AdlibDriver::setFinetune(const uint8_t*&, Channel& channel, uint8_t value)
{
    channel.finetune = value;
    setupNote(channel.rawNote, channel, true);
    return 0;
}

int AdlibDriver::update_resetToGlobalTempo(const uint8_t*& dataptr, Channel& channel,
                                           uint8_t)
{
    --dataptr;
    channel.tempo = m_tempo;
    return 0;
}

int AdlibDriver::update_nop1(const uint8_t*& dataptr, Channel&, uint8_t)
{
    --dataptr;
    return 0;
}

int AdlibDriver::update_setDurationRandomness(const uint8_t*&, Channel& channel,
                                              uint8_t value)
{
    channel.durationRandomness = value;
    return 0;
}

int AdlibDriver::update_changeChannelTempo(const uint8_t*&, Channel& channel,
                                           uint8_t value)
{
    int tempo = channel.tempo + static_cast<int8_t>(value);

    if( tempo <= 0 )
        tempo = 1;
    else if( tempo > 255 )
        tempo = 255;

    channel.tempo = tempo;
    return 0;
}

int AdlibDriver::updateCallback46(const uint8_t*& dataptr, Channel&, uint8_t value)
{
    uint8_t entry = *dataptr++;
    _tablePtr1 = _unkTable2[entry++];
    _tablePtr2 = _unkTable2[entry];
    if( value == 2 )
    {
        // Frequency
        writeOPL(0xA0, _tablePtr2[0]);
    }
    return 0;
}

// TODO: This is really the same as update_nop1(), so they should be combined
//       into one single update_nop().

int AdlibDriver::update_nop2(const uint8_t*& dataptr, Channel&, uint8_t)
{
    --dataptr;
    return 0;
}

int AdlibDriver::update_setupRhythmSection(const uint8_t*& dataptr, Channel& channel,
                                           uint8_t value)
{
    int channelBackUp = m_oplChannel;
    int regOffsetBackUp = m_operatorRegisterOffset;

    m_oplChannel = 6;
    m_operatorRegisterOffset = operatorRegisterOffsets[6];

    setupInstrument(m_operatorRegisterOffset, getInstrument(value), channel);
    _unkValue6 = channel.opLevel2;

    m_oplChannel = 7;
    m_operatorRegisterOffset = operatorRegisterOffsets[7];

    setupInstrument(m_operatorRegisterOffset, getInstrument(*dataptr++), channel);
    _unkValue7 = channel.opLevel1;
    _unkValue8 = channel.opLevel2;

    m_oplChannel = 8;
    m_operatorRegisterOffset = operatorRegisterOffsets[8];

    setupInstrument(m_operatorRegisterOffset, getInstrument(*dataptr++), channel);
    _unkValue9 = channel.opLevel1;
    _unkValue10 = channel.opLevel2;

    // Octave / F-Number / Key-On for channels 6, 7 and 8

    m_channels[6].konBlockFnumH = *dataptr++ & 0x2F;
    writeOPL(0xB6, m_channels[6].konBlockFnumH);
    writeOPL(0xA6, *dataptr++);

    m_channels[7].konBlockFnumH = *dataptr++ & 0x2F;
    writeOPL(0xB7, m_channels[7].konBlockFnumH);
    writeOPL(0xA7, *dataptr++);

    m_channels[8].konBlockFnumH = *dataptr++ & 0x2F;
    writeOPL(0xB8, m_channels[8].konBlockFnumH);
    writeOPL(0xA8, *dataptr++);

    _rhythmSectionBits = 0x20;

    m_operatorRegisterOffset = regOffsetBackUp;
    m_oplChannel = channelBackUp;
    return 0;
}

int AdlibDriver::update_playRhythmSection(const uint8_t*&, Channel&, uint8_t value)
{
    // Any instrument that we want to play, and which was already playing,
    // is temporarily keyed off. Instruments that were off already, or
    // which we don't want to play, retain their old on/off status. This is
    // probably so that the instrument's envelope is played from its
    // beginning again...

    writeOPL(0xBD, (_rhythmSectionBits & ~(value & 0x1F)) | 0x20);

    // ...but since we only set the rhythm instrument bits, and never clear
    // them (until the entire rhythm section is disabled), I'm not sure how
    // useful the cleverness above is. We could perhaps simply turn off all
    // the rhythm instruments instead.

    _rhythmSectionBits |= value;

    writeOPL(0xBD, m_vibratoAndAMDepthBits | 0x20 | _rhythmSectionBits);
    return 0;
}

int AdlibDriver::update_removeRhythmSection(const uint8_t*& dataptr, Channel&, uint8_t)
{
    --dataptr;
    _rhythmSectionBits = 0;

    // All the rhythm bits are cleared. The AM and Vibrato depth bits
    // remain unchanged.

    writeOPL(0xBD, m_vibratoAndAMDepthBits);
    return 0;
}

int AdlibDriver::updateCallback51(const uint8_t*& dataptr, Channel&, uint8_t value)
{
    uint8_t value2 = *dataptr++;

    if( value & 1 )
    {
        _unkValue12 = value2;

        // Channel 7, op1: Level Key Scaling / Total Level
        writeOPL(0x51, clampTotalLevel(value2 + _unkValue7 + _unkValue11 + _unkValue12));
    }

    if( value & 2 )
    {
        _unkValue14 = value2;

        // Channel 8, op2: Level Key Scaling / Total Level
        writeOPL(0x55,
                 clampTotalLevel(value2 + _unkValue10 + _unkValue13 + _unkValue14));
    }

    if( value & 4 )
    {
        _unkValue15 = value2;

        // Channel 8, op1: Level Key Scaling / Total Level
        writeOPL(0x52, clampTotalLevel(value2 + _unkValue9 + _unkValue16 + _unkValue15));
    }

    if( value & 8 )
    {
        _unkValue18 = value2;

        // Channel 7, op2: Level Key Scaling / Total Level
        writeOPL(0x54, clampTotalLevel(value2 + _unkValue8 + _unkValue17 + _unkValue18));
    }

    if( value & 16 )
    {
        _unkValue20 = value2;

        // Channel 6, op2: Level Key Scaling / Total Level
        writeOPL(0x53, clampTotalLevel(value2 + _unkValue6 + _unkValue19 + _unkValue20));
    }

    return 0;
}

int AdlibDriver::updateCallback52(const uint8_t*& dataptr, Channel&, uint8_t value)
{
    uint8_t value2 = *dataptr++;

    if( value & 1 )
    {
        _unkValue11 = clampTotalLevel(value2 + _unkValue7 + _unkValue11 + _unkValue12);

        // Channel 7, op1: Level Key Scaling / Total Level
        writeOPL(0x51, _unkValue11);
    }

    if( value & 2 )
    {
        _unkValue13 = clampTotalLevel(value2 + _unkValue10 + _unkValue13 + _unkValue14);

        // Channel 8, op2: Level Key Scaling / Total Level
        writeOPL(0x55, _unkValue13);
    }

    if( value & 4 )
    {
        _unkValue16 = clampTotalLevel(value2 + _unkValue9 + _unkValue16 + _unkValue15);

        // Channel 8, op1: Level Key Scaling / Total Level
        writeOPL(0x52, _unkValue16);
    }

    if( value & 8 )
    {
        _unkValue17 = clampTotalLevel(value2 + _unkValue8 + _unkValue17 + _unkValue18);

        // Channel 7, op2: Level Key Scaling / Total Level
        writeOPL(0x54, _unkValue17);
    }

    if( value & 16 )
    {
        _unkValue19 = clampTotalLevel(value2 + _unkValue6 + _unkValue19 + _unkValue20);

        // Channel 6, op2: Level Key Scaling / Total Level
        writeOPL(0x53, _unkValue19);
    }

    return 0;
}

int AdlibDriver::updateCallback53(const uint8_t*& dataptr, Channel&, uint8_t value)
{
    uint8_t value2 = *dataptr++;

    if( value & 1 )
    {
        _unkValue11 = value2;

        // Channel 7, op1: Level Key Scaling / Total Level
        writeOPL(0x51, clampTotalLevel(value2 + _unkValue7 + _unkValue12));
    }

    if( value & 2 )
    {
        _unkValue13 = value2;

        // Channel 8, op2: Level Key Scaling / Total Level
        writeOPL(0x55, clampTotalLevel(value2 + _unkValue10 + _unkValue14));
    }

    if( value & 4 )
    {
        _unkValue16 = value2;

        // Channel 8, op1: Level Key Scaling / Total Level
        writeOPL(0x52, clampTotalLevel(value2 + _unkValue9 + _unkValue15));
    }

    if( value & 8 )
    {
        _unkValue17 = value2;

        // Channel 7, op2: Level Key Scaling / Total Level
        writeOPL(0x54, clampTotalLevel(value2 + _unkValue8 + _unkValue18));
    }

    if( value & 16 )
    {
        _unkValue19 = value2;

        // Channel 6, op2: Level Key Scaling / Total Level
        writeOPL(0x53, clampTotalLevel(value2 + _unkValue6 + _unkValue20));
    }

    return 0;
}

int AdlibDriver::update_setSoundTrigger(const uint8_t*&, Channel&, uint8_t value)
{
    _soundTrigger = value;
    return 0;
}

int AdlibDriver::update_setTempoReset(const uint8_t*&, Channel& channel, uint8_t value)
{
    channel.tempoReset = value;
    return 0;
}

int AdlibDriver::updateCallback56(const uint8_t*& dataptr, Channel& channel,
                                  uint8_t value)
{
    channel.unk39 = value;
    channel.unk40 = *dataptr++;
    return 0;
}

// At the time of writing, the only known case where Kyra 1 uses sound triggers
// is in the castle, to cycle between three different songs.

AdlPlayer::AdlPlayer()
    : m_driver(new AdlibDriver)
{
    m_trackEntries.fill(0);
    init();
}

bool AdlPlayer::init()
{
    m_driver->snd_initDriver();
    m_driver->snd_setFlag(4);
    return true;
}

void AdlPlayer::process()
{
    uint8_t trigger = m_driver->snd_getSoundTrigger();

    if( trigger < 4 )
    {
        static constexpr int kyra1SoundTriggers[] = {0, 4, 5, 3};
        int soundId = kyra1SoundTriggers[trigger];

        if( soundId )
        {
            playTrack(soundId);
        }
    }
    else
    {
        // TODO: At this point, we really want to clear the trigger...
    }
}

void AdlPlayer::playTrack(uint8_t track)
{
    play(track);
}

void AdlPlayer::playSoundEffect(uint8_t track)
{
    play(track);
}

void AdlPlayer::play(uint8_t track)
{
    uint8_t soundId = m_trackEntries[track];
    if( static_cast<int8_t>(soundId) == -1 || m_soundDataPtr.empty() )
        return;
    soundId &= 0xFF;
    m_driver->snd_setFlag(0);
    // 	while ((_driver->callback(16, 0) & 8)) {
    // We call the system delay and not the game delay to avoid concurrency
    // issues.
    // 		_engine->_system->delayMillis(10);
    // 	}
    if( m_sfxPlayingSound != -1 )
    {
        // Restore the sounds's normal values.
        m_driver->snd_writeByte(m_sfxPlayingSound, 1, m_sfxPriority);
        m_driver->snd_writeByte(m_sfxPlayingSound, 3, m_sfxFourthByteOfSong);
        m_sfxPlayingSound = -1;
    }

    int chan = m_driver->snd_readByte(soundId, 0);

    if( chan != 9 )
    {
        m_sfxPlayingSound = soundId;
        m_sfxPriority = m_driver->snd_readByte(soundId, 1);
        m_sfxFourthByteOfSong = m_driver->snd_readByte(soundId, 3);

        // In the cases I've seen, the mysterious fourth byte has been
        // the parameter for the update_setExtraLevel3() callback.
        //
        // The extra level is part of the channels "total level", which
        // is a six-bit value where larger values means softer volume.
        //
        // So what seems to be happening here is that sounds which are
        // started by this function are given a slightly lower priority
        // and a slightly higher (i.e. softer) extra level 3 than they
        // would have if they were started from anywhere else. Strange.

        int newVal = ((((-m_sfxFourthByteOfSong) + 63) * 0xFF) >> 8) & 0xFF;
        newVal = -newVal + 63;
        m_driver->snd_writeByte(soundId, 3, newVal);
        newVal = ((m_sfxPriority * 0xFF) >> 8) & 0xFF;
        m_driver->snd_writeByte(soundId, 1, newVal);
    }

    m_driver->snd_startSong(soundId);
}

bool AdlPlayer::load(const std::string& filename)
{
    FileStream f(filename);

    // file validation section
    if( !f || f.extension() != ".adl" )
    {
        return false;
    }

    unk2();
    unk1();

    std::vector<uint8_t> file_data;
    file_data.resize(f.size());
    f.read(file_data.data(), f.size());

    m_driver->snd_unkOpcode3(-1);

    std::copy_n(file_data.begin(), 120, m_trackEntries.begin());

    m_soundDataPtr.resize(file_data.size() - 120);
    std::copy_n(file_data.data() + 120, m_soundDataPtr.size(), m_soundDataPtr.begin());

    file_data.clear();

    m_driver->snd_setSoundData(m_soundDataPtr.data());

    // 	_soundFileLoaded = file;

    // find last subsong
    auto it = std::find_if(m_trackEntries.rbegin(), m_trackEntries.rend(), [](uint8_t x)
                           {
                               return x != 0xff;
                           });
    m_subSongCount = m_trackEntries.size() - std::distance(m_trackEntries.rbegin(), it);

    m_currentSubSong = 2;
    rewind(boost::none);
    addOrder(0);
    return true;
}

void AdlPlayer::rewind(const boost::optional<size_t>& subsong)
{
    const auto ss = subsong.get_value_or(m_currentSubSong);
    getOpl()->writeReg(1, 32);
    playSoundEffect(ss);
    m_currentSubSong = ss;
    update();
}

size_t AdlPlayer::subSongCount() const
{
    return m_subSongCount;
}

bool AdlPlayer::update()
{
    bool songend = true;

    //   if(_trackEntries[cursubsong] == 0xff)
    //     return false;

    m_driver->callback();

    for( int i = 0; i < 10; i++ )
        if( m_driver->m_channels[i].dataptr != nullptr )
            songend = false;

    return !songend;
}

void AdlPlayer::unk1()
{
    playSoundEffect(0);
    //_engine->_system->delayMillis(5 * 60);
}

void AdlPlayer::unk2()
{
    playSoundEffect(0);
}

Player* AdlPlayer::factory()
{
    return new AdlPlayer();
}
