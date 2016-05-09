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
 * ksm.cpp - KSM Player for AdPlug by Simon Peter <dn.tlp@gmx.net>
 */

#include "stream/filestream.h"

#include <boost/filesystem.hpp>

#include "ksm.h"

const unsigned int CksmPlayer::adlibfreq[63] = {
    0, 2390, 2411, 2434, 2456, 2480, 2506, 2533, 2562, 2592, 2625, 2659, 2695,
    3414, 3435, 3458, 3480, 3504, 3530, 3557, 3586, 3616, 3649, 3683, 3719, 4438,
    4459, 4482, 4504, 4528, 4554, 4581, 4610, 4640, 4673, 4707, 4743, 5462, 5483,
    5506, 5528, 5552, 5578, 5605, 5634, 5664, 5697, 5731, 5767, 6486, 6507, 6530,
    6552, 6576, 6602, 6629, 6658, 6688, 6721, 6755, 6791, 7510
};

/*** public methods **************************************/

CPlayer* CksmPlayer::factory()
{
    return new CksmPlayer();
}

bool CksmPlayer::load(const std::string& filename)
{
    FileStream f(filename);

    // file validation section
    if( !f || f.extension() != ".ksm" )
    {
        return false;
    }

    // Load instruments from 'insts.dat'
    boost::filesystem::path fn(filename);
    fn.remove_filename() /= "insts.dat";

    FileStream insts(fn.string());
    if( !f )
    {
        return false;
    }
    loadinsts(insts);

    f.read(trinst, 16);
    f.read(trquant, 16);
    f.read(trchan, 16);
    f.seekrel(16);
    f.read(trvol, 16);
    uint16_t numnotes;
    f >> numnotes;
    note.resize(numnotes);
    f.read(note.data(), note.size());

    if( !trchan[11] )
    {
        drumstat = 0;
        numchans = 9;
    }
    else
    {
        drumstat = 32;
        numchans = 6;
    }

    rewind(0);
    return true;
}

