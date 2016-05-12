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
 * cmf.cpp - CMF player by Adam Nielsen <malvineous@shikadi.net>
 *   Subset of CMF reader in MOPL code (Malvineous' OPL player), no seeking etc.
 */

#include <cassert>
#include <cmath>   // for pow() etc.

#include "stream/filestream.h"

#include "cmf.h"
#include "light4cxx/logger.h"

namespace
{
light4cxx::Logger* logger = light4cxx::Logger::get("badplay.cmf");
}

// ------------------------------
// OPTIONS
// ------------------------------

// The official Creative Labs CMF player seems to ignore the note velocity
// (playing every note at the same volume), but you can uncomment this to
// allow the note velocity to affect the volume (as presumably the composer
// originally intended.)
//
//#define USE_VELOCITY
//
// The Xargon demo song is a good example of a song that uses note velocity.

// OPL register offsets
#define BASE_CHAR_MULT 0x20
#define BASE_SCAL_LEVL 0x40
#define BASE_ATCK_DCAY 0x60
#define BASE_SUST_RLSE 0x80
#define BASE_FNUM_L 0xA0
#define BASE_KEYON_FREQ 0xB0
#define BASE_RHYTHM 0xBD
#define BASE_WAVE 0xE0
#define BASE_FEED_CONN 0xC0

#define OPLBIT_KEYON                                                           \
    0x20 // Bit in BASE_KEYON_FREQ register for turning a note on

// Supplied with a channel, return the offset from a base OPL register for the
// Modulator cell (e.g. channel 4's modulator is at offset 0x09.  Since 0x60 is
// the attack/decay function, register 0x69 will thus set the attack/decay for
// channel 4's modulator.)  (channels go from 0 to 8 inclusive)
#define OPLOFFSET(channel) (((channel) / 3) * 8 + ((channel) % 3))

// These 16 instruments are repeated to fill up the 128 available slots.  A CMF
// file can override none/some/all of the 128 slots with custom instruments,
// so any that aren't overridden are still available for use with these default
// patches.  The Word Rescue CMFs are good examples of songs that rely on these
// default patches.
uint8_t cDefaultPatches[] = "\x01\x11\x4F\x00\xF1\xD2\x53\x74\x00\x00\x06"
"\x07\x12\x4F\x00\xF2\xF2\x60\x72\x00\x00\x08"
"\x31\xA1\x1C\x80\x51\x54\x03\x67\x00\x00\x0E"
"\x31\xA1\x1C\x80\x41\x92\x0B\x3B\x00\x00\x0E"
"\x31\x16\x87\x80\xA1\x7D\x11\x43\x00\x00\x08"
"\x30\xB1\xC8\x80\xD5\x61\x19\x1B\x00\x00\x0C"
"\xF1\x21\x01\x00\x97\xF1\x17\x18\x00\x00\x08"
"\x32\x16\x87\x80\xA1\x7D\x10\x33\x00\x00\x08"
"\x01\x12\x4F\x00\x71\x52\x53\x7C\x00\x00\x0A"
"\x02\x03\x8D\x00\xD7\xF5\x37\x18\x00\x00\x04"
"\x21\x21\xD1\x00\xA3\xA4\x46\x25\x00\x00\x0A"
"\x22\x22\x0F\x00\xF6\xF6\x95\x36\x00\x00\x0A"
"\xE1\xE1\x00\x00\x44\x54\x24\x34\x02\x02\x07"
"\xA5\xB1\xD2\x80\x81\xF1\x03\x05\x00\x00\x02"
"\x71\x22\xC5\x00\x6E\x8B\x17\x0E\x00\x00\x02"
"\x32\x21\x16\x80\x73\x75\x24\x57\x00\x00\x0E";

Player* CmfPlayer::factory()
{
    return new CmfPlayer();
}

CmfPlayer::CmfPlayer()
    : Player()
{
    assert(OPLOFFSET(1 - 1) == 0x00);
    assert(OPLOFFSET(5 - 1) == 0x09);
    assert(OPLOFFSET(9 - 1) == 0x12);
}

bool CmfPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if(!f)
        return false;

    char cSig[4];
    f.read(cSig, 4);
    if((cSig[0] != 'C') || (cSig[1] != 'T') || (cSig[2] != 'M') || (cSig[3] != 'F'))
    {
        // Not a CMF file
        return false;
    }
    uint16_t iVer;
    f >> iVer;
    if((iVer != 0x0101) && (iVer != 0x0100))
    {
        logger->warn(L4CXX_LOCATION, "CMF file is not v1.0 or v1.1 (reports %d.%d)", iVer >> 8, iVer & 0xFF);
        return false;
    }

    f >> this->cmfHeader.iInstrumentBlockOffset;
    f >> this->cmfHeader.iMusicOffset;
    f >> this->cmfHeader.iTicksPerQuarterNote;
    f >> this->cmfHeader.iTicksPerSecond;
    f >> this->cmfHeader.iTagOffsetTitle;
    f >> this->cmfHeader.iTagOffsetComposer;
    f >> this->cmfHeader.iTagOffsetRemarks;
    f.read(this->cmfHeader.iChannelsInUse, 16);
    if(iVer == 0x0100)
    {
        uint8_t tmp;
        f >> tmp;
        this->cmfHeader.iNumInstruments = tmp;
        this->cmfHeader.iTempo = 0;
    }
    else
    { // 0x0101
        f >> this->cmfHeader.iNumInstruments;
        f >> this->cmfHeader.iTempo;
    }

    // Load the instruments

    f.seek(this->cmfHeader.iInstrumentBlockOffset);
    this->pInstruments.resize(
        std::max(uint16_t(128), this->cmfHeader.iNumInstruments)
    ); // Always at least 128 available for use

    for(int i = 0; i < this->cmfHeader.iNumInstruments; i++)
    {
        f >> this->pInstruments[i].op[0].iCharMult;
        f >> this->pInstruments[i].op[1].iCharMult;
        f >> this->pInstruments[i].op[0].iScalingOutput;
        f >> this->pInstruments[i].op[1].iScalingOutput;
        f >> this->pInstruments[i].op[0].iAttackDecay;
        f >> this->pInstruments[i].op[1].iAttackDecay;
        f >> this->pInstruments[i].op[0].iSustainRelease;
        f >> this->pInstruments[i].op[1].iSustainRelease;
        f >> this->pInstruments[i].op[0].iWaveSel;
        f >> this->pInstruments[i].op[1].iWaveSel;
        f >> this->pInstruments[i].iConnection;
        f.seekrel(5); // skip over the padding bytes
    }

    // Set the rest of the instruments to the CMF defaults
    for(int i = this->cmfHeader.iNumInstruments; i < 128; i++)
    {
        this->pInstruments[i].op[0].iCharMult = cDefaultPatches[(i % 16) * 11 + 0];
        this->pInstruments[i].op[1].iCharMult = cDefaultPatches[(i % 16) * 11 + 1];
        this->pInstruments[i].op[0].iScalingOutput = cDefaultPatches[(i % 16) * 11 + 2];
        this->pInstruments[i].op[1].iScalingOutput = cDefaultPatches[(i % 16) * 11 + 3];
        this->pInstruments[i].op[0].iAttackDecay = cDefaultPatches[(i % 16) * 11 + 4];
        this->pInstruments[i].op[1].iAttackDecay = cDefaultPatches[(i % 16) * 11 + 5];
        this->pInstruments[i].op[0].iSustainRelease = cDefaultPatches[(i % 16) * 11 + 6];
        this->pInstruments[i].op[1].iSustainRelease = cDefaultPatches[(i % 16) * 11 + 7];
        this->pInstruments[i].op[0].iWaveSel = cDefaultPatches[(i % 16) * 11 + 8];
        this->pInstruments[i].op[1].iWaveSel = cDefaultPatches[(i % 16) * 11 + 9];
        this->pInstruments[i].iConnection = cDefaultPatches[(i % 16) * 11 + 10];
    }

    if(this->cmfHeader.iTagOffsetTitle)
    {
        f.seek(this->cmfHeader.iTagOffsetTitle);
        char c;
        while(f >> c && c != '\0')
        {
            this->strTitle += c;
        }
    }
    if(this->cmfHeader.iTagOffsetComposer)
    {
        f.seek(this->cmfHeader.iTagOffsetComposer);
        char c;
        while(f >> c && c != '\0')
        {
            this->strComposer += c;
        }
    }
    if(this->cmfHeader.iTagOffsetRemarks)
    {
        f.seek(this->cmfHeader.iTagOffsetRemarks);
        char c;
        while(f >> c && c != '\0')
        {
            this->strRemarks += c;
        }
    }

    // Load the MIDI data into memory
    f.seek(this->cmfHeader.iMusicOffset);
    this->iSongLen = f.size() - this->cmfHeader.iMusicOffset;
    this->data.resize(this->iSongLen);
    f.read(this->data.data(), this->iSongLen);

    rewind(0);

    return true;
}

