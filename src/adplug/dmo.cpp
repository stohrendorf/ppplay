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

#include <cstring>

#include "stream/filestream.h"
#include "stream/memorystream.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "dmo.h"
#include "debug.h"

#define LOWORD(l) ((l) & 0xffff)
#define HIWORD(l) ((l) >> 16)
#define LOBYTE(w) ((w) & 0xff)
#define HIBYTE(w) ((w) >> 8)

#define ARRAY_AS_DWORD(a, i)                                                   \
    ((a[i + 3] << 24) + (a[i + 2] << 16) + (a[i + 1] << 8) + a[i])
#define ARRAY_AS_WORD(a, i) ((a[i + 1] << 8) + a[i])

#define CHARP_AS_WORD(p) (((*(p + 1)) << 8) + (*p))

/* -------- Public Methods -------------------------------- */

CPlayer *CdmoLoader::factory() { return new CdmoLoader(); }

bool CdmoLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f || f.extension() != ".dmo")
        return false;

    unsigned char chkhdr[16];
    f.read(chkhdr, 16);

    dmo_unpacker unpacker;
    if (!unpacker.decrypt(chkhdr, 16)) {
        return false;
    }

    f.seek(0);

    std::vector<uint8_t> packed_module(f.size());

    // load file
    f.read(packed_module.data(), f.size());

    // decrypt
    unpacker.decrypt(packed_module.data(), packed_module.size());

    const auto unpacked_length = 0x2000 * ARRAY_AS_WORD(packed_module, 12);
    std::vector<uint8_t> module(unpacked_length);

    // unpack
    if (!unpacker.unpack(packed_module.data() + 12, module.data(), unpacked_length)) {
        return false;
    }

    // "TwinTeam" - signed ?
    if (memcmp(module.data(), "TwinTeam Module File"
               "\x0D\x0A",
               22)) {
        return false;
    }

    // load header
    MemoryStream uf;
    uf.write(module.data(), module.size());
    uf.seek(0);

    memset(&m_header, 0, sizeof(S3mHeader));

    uf.seekrel(22); // ignore DMO header ID string
    uf.read(m_header.name, 28);

    uf.seekrel(2); // _unk_1
    uf >> m_header.ordnum >> m_header.insnum >> m_header.patnum;
    uf.seekrel(2); // _unk_2
    uf >> m_header.is >> m_header.it;

    memset(m_header.chanset, 0xFF, 32);

    for (int i = 0; i < 9; i++)
        m_header.chanset[i] = 0x10 + i;

    uf.seekrel(32); // ignore panning settings for all 32 channels

    // load orders
    uf.read(m_orders, 256);

    m_orders[m_header.ordnum] = 0xFF;

    // load pattern lengths
    uint16_t my_patlen[100];
    uf.read(my_patlen, 100);

    // load instruments
    for (int i = 0; i < m_header.insnum; i++) {
        memset(&m_instruments[i], 0, sizeof(S3mInstrument));

        uf.read(m_instruments[i].name, 28);

        uf >> m_instruments[i].volume;
        uf >> m_instruments[i].dsk;
        uf >> m_instruments[i].c2spd;
        uf >> m_instruments[i].type;
        uf >> m_instruments[i].d00;
        uf >> m_instruments[i].d01;
        uf >> m_instruments[i].d02;
        uf >> m_instruments[i].d03;
        uf >> m_instruments[i].d04;
        uf >> m_instruments[i].d05;
        uf >> m_instruments[i].d06;
        uf >> m_instruments[i].d07;
        uf >> m_instruments[i].d08;
        uf >> m_instruments[i].d09;
        uf >> m_instruments[i].d0a;
        /*
         * Originally, riven sets d0b = d0a and ignores 1 byte in the
         * stream, but i guess this was a typo, so i read it here.
         */
        uf >> m_instruments[i].d0b;
    }

    // load patterns
    for (int i = 0; i < m_header.patnum; i++) {
        const auto cur_pos = uf.pos();

        for (int j = 0; j < 64; j++) {
            while(true) {
                uint8_t token;
                uf >> token;

                if (!token)
                    break;

                const auto chan = token & 31;

                // note + instrument ?
                if (token & 32) {
                    uint8_t bufbyte;
                    uf >> bufbyte;

                    m_patterns[i][j][chan].note = bufbyte & 15;
                    m_patterns[i][j][chan].octave = bufbyte >> 4;
                    uf >> m_patterns[i][j][chan].instrument;
                }

                // volume ?
                if (token & 64)
                    uf >> m_patterns[i][j][chan].volume;

                // command ?
                if (token & 128) {
                    uf >> m_patterns[i][j][chan].effect;
                    uf >> m_patterns[i][j][chan].effectValue;
                }
            }
        }

        uf.seek(cur_pos + my_patlen[i]);
    }

    rewind(0);
    return true;
}

std::string CdmoLoader::gettype() {
    return std::string("TwinTeam (packed S3M)");
}