bool CksmPlayer::update()
{
    count++;
    if( count >= countstop )
    {
        size_t bufnum = 0;
        while( count >= countstop )
        {
            auto templong = note[nownote];
            const auto track = ((templong >> 8) & 15);
            if( (templong & 192) == 0 )
            {
                size_t i = 0;

                while( (i < numchans) && ((chanfreq[i] != (templong & 63)) ||
                    (chantrack[i] != ((templong >> 8) & 15))) )
                    i++;
                if( i < numchans )
                {
                    databuf[bufnum++] = 0;
                    databuf[bufnum++] = static_cast<uint8_t>(0xb0 + i);
                    databuf[bufnum++] =
                        static_cast<uint8_t>((adlibfreq[templong & 63] >> 8) & 223);
                    chanfreq[i] = 0;
                    chanage[i] = 0;
                }
            }
            else
            {
                int volevel = trvol[track];
                if( (templong & 192) == 128 )
                {
                    volevel -= 4;
                    if( volevel < 0 )
                        volevel = 0;
                }
                if( (templong & 192) == 192 )
                {
                    volevel += 4;
                    if( volevel > 63 )
                        volevel = 63;
                }
                if( track < 11 )
                {
                    size_t temp = 0;
                    auto i = numchans;
                    for( size_t j = 0; j < numchans; j++ )
                        if( (countstop - chanage[j] >= temp) && (chantrack[j] == track) )
                        {
                            temp = countstop - chanage[j];
                            i = j;
                        }
                    if( i < numchans )
                    {
                        databuf[bufnum++] = 0;
                        databuf[bufnum++] = static_cast<uint8_t>(0xb0 + i);
                        databuf[bufnum++] = 0;
                        const auto volval = (inst[trinst[track]][1] & 192) + (volevel ^ 63);
                        databuf[bufnum++] = 0;
                        databuf[bufnum++] = static_cast<uint8_t>(0x40 + s_opTable[i] + 3);
                        databuf[bufnum++] = static_cast<uint8_t>(volval);
                        databuf[bufnum++] = 0;
                        databuf[bufnum++] = static_cast<uint8_t>(0xa0 + i);
                        databuf[bufnum++] = static_cast<uint8_t>(adlibfreq[templong & 63] & 255);
                        databuf[bufnum++] = 0;
                        databuf[bufnum++] = static_cast<uint8_t>(0xb0 + i);
                        databuf[bufnum++] =
                            static_cast<uint8_t>((adlibfreq[templong & 63] >> 8) | 32);
                        chanfreq[i] = templong & 63;
                        chanage[i] = countstop;
                    }
                }
                else if( (drumstat & 32) > 0 )
                {
                    auto freq = adlibfreq[templong & 63];
                    int chan, drumnum;
                    switch( track )
                    {
                    case 11:
                        drumnum = 16;
                        chan = 6;
                        freq -= 2048;
                        break;
                    case 12:
                        drumnum = 8;
                        chan = 7;
                        freq -= 2048;
                        break;
                    case 13:
                        drumnum = 4;
                        chan = 8;
                        break;
                    case 14:
                        drumnum = 2;
                        chan = 8;
                        break;
                    case 15:
                        drumnum = 1;
                        chan = 7;
                        freq -= 2048;
                        break;
                    default:
                        BOOST_THROW_EXCEPTION(std::runtime_error("Unexpected track number"));
                    }
                    databuf[bufnum++] = 0;
                    databuf[bufnum++] = static_cast<uint8_t>(0xa0 + chan);
                    databuf[bufnum++] = static_cast<uint8_t>(freq & 255);
                    databuf[bufnum++] = 0;
                    databuf[bufnum++] = static_cast<uint8_t>(0xb0 + chan);
                    databuf[bufnum++] = static_cast<uint8_t>((freq >> 8) & 223);
                    databuf[bufnum++] = 0;
                    databuf[bufnum++] = static_cast<uint8_t>(0xbd);
                    databuf[bufnum++] = static_cast<uint8_t>(drumstat & (255 - drumnum));
                    drumstat |= drumnum;
                    if( (track == 11) || (track == 12) || (track == 14) )
                    {
                        const auto volval = (inst[trinst[track]][1] & 192) + (volevel ^ 63);
                        databuf[bufnum++] = 0;
                        databuf[bufnum++] = static_cast<uint8_t>(0x40 + s_opTable[chan] + 3);
                        databuf[bufnum++] = static_cast<uint8_t>(volval);
                    }
                    else
                    {
                        const auto volval = (inst[trinst[track]][6] & 192) + (volevel ^ 63);
                        databuf[bufnum++] = 0;
                        databuf[bufnum++] = static_cast<uint8_t>(0x40 + s_opTable[chan]);
                        databuf[bufnum++] = static_cast<uint8_t>(volval);
                    }
                    databuf[bufnum++] = 0;
                    databuf[bufnum++] = static_cast<uint8_t>(0xbd);
                    databuf[bufnum++] = static_cast<uint8_t>(drumstat);
                }
            }
            nownote++;
            if( nownote >= note.size() )
            {
                nownote = 0;
                songend = true;
            }
            templong = note[nownote];
            if( nownote == 0 )
                count = (templong >> 12) - 1;
            const auto quanter = (240 / trquant[(templong >> 8) & 15]);
            countstop = (((templong >> 12) + (quanter >> 1)) / quanter) * quanter;
        }
        for( int i = 0; i < bufnum; i += 3 )
            getOpl()->writeReg(databuf[i + 1], databuf[i + 2]);
    }
    return !songend;
}