bool CmfPlayer::update()
{
    // This has to be here and not in getrefresh() for some reason.
    this->iDelayRemaining = 0;

    // Read in the next event
    while(!this->iDelayRemaining)
    {
        uint8_t iCommand = this->data[this->iPlayPointer++];
        if((iCommand & 0x80) == 0)
        {
            // Running status, use previous command
            this->iPlayPointer--;
            iCommand = this->iPrevCommand;
        }
        else
        {
            this->iPrevCommand = iCommand;
        }
        uint8_t iChannel = iCommand & 0x0F;
        switch(iCommand & 0xF0)
        {
            case 0x80:
            { // Note off (two data bytes)
                uint8_t iNote = this->data[this->iPlayPointer++];
                uint8_t iVelocity = this->data[this->iPlayPointer++]; // release velocity
                this->cmfNoteOff(iChannel, iNote, iVelocity);
                break;
            }
            case 0x90:
            { // Note on (two data bytes)
                uint8_t iNote = this->data[this->iPlayPointer++];
                uint8_t iVelocity = this->data[this->iPlayPointer++]; // attack velocity
                if(iVelocity)
                {
                    this->cmfNoteOn(iChannel, iNote, iVelocity);
                }
                else
                {
                    // This is a note-off instead (velocity == 0)
                    this->cmfNoteOff(iChannel, iNote,
                                     iVelocity); // 64 is the MIDI default note-off velocity
                    break;
                }
                break;
            }
            case 0xA0:
            { // Polyphonic key pressure (two data bytes)
                uint8_t iNote = this->data[this->iPlayPointer++];
                uint8_t iPressure = this->data[this->iPlayPointer++];
                logger->debug(L4CXX_LOCATION, "CMF: Key pressure not yet implemented! (wanted ch%d/note %d set to %d)",
                              iChannel, iNote, iPressure);
                break;
            }
            case 0xB0:
            { // Controller (two data bytes)
                uint8_t iController = this->data[this->iPlayPointer++];
                uint8_t iValue = this->data[this->iPlayPointer++];
                this->MIDIcontroller(iChannel, iController, iValue);
                break;
            }
            case 0xC0:
            { // Instrument change (one data byte)
                uint8_t iNewInstrument = this->data[this->iPlayPointer++];
                this->chMIDI[iChannel].iPatch = iNewInstrument;
                logger->debug(L4CXX_LOCATION, "CMF: Remembering MIDI channel %d now uses patch %d",
                              iChannel, iNewInstrument);
                break;
            }
            case 0xD0:
            { // Channel pressure (one data byte)
                uint8_t iPressure = this->data[this->iPlayPointer++];
                logger->debug(L4CXX_LOCATION, "CMF: Channel pressure not yet implemented! (wanted ch%d set to %d)",
                              iChannel, iPressure);
                break;
            }
            case 0xE0:
            { // Pitch bend (two data bytes)
                uint8_t iLSB = this->data[this->iPlayPointer++];
                uint8_t iMSB = this->data[this->iPlayPointer++];
                uint16_t iValue = (iMSB << 7) | iLSB;
                // 8192 is middle/off, 0 is -2 semitones, 16384 is +2 semitones
                this->chMIDI[iChannel].iPitchbend = iValue;
                logger->debug(L4CXX_LOCATION, "CMF: Channel %d pitchbent to %d (%+.2f)", iChannel + 1,
                              iValue, (iValue - 8192.0f) / 8192);
                break;
            }
            case 0xF0: // System message (arbitrary data bytes)
                switch(iCommand)
                {
                    case 0xF0:
                    { // Sysex
                        uint8_t iNextByte;
                        logger->debug(L4CXX_LOCATION, "Sysex message: ");
                        do
                        {
                            iNextByte = this->data[this->iPlayPointer++];
                            logger->debug(L4CXX_LOCATION, "%02X", iNextByte);
                        } while((iNextByte & 0x80) == 0);
                        // This will have read in the terminating EOX (0xF7) message too
                        break;
                    }
                    case 0xF1: // MIDI Time Code Quarter Frame
                        this->iPlayPointer++; // message data (ignored)
                        break;
                    case 0xF2: // Song position pointer
                        this->iPlayPointer += 2; // message data (ignored)
                        break;
                    case 0xF3: // Song select
                        this->iPlayPointer++; // message data (ignored)
                        logger->debug(L4CXX_LOCATION, "CMF: MIDI Song Select is not implemented.");
                        break;
                    case 0xF6: // Tune request
                        break;
                    case 0xF7: // End of System Exclusive (EOX) - should never be read,
                        // should be absorbed by Sysex handling code
                        break;

                        // These messages are "real time", meaning they can be sent between
                        // the bytes of other messages - but we're lazy and don't handle these
                        // here (hopefully they're not necessary in a MIDI file, and even less
                        // likely to occur in a CMF.)
                    case 0xF8: // Timing clock (sent 24 times per quarter note, only when
                        // playing)
                    case 0xFA: // Start
                    case 0xFB: // Continue
                    case 0xFE: // Active sensing (sent every 300ms or MIDI connection
                        // assumed lost)
                        break;
                    case 0xFC: // Stop
                        logger->debug(L4CXX_LOCATION, "CMF: Received Real Time Stop message (0xFC)");
                        this->bSongEnd = true;
                        this->iPlayPointer = 0; // for repeat in endless-play mode
                        break;
                    case 0xFF:
                    { // System reset, used as meta-events in a MIDI file
                        uint8_t iEvent = this->data[this->iPlayPointer++];
                        switch(iEvent)
                        {
                            case 0x2F: // end of track
                                logger->debug(L4CXX_LOCATION, "CMF: End-of-track, stopping playback");
                                this->bSongEnd = true;
                                this->iPlayPointer = 0; // for repeat in endless-play mode
                                break;
                            default:
                                logger->debug(L4CXX_LOCATION, "CMF: Unknown MIDI meta-event 0xFF 0x%02X", int(iEvent));
                                break;
                        }
                        break;
                    }
                    default:
                        logger->debug(L4CXX_LOCATION, "CMF: Unknown MIDI system command 0x%02X\n", int(iCommand));
                        break;
                }
                break;
            default:
                logger->debug(L4CXX_LOCATION, "CMF: Unknown MIDI command 0x%02X\n", int(iCommand));
                break;
        }

        if(this->iPlayPointer >= this->iSongLen)
        {
            this->bSongEnd = true;
            this->iPlayPointer = 0; // for repeat in endless-play mode
        }

        // Read in the number of ticks until the next event
        this->iDelayRemaining = this->readMIDINumber();
    }

    return !this->bSongEnd;
}

