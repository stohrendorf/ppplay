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
 * d00.c - D00 Player by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * Sorry for the goto's, but the code looks so much nicer now.
 * I tried it with while loops but it was just a mess. If you
 * can come up with a nicer solution, just tell me.
 *
 * BUGS:
 * Hard restart SR is sometimes wrong
 */

#include <cstdio>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "stream/filestream.h"

#include "d00.h"

#define HIBYTE(val) (val >> 8)
#define LOBYTE(val) (val & 0xff)

static const unsigned short notetable[12] = // D00 note table
{ 340, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647 };

static inline uint16_t LE_WORD(const uint16_t *val) {
    const uint8_t *b = reinterpret_cast<const uint8_t *>(val);
    return (b[1] << 8) + b[0];
}

/*** public methods *************************************/

CPlayer *Cd00Player::factory() { return new Cd00Player(); }

bool Cd00Player::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;

    // file validation section
    d00header checkhead;
    f >> checkhead;

    bool ver1 = false;
    // Check for version 2-4 header
    if (strncmp(checkhead.id, "JCH\x26\x02\x66", 6) || checkhead.type || !checkhead.subsongs || checkhead.soundcard) {
        // Check for version 0 or 1 header (and .d00 file extension)
        if(f.extension() != ".d00") {
            return false;
        }
        f.seek(0);
        d00header1 ch;
        f >> ch;
        if (ch.version > 1 || !ch.subsongs) {
            return false;
        }
        ver1 = true;
    }

    // load section
    auto filesize = f.size();
    f.seek(0);
    filedata.resize(filesize+1); // 1 byte is needed for old-style DataInfo block
    f.read(filedata.data(), filesize);
    if (!ver1) { // version 2 and above
        header = reinterpret_cast<d00header*>(filedata.data());
        version = header->version;
        datainfo = filedata.data() + LE_WORD(&header->infoptr);
        inst = reinterpret_cast<const Sinsts *>(filedata.data() + LE_WORD(&header->instptr));
        seqptr = reinterpret_cast<const uint16_t*>(filedata.data() + LE_WORD(&header->seqptr));
        for (int i = 31; i >= 0; i--) { // erase whitespace
            if (header->songname[i] == ' ')
                header->songname[i] = '\0';
            else
                break;
        }
        for (int i = 31; i >= 0; i--) {
            if (header->author[i] == ' ')
                header->author[i] = '\0';
            else
                break;
        }
    }
    else { // version 1
        header1 = reinterpret_cast<d00header1*>(filedata.data());
        version = header1->version;
        datainfo = filedata.data() + LE_WORD(&header1->infoptr);
        inst = reinterpret_cast<const Sinsts*>(filedata.data() + LE_WORD(&header1->instptr));
        seqptr = reinterpret_cast<const uint16_t*>(filedata.data() + LE_WORD(&header1->seqptr));
    }
    switch (version) {
    case 0:
        levpuls = nullptr;
        spfx = nullptr;
        header1->speed = 70; // v0 files default to 70Hz
        break;
    case 1:
        levpuls = reinterpret_cast<const Slevpuls*>(filedata.data() + LE_WORD(&header1->lpulptr));
        spfx = nullptr;
        break;
    case 2:
        levpuls = reinterpret_cast<const Slevpuls*>(filedata.data() + LE_WORD(&header->spfxptr));
        spfx = nullptr;
        break;
    case 3:
        spfx = nullptr;
        levpuls = nullptr;
        break;
    case 4:
        spfx = reinterpret_cast<const Sspfx*>(filedata.data() + LE_WORD(&header->spfxptr));
        levpuls = nullptr;
        break;
    }
    if (auto str = strstr(datainfo, "\xff\xff")) {
        while ((*str == '\xff' || *str == ' ') && str >= datainfo) {
            *str = '\0';
            str--;
        }
    }
    else // old-style block
        filedata.back() = 0;

    addOrder(0);

    rewind(0);
    return true;
}

