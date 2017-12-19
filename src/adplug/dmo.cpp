/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2004, 2006 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  dmo.cpp - TwinTeam loader by Riven the Mage <riven@ok.ru>
*/
/*
  NOTES:
  Panning is ignored.

  A WORD ist 16 bits, a DWORD is 32 bits and a BYTE is 8 bits in this context.
*/

#include "stream/filestream.h"
#include "stream/memorystream.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "dmo.h"

namespace
{
constexpr inline uint16_t LOWORD(uint32_t value)
{
    return value & 0xffff;
}

constexpr inline uint16_t HIWORD(uint32_t value)
{
    return value >> 16;
}

constexpr inline uint8_t LOBYTE(uint16_t value)
{
    return value & 0xff;
}

constexpr inline uint8_t HIBYTE(uint16_t value)
{
    return value >> 8;
}

constexpr inline uint32_t ARRAY_AS_DWORD(const uint8_t* data, size_t i)
{
    return (data[i + 3] << 24) + (data[i + 2] << 16) + (data[i + 1] << 8) + data[i];
}

constexpr inline uint16_t ARRAY_AS_WORD(const uint8_t* data, size_t i)
{
    return (data[i + 1] << 8) + data[i];
}

constexpr inline uint16_t CHARP_AS_WORD(const uint8_t* data)
{
    return ARRAY_AS_WORD(data, 0);
}

class DMOUnpacker
{
public:
    bool decrypt(uint8_t* buf, size_t len);

    size_t unpack(uint8_t* ibuf, uint8_t* obuf, size_t outputsize);

private:
    uint16_t brand(uint16_t range);

    int16_t unpack_block(uint8_t* ibuf, size_t ilen, uint8_t* obuf) const;

    size_t bseed = 0;
    uint8_t* outputEnd = nullptr;
};
}

/* -------- Public Methods -------------------------------- */

Player* DmoPlayer::factory()
{
    return new DmoPlayer();
}

bool DmoPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f || f.extension() != ".dmo" )
    {
        return false;
    }

    unsigned char chkhdr[16];
    f.read(chkhdr, 16);

    DMOUnpacker unpacker;
    if( !unpacker.decrypt(chkhdr, 16) )
    {
        return false;
    }

    f.seek(0);

    std::vector<uint8_t> packed_module(f.size());

    // load file
    f.read(packed_module.data(), f.size());

    // decrypt
    unpacker.decrypt(packed_module.data(), packed_module.size());

    const auto unpacked_length = 0x2000 * ARRAY_AS_WORD(packed_module.data(), 12);
    std::vector<uint8_t> module(unpacked_length);

    // unpack
    if( !unpacker.unpack(packed_module.data() + 12, module.data(), unpacked_length) )
    {
        return false;
    }

    // "TwinTeam" - signed ?
    if( memcmp(module.data(), "TwinTeam Module File"
                   "\x0D\x0A",
               22) )
    {
        return false;
    }

    // load header
    MemoryStream uf;
    uf.write(module.data(), module.size());
    uf.seek(0);

    S3mHeader header;

    uf.seekrel(22); // ignore DMO header ID string
    uf.read(header.name, 28);

    uf.seekrel(2); // _unk_1
    uf >> header.orderCount >> header.instrumentCount >> header.patternCount;
    uf.seekrel(2); // _unk_2
    uf >> header.initialSpeed >> header.initialTempo;

    header.chanset.fill(0xff);

    for( int i = 0; i < 9; i++ )
    {
        header.chanset[i] = 0x10 + i;
    }

    uf.seekrel(32); // ignore panning settings for all 32 channels

    // load orders
    for( int i = 0; i < header.orderCount; ++i )
    {
        uint8_t tmp;
        uf >> tmp;
        addOrder(tmp);
        std::cerr << "ORD=" << int(tmp) << "\n";
    }
    addOrder(0xff);
    uf.seekrel(256 - header.orderCount);

    // load pattern lengths
    uint16_t my_patlen[100];
    uf.read(my_patlen, 100);

    // load instruments
    for( int i = 0; i < header.instrumentCount; i++ )
    {
        S3mInstrument instrument;

        uf.read(instrument.name, 28);

        uf >> instrument.volume;
        uf >> instrument.dsk;
        uf >> instrument.c2spd;
        uf >> instrument.type;
        uf >> instrument.d00;
        uf >> instrument.d01;
        uf >> instrument.d02;
        uf >> instrument.d03;
        uf >> instrument.d04;
        uf >> instrument.d05;
        uf >> instrument.d06;
        uf >> instrument.d07;
        uf >> instrument.d08;
        uf >> instrument.d09;
        uf >> instrument.d0a;
        /*
         * Originally, riven sets d0b = d0a and ignores 1 byte in the
         * stream, but i guess this was a typo, so i read it here.
         */
        uf >> instrument.d0b;

        setInstrument(i, instrument);
    }

    // load patterns
    for( int pattern = 0; pattern < header.patternCount; pattern++ )
    {
        const auto cur_pos = uf.pos();

        for( int row = 0; row < 64; row++ )
        {
            S3mCell* currentChannel = patternChannel(pattern, row);

            while( true )
            {
                uint8_t token;
                uf >> token;

                if( !token )
                {
                    break;
                }

                const auto chan = token & 31;

                // note + instrument ?
                if( token & 32 )
                {
                    uint8_t bufbyte;
                    uf >> bufbyte;

                    currentChannel[chan].note = bufbyte & 15;
                    currentChannel[chan].octave = bufbyte >> 4;
                    uf >> currentChannel[chan].instrument;
                }

                // volume ?
                if( token & 64 )
                {
                    uf >> currentChannel[chan].volume;
                }

                // command ?
                if( token & 128 )
                {
                    uf >> currentChannel[chan].effect;
                    uf >> currentChannel[chan].effectValue;
                }
            }
        }

        uf.seek(cur_pos + my_patlen[pattern]);
    }

    setHeader(header);

    rewind(size_t(0));
    return true;
}