void CmfPlayer::rewind(int)
{
    // Initialise

    // Enable use of WaveSel register on OPL3 (even though we're only an OPL2!)
    // Apparently this enables nine-channel mode?
    this->writeOPL(0x01, 0x20);

    // Disable OPL3 mode (can be left enabled by a previous non-CMF song)
    this->writeOPL(0x05, 0x00);

    // Really make sure CSM+SEL are off (again, Creative's player...)
    this->writeOPL(0x08, 0x00);

    // This freq setting is required for the hihat to sound correct at the start
    // of funky.cmf, even though it's for an unrelated channel.
    // If it's here however, it makes the hihat in Word Rescue's theme.cmf
    // sound really bad.
    // TODO: How do we figure out whether we need it or not???
    this->writeOPL(BASE_FNUM_L + 8, 514 & 0xFF);
    this->writeOPL(BASE_KEYON_FREQ + 8, (1 << 2) | (514 >> 8));

    // default freqs?
    this->writeOPL(BASE_FNUM_L + 7, 509 & 0xFF);
    this->writeOPL(BASE_KEYON_FREQ + 7, (2 << 2) | (509 >> 8));
    this->writeOPL(BASE_FNUM_L + 6, 432 & 0xFF);
    this->writeOPL(BASE_KEYON_FREQ + 6, (2 << 2) | (432 >> 8));

    // Amplify AM + VIB depth.  Creative's CMF player does this, and there
    // doesn't seem to be any way to stop it from doing so - except for the
    // non-standard controller 0x63 I added :-)
    this->writeOPL(0xBD, 0xC0);

    this->bSongEnd = false;
    this->iPlayPointer = 0;
    this->iPrevCommand = 0; // just in case

    // Read in the number of ticks until the first event
    this->iDelayRemaining = this->readMIDINumber();

    // Reset song state.  This used to be in the constructor, but the XMMS2
    // plugin sets the song length before starting playback.  AdPlug plays the
    // song in its entirety (with no synth) to determine the song length, which
    // results in the state variables below matching the end of the song.  When
    // the real OPL synth is activated for playback, it no longer matches the
    // state variables and the instruments are not set correctly!
    for(int i = 0; i < 9; i++)
    {
        this->chOPL[i].iNoteStart = 0; // no note playing atm
        this->chOPL[i].iMIDINote = -1;
        this->chOPL[i].iMIDIChannel = -1;
        this->chOPL[i].iMIDIPatch = -1;

        this->chMIDI[i].iPatch = -2;
        this->chMIDI[i].iPitchbend = 8192;
    }
    for(int i = 9; i < 16; i++)
    {
        this->chMIDI[i].iPatch = -2;
        this->chMIDI[i].iPitchbend = 8192;
    }

    memset(this->iCurrentRegs, 0, 256);
}

