/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * adtrack.cpp - Adlib Tracker 1.0 Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * The original Adlib Tracker 1.0 is behaving a little different from the
 * official spec: The 'octave' integer from the instrument file is stored
 * "minus 1" from the actual value, underflowing from 0 to 0xffff.
 *
 * I also noticed that my player is playing everything transposed a few tones
 * higher than the original tracker. As far as i can see, my player perfectly
 * follows the official spec, so it "must" be the tracker that does something
 * wrong here...
 */

#include <cstdlib>
#include <cstring>

#include "adtrack.h"

#include "stream/filestream.h"

 /*** Public methods ***/

Player* AdTrackPlayer::factory()
{
    return new AdTrackPlayer();
}

bool AdTrackPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if(!f)
        return false;
    unsigned short rwp;
    unsigned char chp, pnote = 0;
    // file validation
    if(f.extension() != ".sng" || f.size() != 36000)
    {
        return false;
    }

    // check for instruments file
    std::string instfilename(filename, 0, filename.find_last_of('.'));
    instfilename += ".ins";
    {
        FileStream instf(instfilename);
        if(!instf || instf.size() != 468)
        {
            return false;
        }

        // give CmodPlayer a hint on what we're up to
        realloc_patterns(1, 1000, 9);
        addOrder(0);
        init_trackord();
        setNoKeyOn();
        setRestartOrder(0);
        setInitialTempo(120);
        setInitialSpeed(3);

        // load instruments from instruments file
        for(int i = 0; i < 9; i++)
        {
            Instrument myinst;
            f >> myinst;
            addInstrument(&myinst);
        }
    }

    // load file
    for(rwp = 0; rwp < 1000; rwp++)
    {
        for(chp = 0; chp < 9; chp++)
        {
            // read next record
            char note[2];
            f.read(note, 2);
            uint8_t octave;
            f >> octave;
            f.seekrel(1);
            switch(*note)
            {
                case 'C':
                    if(note[1] == '#')
                        pnote = 2;
                    else
                        pnote = 1;
                    break;
                case 'D':
                    if(note[1] == '#')
                        pnote = 4;
                    else
                        pnote = 3;
                    break;
                case 'E':
                    pnote = 5;
                    break;
                case 'F':
                    if(note[1] == '#')
                        pnote = 7;
                    else
                        pnote = 6;
                    break;
                case 'G':
                    if(note[1] == '#')
                        pnote = 9;
                    else
                        pnote = 8;
                    break;
                case 'A':
                    if(note[1] == '#')
                        pnote = 11;
                    else
                        pnote = 10;
                    break;
                case 'B':
                    pnote = 12;
                    break;
                case '\0':
                    if(note[1] == '\0')
                        patternCell(chp, rwp).note = 127;
                    else
                    {
                        return false;
                    }
                    break;
                default:
                    return false;
            }
            if((*note) != '\0')
            {
                patternCell(chp, rwp).note = pnote + (octave * 12);
                patternCell(chp, rwp).instrument = chp + 1;
            }
        }
    }

    rewind(0);
    return true;
}

size_t AdTrackPlayer::framesUntilUpdate() const
{
    return static_cast<size_t>(SampleRate / 18.2);
}

/*** Private methods ***/

void AdTrackPlayer::addInstrument(Instrument* i)
{
    ModPlayer::Instrument& inst = addInstrument();
    // Carrier "Amp Mod / Vib / Env Type / KSR / Multiple" register
    inst.data[2] = i->op[Carrier].appampmod ? 1 << 7 : 0;
    inst.data[2] += i->op[Carrier].appvib ? 1 << 6 : 0;
    inst.data[2] += i->op[Carrier].maintsuslvl ? 1 << 5 : 0;
    inst.data[2] += i->op[Carrier].keybscale ? 1 << 4 : 0;
    inst.data[2] += (i->op[Carrier].octave + 1) & 0xffff; // Bug in original tracker
    // Modulator...
    inst.data[1] = i->op[Modulator].appampmod ? 1 << 7 : 0;
    inst.data[1] += i->op[Modulator].appvib ? 1 << 6 : 0;
    inst.data[1] += i->op[Modulator].maintsuslvl ? 1 << 5 : 0;
    inst.data[1] += i->op[Modulator].keybscale ? 1 << 4 : 0;
    inst.data[1] += (i->op[Modulator].octave + 1) & 0xffff; // Bug in original tracker

    // Carrier "Key Scaling / Level" register
    inst.data[10] = (i->op[Carrier].freqrisevollvldn & 3) << 6;
    inst.data[10] += i->op[Carrier].softness & 63;
    // Modulator...
    inst.data[9] = (i->op[Modulator].freqrisevollvldn & 3) << 6;
    inst.data[9] += i->op[Modulator].softness & 63;

    // Carrier "Attack / Decay" register
    inst.data[4] = (i->op[Carrier].attack & 0x0f) << 4;
    inst.data[4] += i->op[Carrier].decay & 0x0f;
    // Modulator...
    inst.data[3] = (i->op[Modulator].attack & 0x0f) << 4;
    inst.data[3] += i->op[Modulator].decay & 0x0f;

    // Carrier "Release / Sustain" register
    inst.data[6] = (i->op[Carrier].release & 0x0f) << 4;
    inst.data[6] += i->op[Carrier].sustain & 0x0f;
    // Modulator...
    inst.data[5] = (i->op[Modulator].release & 0x0f) << 4;
    inst.data[5] += i->op[Modulator].sustain & 0x0f;

    // Channel "Feedback / Connection" register
    inst.data[0] = (i->op[Carrier].feedback & 7) << 1;

    // Carrier/Modulator "Wave Select" registers
    inst.data[8] = i->op[Carrier].waveform & 3;
    inst.data[7] = i->op[Modulator].waveform & 3;
}