/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2003 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libbinio/binwrap.h>
#include <libbinio/binfile.h>

#include "defines.h"
#include "disk.h"

#define BUFSIZE		512

DiskWriter::DiskWriter(const char *filename, uint32_t nfreq)
    : EmuPlayer(nfreq, BUFSIZE)
    , m_file(filename, FileStream::Mode::Write)
    , m_bytesWritten(0)
{
    if(!m_file) {
        message(MSG_ERROR, "cannot open file for output -- %s", filename);
        exit(EXIT_FAILURE);
    }

    // Write Microsoft RIFF WAVE header
    m_file.write("RIFF", 4);
    uint32_t t32;
    t32 = 36; m_file << t32;
    m_file.write("WAVEfmt ", 8);
    t32 = 16; m_file << t32;
    uint16_t t16;
    t16 = 1; m_file << t16;
    t16 = 2; m_file << t16;
    m_file << nfreq;
    nfreq *= 4;
    m_file << nfreq;
    t16 = 4; m_file << t16;
    t16 = 16; m_file << t16;
    m_file.write("data", 4);
    t32 = 0; m_file << t32;
}

DiskWriter::~DiskWriter()
{
    if(!m_file) return;

    if(m_bytesWritten % 2) { // Wave data must end on an even byte boundary
        uint8_t tmp = 0;
        m_file << tmp;
        m_bytesWritten++;
    }

    // Write file sizes
    m_file.seek(40);
    m_file << m_bytesWritten;
    m_bytesWritten += 36; // make absolute filesize (add header size)
    m_file.seek(4);
    m_file << m_bytesWritten;
}

void DiskWriter::output(const std::vector<int16_t> &buf)
{
    m_file.write(buf.data(), buf.size());

    m_bytesWritten += buf.size()*2;
}
