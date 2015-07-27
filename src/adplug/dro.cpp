/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * dro.c - DOSBox Raw OPL Player by Sjoerd van der Berg <harekiet@zophar.net>
 *
 * upgraded by matthew gambrell <zeromus@zeromus.org>
 *
 * NOTES: 3-oct-04: the DRO format is not yet finalized. beware.
 */

#include <cstring>
#include <cstdio>

#include "stream/filestream.h"

#include "dro.h"

/*** public methods *************************************/

CPlayer *CdroPlayer::factory() { return new CdroPlayer(); }

bool CdroPlayer::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;

    // file validation section
    char id[8];
    f.read(id, 8);
    if (strncmp(id, "DBRAWOPL", 8)) {
        return false;
    }
    uint32_t version;
    f >> version;
    if (version != 0x10000) {
        return false;
    }

    // load section
    f >> m_msTotal; // Total milliseconds in file
    uint32_t length;
    f >> length;  // Total data bytes in file
    m_data.resize(length);

    f.seekrel(1); // Type of opl data this can contain - ignored
    f.read(m_data.data(), 3);

    if ((m_data[0] == 0) || (m_data[1] == 0) || (m_data[2] == 0)) {
        // Some early .DRO files only used one byte for the hardware type, then
        // later changed to four bytes with no version number change.  If we're
        // here then this is a later (more popular) file with the full four bytes
        // for the hardware-type.
        f.seekrel(-3);
    }
    f.read(m_data.data(), m_data.size());
    rewind(0);
    return true;
}

bool CdroPlayer::update() {
    if (m_delay > 500) {
        m_delay -= 500;
        return true;
    }
    else
        m_delay = 0;

    while (m_pos < m_data.size()) {
        unsigned char cmd = m_data[m_pos++];
        switch (cmd) {
        case 0:
            m_delay = 1 + m_data[m_pos++];
            return true;
        case 1:
            m_delay = 1 + m_data[m_pos] + (m_data[m_pos + 1] << 8);
            m_pos += 2;
            return true;
        case 2:
            m_index = 0;
            break;
        case 3:
            m_index = 1;
            break;
        default:
            if (cmd == 4)
                cmd = m_data[m_pos++]; //data override
            getOpl()->writeReg(cmd, m_data[m_pos++]);
            break;
        }
    }

    return m_pos < m_data.size();
}

void CdroPlayer::rewind(int) {
    m_delay = 1;
    m_pos = m_index = 0;

    //dro assumes all registers are initialized to 0
    //registers not initialized to 0 will be corrected
    //in the data stream
    for (int i = 0; i < 256; i++)
        getOpl()->writeReg(i, 0);
}

size_t CdroPlayer::framesUntilUpdate() const {
    if (m_delay > 500)
        return SampleRate / 2;
    else
        return SampleRate * m_delay / 1000;
}