bool Cd00Player::update() {
    // effect handling (timer dependant)
    for (auto c = 0; c < 9; c++) {
        channel[c].slideval += channel[c].slide;
        setfreq(c); // sliding
        vibrato(c); // vibrato

        if (channel[c].spfx != 0xffff) { // SpFX
            if (channel[c].fxdel) {
                channel[c].fxdel--;
            }
            else {
                channel[c].spfx = LE_WORD(&spfx[channel[c].spfx].ptr);
                channel[c].fxdel = spfx[channel[c].spfx].duration;
                channel[c].inst = LE_WORD(&spfx[channel[c].spfx].instnr) & 0xfff;
                if (spfx[channel[c].spfx].modlev != 0xff)
                    channel[c].modvol = spfx[channel[c].spfx].modlev;
                setinst(c);
                uint8_t note;
                if (LE_WORD(&spfx[channel[c].spfx].instnr) & 0x8000) // locked frequency
                    note = spfx[channel[c].spfx].halfnote;
                else // unlocked frequency
                    note = spfx[channel[c].spfx].halfnote + channel[c].note;
                channel[c].freq = notetable[note % 12] + ((note / 12) << 10);
                setfreq(c);
            }
            channel[c].modvol += spfx[channel[c].spfx].modlevadd;
            channel[c].modvol &= 63;
            setvolume(c);
        }

        if (channel[c].levpuls != 0xff) { // Levelpuls
            if (channel[c].frameskip) {
                channel[c].frameskip--;
            }
            else {
                channel[c].frameskip = inst[channel[c].inst].timer;
                if (channel[c].fxdel)
                    channel[c].fxdel--;
                else {
                    channel[c].levpuls = levpuls[channel[c].levpuls].ptr - 1;
                    channel[c].fxdel = levpuls[channel[c].levpuls].duration;
                    if (levpuls[channel[c].levpuls].level != 0xff)
                        channel[c].modvol = levpuls[channel[c].levpuls].level;
                }
                channel[c].modvol += levpuls[channel[c].levpuls].voladd;
                channel[c].modvol &= 63;
                setvolume(c);
            }
        }
    }

    // song handling
    for (auto c = 0; c < 9; c++)
        if (version < 3 ? channel[c].del : channel[c].del <= 0x7f) {
            if (version == 4) // v4: hard restart SR
                if (channel[c].del == inst[channel[c].inst].timer)
                    if (channel[c].nextnote)
                        getOpl()->writeReg(0x83 + s_opTable[c], inst[channel[c].inst].sr);
            if (version < 3)
                channel[c].del--;
            else if (channel[c].speed)
                channel[c].del += channel[c].speed;
            else {
                channel[c].seqend = 1;
                continue;
            }
        }
        else {
            if (channel[c].speed) {
                if (version < 3)
                    channel[c].del = channel[c].speed;
                else {
                    channel[c].del &= 0x7f;
                    channel[c].del += channel[c].speed;
                }
            }
            else {
                channel[c].seqend = 1;
                continue;
            }
            if (channel[c].rhcnt) { // process pending REST/HOLD events
                channel[c].rhcnt--;
                continue;
            }
readorder: // process arrangement (orderlist)
            auto ord = LE_WORD(&channel[c].order[channel[c].ordpos]);
            const uint16_t* patt = nullptr;
            switch (ord) {
            case 0xfffe:
                channel[c].seqend = 1;
                continue;  // end of arrangement stream
            case 0xffff: // jump to order
                channel[c].ordpos = LE_WORD(&channel[c].order[channel[c].ordpos + 1]);
                channel[c].seqend = 1;
                goto readorder;
            default:
                if (ord >= 0x9000) { // set speed
                    channel[c].speed = ord & 0xff;
                    ord = LE_WORD(&channel[c].order[channel[c].ordpos - 1]);
                    channel[c].ordpos++;
                }
                else if (ord >= 0x8000) { // transpose track
                    channel[c].transpose = ord & 0xff;
                    if (ord & 0x100)
                        channel[c].transpose = -channel[c].transpose;
                    ord = LE_WORD(&channel[c].order[++channel[c].ordpos]);
                }
                patt = reinterpret_cast<const uint16_t*>(filedata.data() + LE_WORD(&seqptr[ord]));
                break;
            }
            channel[c].fxflag = 0;
readseq:        // process sequence (pattern)
            if (!version) // v0: always initialize rhcnt
                channel[c].rhcnt = channel[c].irhcnt;
            auto pattpos = LE_WORD(&patt[channel[c].pattpos]);
            if (pattpos == 0xffff) { // pattern ended?
                channel[c].pattpos = 0;
                channel[c].ordpos++;
                goto readorder;
            }
            auto cnt = HIBYTE(pattpos);
            auto note = LOBYTE(pattpos);
            const auto fx = pattpos >> 12;
            const auto fxop = pattpos & 0x0fff;
            channel[c].pattpos++;
            pattpos = LE_WORD(&patt[channel[c].pattpos]);
            channel[c].nextnote = LOBYTE(pattpos) & 0x7f;
            if (version ? cnt < 0x40 : !fx) { // note event
                switch (note) {
                case 0: // REST event
                case 0x80:
                    if (!note || version) {
                        channel[c].key = 0;
                        setfreq(c);
                    }
                    // fall through...
                case 0x7e: // HOLD event
                    if (version)
                        channel[c].rhcnt = cnt;
                    channel[c].nextnote = 0;
                    break;
                default: // play note
                    // restart fx
                    if (!(channel[c].fxflag & 1))
                        channel[c].vibdepth = 0;
                    if (!(channel[c].fxflag & 2))
                        channel[c].slideval = channel[c].slide = 0;

                    if (version) {     // note handling for v1 and above
                        if (note > 0x80) // locked note (no channel transpose)
                            note -= 0x80;
                        else // unlocked note
                            note += channel[c].transpose;
                        channel[c].note = note; // remember note for SpFX

                        if (channel[c].ispfx != 0xffff && cnt < 0x20) { // reset SpFX
                            channel[c].spfx = channel[c].ispfx;
                            if (LE_WORD(&spfx[channel[c].spfx].instnr) &
                                    0x8000) // locked frequency
                                note = spfx[channel[c].spfx].halfnote;
                            else // unlocked frequency
                                note += spfx[channel[c].spfx].halfnote;
                            channel[c].inst = LE_WORD(&spfx[channel[c].spfx].instnr) & 0xfff;
                            channel[c].fxdel = spfx[channel[c].spfx].duration;
                            if (spfx[channel[c].spfx].modlev != 0xff)
                                channel[c].modvol = spfx[channel[c].spfx].modlev;
                            else
                                channel[c].modvol = inst[channel[c].inst].data[7] & 63;
                        }

                        if (channel[c].ilevpuls != 0xff && cnt < 0x20) { // reset LevelPuls
                            channel[c].levpuls = channel[c].ilevpuls;
                            channel[c].fxdel = levpuls[channel[c].levpuls].duration;
                            channel[c].frameskip = inst[channel[c].inst].timer;
                            if (levpuls[channel[c].levpuls].level != 0xff)
                                channel[c].modvol = levpuls[channel[c].levpuls].level;
                            else
                                channel[c].modvol = inst[channel[c].inst].data[7] & 63;
                        }

                        channel[c].freq = notetable[note % 12] + ((note / 12) << 10);
                        if (cnt < 0x20) // normal note
                            playnote(c);
                        else { // tienote
                            setfreq(c);
                            cnt -= 0x20; // make count proper
                        }
                        channel[c].rhcnt = cnt;
                    } else {       // note handling for v0
                        if (cnt < 2) // unlocked note
                            note += channel[c].transpose;
                        channel[c].note = note;

                        channel[c].freq = notetable[note % 12] + ((note / 12) << 10);
                        if (cnt == 1) // tienote
                            setfreq(c);
                        else // normal note
                            playnote(c);
                    }
                    break;
                }
                continue; // event is complete
            }
            else {    // effect event
                switch (fx) {
                case 6: { // Cut/Stop Voice
                    const auto buf = channel[c].inst;
                    channel[c].inst = 0;
                    playnote(c);
                    channel[c].inst = buf;
                    channel[c].rhcnt = fxop;
                    continue; // no note follows this event
                }
                case 7:     // Vibrato
                    channel[c].vibspeed = fxop & 0xff;
                    channel[c].vibdepth = fxop >> 8;
                    channel[c].trigger = fxop >> 9;
                    channel[c].fxflag |= 1;
                    break;
                case 8: // v0: Duration
                    if (!version)
                        channel[c].irhcnt = fxop;
                    break;
                case 9: // New Level
                    channel[c].vol = fxop & 63;
                    if (channel[c].vol + channel[c].cvol < 63) // apply channel volume
                        channel[c].vol += channel[c].cvol;
                    else
                        channel[c].vol = 63;
                    setvolume(c);
                    break;
                case 0xb: // v4: Set SpFX
                    if (version == 4)
                        channel[c].ispfx = fxop;
                    break;
                case 0xc: // Set Instrument
                    channel[c].ispfx = 0xffff;
                    channel[c].spfx = 0xffff;
                    channel[c].inst = fxop;
                    channel[c].modvol = inst[fxop].data[7] & 63;
                    if (version < 3 && version && inst[fxop].tunelev) // Set LevelPuls
                        channel[c].ilevpuls = inst[fxop].tunelev - 1;
                    else {
                        channel[c].ilevpuls = 0xff;
                        channel[c].levpuls = 0xff;
                    }
                    break;
                case 0xd: // Slide up
                    channel[c].slide = fxop;
                    channel[c].fxflag |= 2;
                    break;
                case 0xe: // Slide down
                    channel[c].slide = -fxop;
                    channel[c].fxflag |= 2;
                    break;
                }
                goto readseq; // event is incomplete, note follows
            }
        }

    int trackend = 0;
    for (auto c = 0; c < 9; c++)
        if (channel[c].seqend)
            trackend++;
    if (trackend == 9)
        songend = true;

    return !songend;
}