// Return value: 1 == 1 second, 2 == 0.5 seconds
size_t CmfPlayer::framesUntilUpdate() const
{
    if(this->iDelayRemaining)
    {
        return SampleRate * iDelayRemaining / cmfHeader.iTicksPerSecond;
    }
    else
    {
        // Delay-remaining is zero (e.g. start of song) so use a tiny delay
        return SampleRate / cmfHeader.iTicksPerSecond; // wait for one tick
    }
}

std::string CmfPlayer::title() const
{
    return this->strTitle;
}

std::string CmfPlayer::author() const
{
    return this->strComposer;
}

std::string CmfPlayer::description() const
{
    return this->strRemarks;
}

//
// PROTECTED
//

// Read a variable-length integer from MIDI data
uint32_t CmfPlayer::readMIDINumber()
{
    uint32_t iValue = 0;
    for(int i = 0; i < 4; i++)
    {
        uint8_t iNext = this->data[this->iPlayPointer++];
        iValue <<= 7;
        iValue |= (iNext & 0x7F); // ignore the MSB
        if((iNext & 0x80) == 0)
            break; // last byte has the MSB unset
    }
    return iValue;
}

// iChannel: OPL channel (0-8)
// iOperator: 0 == Modulator, 1 == Carrier
//   Source - source operator to read from instrument definition
//   Dest - destination operator on OPL chip
// iInstrument: Index into this->pInstruments array of CMF instruments
void CmfPlayer::writeInstrumentSettings(uint8_t iChannel,
                                         uint8_t iOperatorSource,
                                         uint8_t iOperatorDest,
                                         uint8_t iInstrument)
{
    assert(iChannel <= 8);

    uint8_t iOPLOffset = OPLOFFSET(iChannel);
    if(iOperatorDest)
        iOPLOffset += 3; // Carrier if iOperator == 1 (else Modulator)

    this->writeOPL(BASE_CHAR_MULT + iOPLOffset,
                   this->pInstruments[iInstrument].op[iOperatorSource].iCharMult);
    this->writeOPL(
        BASE_SCAL_LEVL + iOPLOffset,
        this->pInstruments[iInstrument].op[iOperatorSource].iScalingOutput);
    this->writeOPL(
        BASE_ATCK_DCAY + iOPLOffset,
        this->pInstruments[iInstrument].op[iOperatorSource].iAttackDecay);
    this->writeOPL(
        BASE_SUST_RLSE + iOPLOffset,
        this->pInstruments[iInstrument].op[iOperatorSource].iSustainRelease);
    this->writeOPL(BASE_WAVE + iOPLOffset,
                   this->pInstruments[iInstrument].op[iOperatorSource].iWaveSel);

    // TODO: Check to see whether we should only be loading this for one or both
    // operators
    this->writeOPL(BASE_FEED_CONN + iChannel,
                   this->pInstruments[iInstrument].iConnection);
}