void CksmPlayer::rewind(int)
{
    unsigned int i, j;
    unsigned char instbuf[11];
    unsigned long templong;

    songend = false;
    getOpl()->writeReg(1, 32);
    getOpl()->writeReg(4, 0);
    getOpl()->writeReg(8, 0);
    getOpl()->writeReg(0xbd, drumstat);

    if( trchan[11] == 1 )
    {
        for( i = 0; i < 11; i++ )
            instbuf[i] = inst[trinst[11]][i];
        instbuf[1] = ((instbuf[1] & 192) | (trvol[11] ^ 63));
        setinst(6, instbuf[0], instbuf[1], instbuf[2], instbuf[3], instbuf[4],
                instbuf[5], instbuf[6], instbuf[7], instbuf[8], instbuf[9],
                instbuf[10]);
        for( i = 0; i < 5; i++ )
            instbuf[i] = inst[trinst[12]][i];
        for( i = 5; i < 11; i++ )
            instbuf[i] = inst[trinst[15]][i];
        instbuf[1] = ((instbuf[1] & 192) | (trvol[12] ^ 63));
        instbuf[6] = ((instbuf[6] & 192) | (trvol[15] ^ 63));
        setinst(7, instbuf[0], instbuf[1], instbuf[2], instbuf[3], instbuf[4],
                instbuf[5], instbuf[6], instbuf[7], instbuf[8], instbuf[9],
                instbuf[10]);
        for( i = 0; i < 5; i++ )
            instbuf[i] = inst[trinst[14]][i];
        for( i = 5; i < 11; i++ )
            instbuf[i] = inst[trinst[13]][i];
        instbuf[1] = ((instbuf[1] & 192) | (trvol[14] ^ 63));
        instbuf[6] = ((instbuf[6] & 192) | (trvol[13] ^ 63));
        setinst(8, instbuf[0], instbuf[1], instbuf[2], instbuf[3], instbuf[4],
                instbuf[5], instbuf[6], instbuf[7], instbuf[8], instbuf[9],
                instbuf[10]);
    }

    for( i = 0; i < numchans; i++ )
    {
        chantrack[i] = 0;
        chanage[i] = 0;
    }
    j = 0;
    for( i = 0; i < 16; i++ )
        if( (trchan[i] > 0) && (j < numchans) )
        {
            auto k = trchan[i];
            while( (j < numchans) && (k > 0) )
            {
                chantrack[j] = i;
                k--;
                j++;
            }
        }
    for( i = 0; i < numchans; i++ )
    {
        for( j = 0; j < 11; j++ )
            instbuf[j] = inst[trinst[chantrack[i]]][j];
        instbuf[1] = ((instbuf[1] & 192) | (63 - trvol[chantrack[i]]));
        setinst(i, instbuf[0], instbuf[1], instbuf[2], instbuf[3], instbuf[4],
                instbuf[5], instbuf[6], instbuf[7], instbuf[8], instbuf[9],
                instbuf[10]);
        chanfreq[i] = 0;
    }
    templong = note.front();
    count = (templong >> 12) - 1;
    countstop = (templong >> 12) - 1;
    nownote = 0;
}

std::string CksmPlayer::instrumentTitle(size_t n) const
{
    if( trchan[n] )
        return instname[trinst[n]];
    else
        return std::string();
}

/*** private methods *************************************/

void CksmPlayer::loadinsts(FileStream& f)
{
    for( int i = 0; i < 256; i++ )
    {
        f.read(instname[i], 20);
        f.read(inst[i], 11);
        f.seekrel(2);
    }
}

void CksmPlayer::setinst(int chan, unsigned char v0, unsigned char v1,
                         unsigned char v2, unsigned char v3, unsigned char v4,
                         unsigned char v5, unsigned char v6, unsigned char v7,
                         unsigned char v8, unsigned char v9,
                         unsigned char v10)
{
    int offs;

    getOpl()->writeReg(0xa0 + chan, 0);
    getOpl()->writeReg(0xb0 + chan, 0);
    getOpl()->writeReg(0xc0 + chan, v10);
    offs = s_opTable[chan];
    getOpl()->writeReg(0x20 + offs, v5);
    getOpl()->writeReg(0x40 + offs, v6);
    getOpl()->writeReg(0x60 + offs, v7);
    getOpl()->writeReg(0x80 + offs, v8);
    getOpl()->writeReg(0xe0 + offs, v9);
    offs += 3;
    getOpl()->writeReg(0x20 + offs, v0);
    getOpl()->writeReg(0x40 + offs, v1);
    getOpl()->writeReg(0x60 + offs, v2);
    getOpl()->writeReg(0x80 + offs, v3);
    getOpl()->writeReg(0xe0 + offs, v4);
}