void Cd00Player::rewind(int subsong) {
    struct Stpoin {
        unsigned short ptr[9];
        unsigned char volume[9], dummy[5];
    };

    if (subsong == -1)
        subsong = cursubsong;

    if (version > 1) { // do nothing if subsong > number of subsongs
        if (subsong >= header->subsongs)
            return;
    } else if (subsong >= header1->subsongs)
        return;

    memset(channel, 0, sizeof(channel));
    const Stpoin* tpoin;
    if (version > 1)
        tpoin = reinterpret_cast<const Stpoin*>(filedata.data() + LE_WORD(&header->tpoin));
    else
        tpoin = reinterpret_cast<const Stpoin*>(filedata.data() + LE_WORD(&header1->tpoin));
    for (int i = 0; i < 9; i++) {
        if (LE_WORD(&tpoin[subsong].ptr[i])) { // track enabled
            channel[i].speed = LE_WORD(reinterpret_cast<const uint16_t*>(filedata.data() + LE_WORD(&tpoin[subsong].ptr[i])));
            channel[i].order = reinterpret_cast<const uint16_t*>(filedata.data() + LE_WORD(&tpoin[subsong].ptr[i]) + 2);
        }
        else { // track disabled
            channel[i].speed = 0;
            channel[i].order = nullptr;
        }
        channel[i].ispfx = 0xffff;
        channel[i].spfx = 0xffff; // no SpFX
        channel[i].ilevpuls = 0xff;
        channel[i].levpuls = 0xff;           // no LevelPuls
        channel[i].cvol = tpoin[subsong].volume[i] & 0x7f; // our player may savely ignore bit 7
        channel[i].vol = channel[i].cvol;    // initialize volume
    }
    songend = false;
    getOpl()->writeReg(1, 32); // reset OPL chip
    cursubsong = subsong;
}