// Write a byte to the OPL "chip" and update the current record of register
// states
void CmfPlayer::writeOPL(uint8_t iRegister, uint8_t iValue)
{
    this->getOpl()->writeReg(iRegister, iValue);
    this->iCurrentRegs[iRegister] = iValue;
}

void CmfPlayer::cmfNoteOn(uint8_t iChannel, uint8_t iNote, uint8_t iVelocity)
{
    uint8_t iBlock = iNote / 12;
    if(iBlock > 1)
        iBlock--; // keep in the same range as the Creative player
    //if (iBlock > 7) iBlock = 7; // don't want to go out of range

    double d =
        pow(2, (iNote + ((this->chMIDI[iChannel].iPitchbend - 8192) /
                         8192.0) + (this->iTranspose / 128) - 9) / 12.0 -
                         (iBlock - 20)) * 440.0 / 32.0 / 50000.0;
    uint16_t iOPLFNum = static_cast<uint16_t>(d + 0.5);
    if(iOPLFNum > 1023)
        logger->error(L4CXX_LOCATION, "CMF: This note is out of range! (send this song to malvineous@shikadi.net!)");

    // See if we're playing a rhythm mode percussive instrument
    if((iChannel > 10) && (this->bPercussive))
    {
        uint8_t iPercChannel = this->getPercChannel(iChannel);

        // Will have to set every time (easier) than figuring out whether the mod
        // or car needs to be changed.
        //if (this->chOPL[iPercChannel].iMIDIPatch != this->chMIDI[iChannel].iPatch)
        //{
        this->MIDIchangeInstrument(iPercChannel, iChannel,
                                   this->chMIDI[iChannel].iPatch);
        //}

        /*  Velocity calculations - TODO: Work out the proper formula

        iVelocity -> iLevel  (values generated by Creative's player)
        7f -> 00
        7c -> 00

        7b -> 09
        73 -> 0a
        6b -> 0b
        63 -> 0c
        5b -> 0d
        53 -> 0e
        4b -> 0f
        43 -> 10
        3b -> 11
        33 -> 13
        2b -> 15
        23 -> 19
        1b -> 1b
        13 -> 1d
        0b -> 1f
        03 -> 21

        02 -> 21
        00 -> N/A (note off)
        */
        // Approximate formula, need to figure out more accurate one (my maths isn't
        // so good...)
        int iLevel =
            0x25 - sqrt(iVelocity * 16 /*6*/); //(127 - iVelocity) * 0x20 / 127;
        if(iVelocity > 0x7b)
            iLevel = 0; // full volume
        if(iLevel < 0)
            iLevel = 0;
        if(iLevel > 0x3F)
            iLevel = 0x3F;
        //if (iVelocity < 0x40) iLevel = 0x10;

        int iOPLOffset = BASE_SCAL_LEVL + OPLOFFSET(iPercChannel);
        //if ((iChannel == 11) || (iChannel == 12) || (iChannel == 14)) {
        if(iChannel == 11)
            iOPLOffset += 3; // only do bassdrum carrier for volume control
        //iOPLOffset += 3; // carrier
        this->writeOPL(iOPLOffset, (this->iCurrentRegs[iOPLOffset] & ~0x3F) |
                       iLevel); //(iVelocity * 0x3F / 127));
        //}
        // Bass drum (ch11) uses both operators
        //if (iChannel == 11) this->writeOPL(iOPLOffset + 3,
        //(this->iCurrentRegs[iOPLOffset + 3] & ~0x3F) | iLevel);

        /*		#ifdef USE_VELOCITY  // Official CMF player seems to ignore velocity
    levels
                uint16_t iLevel = 0x2F - (iVelocity * 0x2F / 127); // 0x2F should be 0x3F
    but it's too quiet then
                AdPlug_LogWrite("%02X + vel %d (lev %02X) == %02X\n",
    this->iCurrentRegs[iOPLOffset], iVelocity, iLevel,
    (this->iCurrentRegs[iOPLOffset] & ~0x3F) | iLevel);
                //this->writeOPL(iOPLOffset, (this->iCurrentRegs[iOPLOffset] & ~0x3F) |
    (0x3F - (iVelocity >> 1)));//(iVelocity * 0x3F / 127));
                this->writeOPL(iOPLOffset, (this->iCurrentRegs[iOPLOffset] & ~0x3F) |
    iLevel);//(iVelocity * 0x3F / 127));
            #endif*/

            // Apparently you can't set the frequency for the cymbal or hihat?
            // Vinyl requires you don't set it, Kiloblaster requires you do!
        this->writeOPL(BASE_FNUM_L + iPercChannel, iOPLFNum & 0xFF);
        this->writeOPL(BASE_KEYON_FREQ + iPercChannel,
            (iBlock << 2) | ((iOPLFNum >> 8) & 0x03));

        uint8_t iBit = 1 << (15 - iChannel);

        // Turn the perc instrument off if it's already playing (OPL can't do
        // polyphonic notes w/ percussion)
        if(this->iCurrentRegs[BASE_RHYTHM] & iBit)
            this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] & ~iBit);

        // I wonder whether we need to delay or anything here?

        // Turn the note on
        //if (iChannel == 15) {
        this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] | iBit);
        //AdPlug_LogWrite("CMF: Note %d on MIDI channel %d (mapped to OPL channel
        //%d-1) - vel %02X, fnum %d/%d\n", iNote, iChannel, iPercChannel+1,
        //iVelocity, iOPLFNum, iBlock);
        //}

        this->chOPL[iPercChannel].iNoteStart = ++this->iNoteCount;
        this->chOPL[iPercChannel].iMIDIChannel = iChannel;
        this->chOPL[iPercChannel].iMIDINote = iNote;
    }
    else
    { // Non rhythm-mode or a normal instrument channel
        // Figure out which OPL channel to play this note on
        int iOPLChannel = -1;
        int iNumChannels = this->bPercussive ? 6 : 9;
        for(int i = iNumChannels - 1; i >= 0; i--)
        {
            // If there's no note playing on this OPL channel, use that
            if(this->chOPL[i].iNoteStart == 0)
            {
                iOPLChannel = i;
                // See if this channel is already set to the instrument we want.
                if(this->chOPL[i].iMIDIPatch == this->chMIDI[iChannel].iPatch)
                {
                    // It is, so stop searching
                    break;
                } // else keep searching just in case there's a better match
            }
        }
        if(iOPLChannel == -1)
        {
            // All channels were in use, find the one with the longest note
            iOPLChannel = 0;
            int iEarliest = this->chOPL[0].iNoteStart;
            for(int i = 1; i < iNumChannels; i++)
            {
                if(this->chOPL[i].iNoteStart < iEarliest)
                {
                    // Found a channel with a note being played for longer
                    iOPLChannel = i;
                    iEarliest = this->chOPL[i].iNoteStart;
                }
            }
            logger->warn(L4CXX_LOCATION, "CMF: Too many polyphonic notes, cutting note on channel %d",
                         iOPLChannel);
        }

        // Run through all the channels with negative notestart values - these
        // channels have had notes recently stop - and increment the counter
        // to slowly move the channel closer to being reused for a future note.
        //for (int i = 0; i < iNumChannels; i++) {
        //	if (this->chOPL[i].iNoteStart < 0) this->chOPL[i].iNoteStart++;
        //}

        // Now the new note should be played on iOPLChannel, but see if the
        // instrument
        // is right first.
        if(this->chOPL[iOPLChannel].iMIDIPatch != this->chMIDI[iChannel].iPatch)
        {
            this->MIDIchangeInstrument(iOPLChannel, iChannel,
                                       this->chMIDI[iChannel].iPatch);
        }

        this->chOPL[iOPLChannel].iNoteStart = ++this->iNoteCount;
        this->chOPL[iOPLChannel].iMIDIChannel = iChannel;
        this->chOPL[iOPLChannel].iMIDINote = iNote;

#ifdef USE_VELOCITY // Official CMF player seems to ignore velocity levels
        // Adjust the channel volume to match the note velocity
        uint8_t iOPLOffset =
            BASE_SCAL_LEVL + OPLOFFSET(iChannel) + 3; // +3 == Carrier
        uint16_t iLevel =
            0x2F -
            (iVelocity * 0x2F / 127); // 0x2F should be 0x3F but it's too quiet then
        this->writeOPL(iOPLOffset,
            (this->iCurrentRegs[iOPLOffset] & ~0x3F) | iLevel);
#endif

        // Set the frequency and play the note
        this->writeOPL(BASE_FNUM_L + iOPLChannel, iOPLFNum & 0xFF);
        this->writeOPL(BASE_KEYON_FREQ + iOPLChannel,
                       OPLBIT_KEYON | (iBlock << 2) | ((iOPLFNum & 0x300) >> 8));
    }
}