std::string CdmoLoader::getauthor() {
    /*
  All available .DMO modules written by one composer. And because all .DMO
  stuff was lost due to hd crash (TwinTeam guys said this), there are
  never(?) be another.
*/
    return std::string("Benjamin GERARDIN");
}

/* -------- Private Methods ------------------------------- */

unsigned short CdmoLoader::dmo_unpacker::brand(unsigned short range) {
    unsigned short ax, bx, cx, dx;

    ax = LOWORD(bseed);
    bx = HIWORD(bseed);
    cx = ax;
    ax = LOWORD(cx * 0x8405);
    dx = HIWORD(cx * 0x8405);
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
    if (!ax)
        dx += 1;

    // leave it that way or amd64 might get it wrong
    bseed = dx;
    bseed <<= 16;
    bseed += ax;

    return HIWORD(HIWORD(LOWORD(bseed) * range) + HIWORD(bseed) * range);
}

bool CdmoLoader::dmo_unpacker::decrypt(unsigned char *buf, long len) {
    unsigned long seed = 0;
    int i;

    bseed = ARRAY_AS_DWORD(buf, 0);

    for (i = 0; i < ARRAY_AS_WORD(buf, 4) + 1; i++)
        seed += brand(0xffff);

    bseed = seed ^ ARRAY_AS_DWORD(buf, 6);

    if (ARRAY_AS_WORD(buf, 10) != brand(0xffff))
        return false;

    for (i = 0; i < (len - 12); i++)
        buf[12 + i] ^= brand(0x100);

    buf[len - 2] = buf[len - 1] = 0;

    return true;
}

short CdmoLoader::dmo_unpacker::unpack_block(unsigned char *ibuf, long ilen,
                                             unsigned char *obuf) {
    unsigned char code, par1, par2;
    unsigned short ax, bx, cx;

    unsigned char *ipos = ibuf;
    unsigned char *opos = obuf;

    // LZ77 child
    while (ipos - ibuf < ilen) {
        code = *ipos++;

        // 00xxxxxx: copy (xxxxxx + 1) bytes
        if ((code >> 6) == 0) {
            cx = (code & 0x3F) + 1;

            if (opos + cx >= oend)
                return -1;

            for (int i = 0; i < cx; i++)
                *opos++ = *ipos++;

            continue;
        }

        // 01xxxxxx xxxyyyyy: copy (Y + 3) bytes from (X + 1)
        if ((code >> 6) == 1) {
            par1 = *ipos++;

            ax = ((code & 0x3F) << 3) + ((par1 & 0xE0) >> 5) + 1;
            cx = (par1 & 0x1F) + 3;

            if (opos + cx >= oend)
                return -1;

            for (int i = 0; i < cx; i++) {
                *opos = *(opos - ax);
                ++opos;
            }

            continue;
        }

        // 10xxxxxx xyyyzzzz: copy (Y + 3) bytes from (X + 1); copy Z bytes
        if ((code >> 6) == 2) {
            int i;

            par1 = *ipos++;

            ax = ((code & 0x3F) << 1) + (par1 >> 7) + 1;
            cx = ((par1 & 0x70) >> 4) + 3;
            bx = par1 & 0x0F;

            if (opos + bx + cx >= oend)
                return -1;

            for (i = 0; i < cx; i++) {
                *opos = *(opos - ax);
                ++opos;
            }

            for (i = 0; i < bx; i++)
                *opos++ = *ipos++;

            continue;
        }

        // 11xxxxxx xxxxxxxy yyyyzzzz: copy (Y + 4) from X; copy Z bytes
        if ((code >> 6) == 3) {
            int i;

            par1 = *ipos++;
            par2 = *ipos++;

            bx = ((code & 0x3F) << 7) + (par1 >> 1);
            cx = ((par1 & 0x01) << 4) + (par2 >> 4) + 4;
            ax = par2 & 0x0F;

            if (opos + ax + cx >= oend)
                return -1;

            for (i = 0; i < cx; i++) {
                *opos = *(opos - bx);
                ++opos;
            }

            for (i = 0; i < ax; i++)
                *opos++ = *ipos++;

            continue;
        }
    }

    return opos - obuf;
}

long CdmoLoader::dmo_unpacker::unpack(unsigned char *ibuf, unsigned char *obuf,
                                      unsigned long outputsize) {
    long olen = 0;
    unsigned short block_count = CHARP_AS_WORD(ibuf);

    ibuf += 2;
    unsigned char *block_length = ibuf;
    ibuf += 2 * block_count;

    oend = obuf + outputsize;

    for (int i = 0; i < block_count; i++) {
        unsigned short bul = CHARP_AS_WORD(ibuf);

        if (unpack_block(ibuf + 2, CHARP_AS_WORD(block_length) - 2, obuf) != bul)
            return 0;

        obuf += bul;
        olen += bul;

        ibuf += CHARP_AS_WORD(block_length);
        block_length += 2;
    }

    return olen;
}