std::string Cd00Player::type() const
{
    char tmpstr[40];

    sprintf(tmpstr, "EdLib packed (version %d)", version > 1 ? header->version : header1->version);
    return tmpstr;
}

size_t Cd00Player::framesUntilUpdate() const
{
    if (version > 1)
        return SampleRate / header->speed;
    else
        return SampleRate / header1->speed;
}

unsigned int Cd00Player::subSongCount() const
{
    if (version <= 1) // return number of subsongs
        return header1->subsongs;
    else
        return header->subsongs;
}

/*** private methods *************************************/

void Cd00Player::setvolume(unsigned char chan) {
    unsigned char op = s_opTable[chan];
    unsigned short insnr = channel[chan].inst;

    getOpl()->writeReg(0x43 + op, static_cast<uint8_t>(63 - ((63 - (inst[insnr].data[2] & 63)) / 63.0) * (63 - channel[chan].vol)) +
            (inst[insnr].data[2] & 192));
    if (inst[insnr].data[10] & 1)
        getOpl()->writeReg( 0x40 + op, static_cast<uint8_t>(63 - ((63 - channel[chan].modvol) / 63.0) * (63 - channel[chan].vol)) + (inst[insnr].data[7] & 192) );
    else
        getOpl()->writeReg(0x40 + op, channel[chan].modvol + (inst[insnr].data[7] & 192));
}