void CmfPlayer::cmfNoteOff(uint8_t iChannel, uint8_t iNote, uint8_t)
{
    if((iChannel > 10) && (this->bPercussive))
    {
        int iOPLChannel = this->getPercChannel(iChannel);
        if(this->chOPL[iOPLChannel].iMIDINote != iNote)
            return; // there's a different note playing now
        this->writeOPL(BASE_RHYTHM,
                       this->iCurrentRegs[BASE_RHYTHM] & ~(1 << (15 - iChannel)));
        this->chOPL[iOPLChannel].iNoteStart = 0; // channel free
    }
    else
    { // Non rhythm-mode or a normal instrument channel
        int iOPLChannel = -1;
        int iNumChannels = this->bPercussive ? 6 : 9;
        for(int i = 0; i < iNumChannels; i++)
        {
            if((this->chOPL[i].iMIDIChannel == iChannel) &&
                (this->chOPL[i].iMIDINote == iNote) &&
               (this->chOPL[i].iNoteStart != 0))
            {
                // Found the note, switch it off
                this->chOPL[i].iNoteStart = 0;
                iOPLChannel = i;
                break;
            }
        }
        if(iOPLChannel == -1)
            return;

        this->writeOPL(
            BASE_KEYON_FREQ + iOPLChannel,
            this->iCurrentRegs[BASE_KEYON_FREQ + iOPLChannel] & ~OPLBIT_KEYON);
    }
}