std::string DmoPlayer::type() const
{
    return "TwinTeam (packed S3M)";
}

std::string DmoPlayer::author() const
{
    /*
  All available .DMO modules written by one composer. And because all .DMO
  stuff was lost due to hd crash (TwinTeam guys said this), there are
  never(?) be another.
*/
    return "Benjamin GERARDIN";
}

/* -------- Private Methods ------------------------------- */

uint16_t DMOUnpacker::brand(uint16_t range)
{
    uint16_t ax = LOWORD(bseed);
    uint16_t bx = HIWORD(bseed);
    uint16_t cx = ax;
    ax = LOWORD(cx * 0x8405);
    uint16_t dx = HIWORD(cx * 0x8405);
    cx <<= 3;
    cx = (((HIBYTE(cx) + LOBYTE(cx)) & 0xFF) << 8) + LOBYTE(cx);
    dx += cx;
    dx += bx;
    bx <<= 2;
    dx += bx;
    dx = (((HIBYTE(dx) + LOBYTE(bx)) & 0xFF) << 8) + LOBYTE(dx);
    bx <<= 5;
    dx = (((HIBYTE(dx) + LOBYTE(bx)) & 0xFF) << 8) + LOBYTE(dx);
    ax += 1;
    if( !ax )
    {
        dx += 1;
    }

    // leave it that way or amd64 might get it wrong
    bseed = dx;
    bseed <<= 16;
    bseed += ax;

    return HIWORD(HIWORD(LOWORD(bseed) * range) + HIWORD(bseed) * range);
}

bool DMOUnpacker::decrypt(uint8_t* buf, size_t len)
{
    bseed = ARRAY_AS_DWORD(buf, 0);

    uint32_t seed = 0;
    for( int i = 0; i < ARRAY_AS_WORD(buf, 4) + 1; i++ )
    {
        seed += brand(0xffff);
    }

    bseed = seed ^ ARRAY_AS_DWORD(buf, 6);

    if( ARRAY_AS_WORD(buf, 10) != brand(0xffff) )
    {
        return false;
    }

    for( int i = 0; i < (len - 12); i++ )
    {
        buf[12 + i] ^= brand(0x100);
    }

    buf[len - 2] = buf[len - 1] = 0;

    return true;
}