void Cd00Player::setfreq(unsigned char chan) {
    unsigned short freq = channel[chan].freq;

    if (version == 4) // v4: apply instrument finetune
        freq += inst[channel[chan].inst].tunelev;

    freq += channel[chan].slideval;
    getOpl()->writeReg(0xa0 + chan, freq & 255);
    if (channel[chan].key)
        getOpl()->writeReg(0xb0 + chan, ((freq >> 8) & 31) | 32);
    else
        getOpl()->writeReg(0xb0 + chan, (freq >> 8) & 31);
}

void Cd00Player::setinst(unsigned char chan) {
    unsigned char op = s_opTable[chan];
    unsigned short insnr = channel[chan].inst;

    // set instrument data
    getOpl()->writeReg(0x63 + op, inst[insnr].data[0]);
    getOpl()->writeReg(0x83 + op, inst[insnr].data[1]);
    getOpl()->writeReg(0x23 + op, inst[insnr].data[3]);
    getOpl()->writeReg(0xe3 + op, inst[insnr].data[4]);
    getOpl()->writeReg(0x60 + op, inst[insnr].data[5]);
    getOpl()->writeReg(0x80 + op, inst[insnr].data[6]);
    getOpl()->writeReg(0x20 + op, inst[insnr].data[8]);
    getOpl()->writeReg(0xe0 + op, inst[insnr].data[9]);
    if (version)
        getOpl()->writeReg(0xc0 + chan, inst[insnr].data[10]);
    else
        getOpl()->writeReg(0xc0 + chan,
                           (inst[insnr].data[10] << 1) + (inst[insnr].tunelev & 1));
}

void Cd00Player::playnote(unsigned char chan) {
    // set misc vars & play
    getOpl()->writeReg(0xb0 + chan, 0); // stop old note
    setinst(chan);
    channel[chan].key = 1;
    setfreq(chan);
    setvolume(chan);
}

void Cd00Player::vibrato(unsigned char chan) {
    if (!channel[chan].vibdepth)
        return;

    if (channel[chan].trigger)
        channel[chan].trigger--;
    else {
        channel[chan].trigger = channel[chan].vibdepth;
        channel[chan].vibspeed = -channel[chan].vibspeed;
    }
    channel[chan].freq += channel[chan].vibspeed;
    setfreq(chan);
}