uint8_t CmfPlayer::getPercChannel(uint8_t iChannel)
{
    switch(iChannel)
    {
        case 11:
            return 7 - 1; // Bass drum
        case 12:
            return 8 - 1; // Snare drum
        case 13:
            return 9 - 1; // Tom tom
        case 14:
            return 9 - 1; // Top cymbal
        case 15:
            return 8 - 1; // Hihat
    }
    logger->error(L4CXX_LOCATION, "CMF ERR: Tried to get the percussion channel from MIDI channel %d - this shouldn't happen!",
                  iChannel);
    return 0;
}

void CmfPlayer::MIDIchangeInstrument(uint8_t iOPLChannel, uint8_t iMIDIChannel,
                                      uint8_t iNewInstrument)
{
    if((iMIDIChannel > 10) && (this->bPercussive))
    {
        switch(iMIDIChannel)
        {
            case 11: // Bass drum (operator 13+16 == channel 7 modulator+carrier)
                this->writeInstrumentSettings(7 - 1, 0, 0, iNewInstrument);
                this->writeInstrumentSettings(7 - 1, 1, 1, iNewInstrument);
                break;
            case 12: // Snare drum (operator 17 == channel 8 carrier)
                //case 15:
                this->writeInstrumentSettings(8 - 1, 0, 1, iNewInstrument);

                //
                //this->writeInstrumentSettings(8-1, 0, 0, iNewInstrument);
                break;
            case 13: // Tom tom (operator 15 == channel 9 modulator)
                //case 14:
                this->writeInstrumentSettings(9 - 1, 0, 0, iNewInstrument);

                //
                //this->writeInstrumentSettings(9-1, 0, 1, iNewInstrument);
                break;
            case 14: // Top cymbal (operator 18 == channel 9 carrier)
                this->writeInstrumentSettings(9 - 1, 0, 1, iNewInstrument);
                break;
            case 15: // Hi-hat (operator 14 == channel 8 modulator)
                this->writeInstrumentSettings(8 - 1, 0, 0, iNewInstrument);
                break;
            default:
                logger->warn(L4CXX_LOCATION, "CMF: Invalid MIDI channel %d (not melodic and not percussive!)",
                             iMIDIChannel + 1);
                break;
        }
        this->chOPL[iOPLChannel].iMIDIPatch = iNewInstrument;
    }
    else
    {
        // Standard nine OPL channels
        this->writeInstrumentSettings(iOPLChannel, 0, 0, iNewInstrument);
        this->writeInstrumentSettings(iOPLChannel, 1, 1, iNewInstrument);
        this->chOPL[iOPLChannel].iMIDIPatch = iNewInstrument;
    }
}