int16_t DMOUnpacker::unpack_block(uint8_t* ibuf, size_t ilen, uint8_t* obuf) const
{
    uint8_t* ipos = ibuf;
    uint8_t* opos = obuf;

    // LZ77 child
    while( ipos - ibuf < ilen )
    {
        const auto code = *ipos++;

        // 00xxxxxx: copy (xxxxxx + 1) bytes
        if( (code >> 6) == 0 )
        {
            const uint16_t cx = (code & 0x3F) + 1;

            if( opos + cx >= outputEnd )
            {
                return -1;
            }

            for( int i = 0; i < cx; i++ )
            {
                *opos++ = *ipos++;
            }

            continue;
        }

        // 01xxxxxx xxxyyyyy: copy (Y + 3) bytes from (X + 1)
        if( (code >> 6) == 1 )
        {
            const auto par1 = *ipos++;

            const uint16_t ax = ((code & 0x3F) << 3) + ((par1 & 0xE0) >> 5) + 1;
            const uint16_t cx = (par1 & 0x1F) + 3;

            if( opos + cx >= outputEnd )
            {
                return -1;
            }

            for( int i = 0; i < cx; i++ )
            {
                *opos = *(opos - ax);
                ++opos;
            }

            continue;
        }

        // 10xxxxxx xyyyzzzz: copy (Y + 3) bytes from (X + 1); copy Z bytes
        if( (code >> 6) == 2 )
        {
            const auto par1 = *ipos++;

            const uint16_t ax = ((code & 0x3F) << 1) + (par1 >> 7) + 1;
            const uint16_t cx = ((par1 & 0x70) >> 4) + 3;
            const uint16_t bx = par1 & 0x0F;

            if( opos + bx + cx >= outputEnd )
            {
                return -1;
            }

            for( int i = 0; i < cx; i++ )
            {
                *opos = *(opos - ax);
                ++opos;
            }

            for( int i = 0; i < bx; i++ )
            {
                *opos++ = *ipos++;
            }

            continue;
        }

        // 11xxxxxx xxxxxxxy yyyyzzzz: copy (Y + 4) from X; copy Z bytes
        if( (code >> 6) == 3 )
        {
            const auto par1 = *ipos++;
            const auto par2 = *ipos++;

            const uint16_t bx = ((code & 0x3F) << 7) + (par1 >> 1);
            const uint16_t cx = ((par1 & 0x01) << 4) + (par2 >> 4) + 4;
            const uint16_t ax = par2 & 0x0F;

            if( opos + ax + cx >= outputEnd )
            {
                return -1;
            }

            for( int i = 0; i < cx; i++ )
            {
                *opos = *(opos - bx);
                ++opos;
            }

            for( int i = 0; i < ax; i++ )
            {
                *opos++ = *ipos++;
            }
        }
    }

    return opos - obuf;
}

size_t DMOUnpacker::unpack(uint8_t* inputBuf, uint8_t* outputBuf, size_t outputSize)
{
    size_t outputLength = 0;
    const uint16_t blockCount = CHARP_AS_WORD(inputBuf);

    inputBuf += 2;
    uint8_t* blockLength = inputBuf;
    inputBuf += 2 * blockCount;

    outputEnd = outputBuf + outputSize;

    for( uint16_t i = 0; i < blockCount; i++ )
    {
        auto bul = CHARP_AS_WORD(inputBuf);

        if( unpack_block(inputBuf + 2, CHARP_AS_WORD(blockLength) - 2, outputBuf) != bul )
        {
            return 0;
        }

        outputBuf += bul;
        outputLength += bul;

        inputBuf += CHARP_AS_WORD(blockLength);
        blockLength += 2;
    }

    return outputLength;
}
