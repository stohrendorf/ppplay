/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2009 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * cmf.h - CMF player by Adam Nielsen <malvineous@shikadi.net>
 */

#include <stdint.h> // for uintxx_t
#include "player.h"

struct CMFHEADER
{
    uint16_t iInstrumentBlockOffset = 0;
    uint16_t iMusicOffset = 0;
    uint16_t iTicksPerQuarterNote = 0;
    uint16_t iTicksPerSecond = 0;
    uint16_t iTagOffsetTitle = 0;
    uint16_t iTagOffsetComposer = 0;
    uint16_t iTagOffsetRemarks = 0;
    uint8_t iChannelsInUse[16] = {0};
    uint16_t iNumInstruments = 0;
    uint16_t iTempo = 0;
};

struct OPERATOR {
    uint8_t iCharMult;
    uint8_t iScalingOutput;
    uint8_t iAttackDecay;
    uint8_t iSustainRelease;
    uint8_t iWaveSel;
};

struct SBI {
    OPERATOR op[2]; // 0 == modulator, 1 == carrier
    uint8_t iConnection;
};

struct MIDICHANNEL {
    int iPatch;     // MIDI patch for this channel
    int iPitchbend; // Current pitchbend amount for this channel
};

struct OPLCHANNEL {
    int iNoteStart; // When the note started playing (longest notes get cut
    // first, 0 == channel free)
    int iMIDINote;  // MIDI note number currently being played on this OPL channel
    int iMIDIChannel; // Source MIDI channel where this note came from
    int iMIDIPatch;   // Current MIDI patch set on this OPL channel
};

class CcmfPlayer : public CPlayer {
    DISABLE_COPY(CcmfPlayer)
private:
    std::vector<uint8_t> data{};    // song data (CMF music block)
    int iPlayPointer = 0; // Current location of playback pointer
    int iSongLen = 0;     // Max value for iPlayPointer
    CMFHEADER cmfHeader{};
    std::vector<SBI> pInstruments{};
    bool bPercussive = false;          // are rhythm-mode instruments enabled?
    uint8_t iCurrentRegs[256] = {0}; // Current values in the OPL chip
    int iTranspose = 0; // Transpose amount for entire song (between -128 and +128)
    uint8_t iPrevCommand = 0; // Previous command (used for repeated MIDI commands,
    // as the seek and playback code need to share this)

    int iNoteCount = 0; // Used to count how long notes have been playing for
    MIDICHANNEL chMIDI[16]{};
    OPLCHANNEL chOPL[9]{};

    // Additions for AdPlug's design
    int iDelayRemaining = 0;
    bool bSongEnd = false;
    std::string strTitle{};
    std::string strComposer{};
    std::string strRemarks{};

public:
    static CPlayer *factory();

    CcmfPlayer();
    ~CcmfPlayer() = default;

    bool load(const std::string &filename);
    bool update();
    void rewind(int);
    size_t framesUntilUpdate() const;

    std::string type() const
    {
        return "Creative Music File (CMF)";
    }
    std::string title() const;
    std::string author() const;
    std::string description() const;

private:
    uint32_t readMIDINumber();
    void writeInstrumentSettings(uint8_t iChannel, uint8_t iOperatorSource, uint8_t iOperatorDest, uint8_t iInstrument);
    void writeOPL(uint8_t iRegister, uint8_t iValue);
    void cmfNoteOn(uint8_t iChannel, uint8_t iNote, uint8_t iVelocity);
    void cmfNoteOff(uint8_t iChannel, uint8_t iNote, uint8_t);
    uint8_t getPercChannel(uint8_t iChannel);
    void MIDIchangeInstrument(uint8_t iOPLChannel, uint8_t iMIDIChannel, uint8_t iNewInstrument);
    void MIDIcontroller(uint8_t, uint8_t iController, uint8_t iValue);

};