void CmfPlayer::MIDIcontroller(uint8_t, uint8_t iController, uint8_t iValue)
{
    switch(iController)
    {
        case 0x63:
            // Custom extension to allow CMF files to switch the AM+VIB depth on and
            // off (officially both are on, and there's no way to switch them off.)
            // Controller values:
            //   0 == AM+VIB off
            //   1 == VIB on
            //   2 == AM on
            //   3 == AM+VIB on
            if(iValue)
            {
                this->writeOPL(BASE_RHYTHM,
                    (this->iCurrentRegs[BASE_RHYTHM] & ~0xC0) |
                               (iValue << 6)); // switch AM+VIB extension on
            }
            else
            {
                this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] &
                               ~0xC0); // switch AM+VIB extension off
            }
            logger->debug(L4CXX_LOCATION, "CMF: AM+VIB depth change - AM %s, VIB %s",
                (this->iCurrentRegs[BASE_RHYTHM] & 0x80) ? "on" : "off",
                          (this->iCurrentRegs[BASE_RHYTHM] & 0x40) ? "on" : "off");
            break;
        case 0x66:
            logger->debug(L4CXX_LOCATION, "CMF: Song set marker to 0x%02X", int(iValue));
            break;
        case 0x67:
            this->bPercussive = (iValue != 0);
            if(this->bPercussive)
            {
                this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] |
                               0x20); // switch rhythm-mode on
            }
            else
            {
                this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] &
                               ~0x20); // switch rhythm-mode off
            }
            logger->debug(L4CXX_LOCATION, "CMF: Percussive/rhythm mode %s",
                          this->bPercussive ? "enabled" : "disabled");
            break;
        case 0x68:
            // TODO: Shouldn't this just affect the one channel, not the whole song?
            // -- have pitchbends for that
            this->iTranspose = iValue;
            logger->debug(L4CXX_LOCATION, "CMF: Transposing all notes up by %d * 1/128ths of a semitone.",
                          int(iValue));
            break;
        case 0x69:
            this->iTranspose = -iValue;
            logger->debug(L4CXX_LOCATION, "CMF: Transposing all notes down by %d * 1/128ths of a semitone.",
                          int(iValue));
            break;
        default:
            logger->debug(L4CXX_LOCATION, "CMF: Unsupported MIDI controller 0x%02X, ignoring.",
                          int(iController));
            break;
    }
}