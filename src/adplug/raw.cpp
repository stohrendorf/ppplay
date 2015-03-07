/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2005 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * raw.c - RAW Player by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstring>

#include "stream/filestream.h"

#include "raw.h"

/*** public methods *************************************/

CPlayer *CrawPlayer::factory() { return new CrawPlayer(); }

bool CrawPlayer::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;

    // file validation section
    char id[8];
    f.read(id, 8);
    if (strncmp(id, "RAWADATA", 8)) {
        return false;
    }

    // load section
    f >> m_clock; // clock speed
    auto length = (f.size() - 10) / 2;
    m_data.resize(length);
    f.read(m_data.data(), length);

    rewind(0);
    return true;
}

bool CrawPlayer::update() {
    bool setspeed;

    if (m_pos >= m_data.size())
        return false;

    if (m_del) {
        m_del--;
        return !m_songend;
    }

    do {
        setspeed = false;
        switch (m_data[m_pos].command) {
        case 0:
            m_del = m_data[m_pos].param - 1;
            break;
        case 2:
            if (!m_data[m_pos].param) {
                m_pos++;
                m_speed = m_data[m_pos].param + (m_data[m_pos].command << 8);
                setspeed = true;
            } else
                ; //FIXME sto opl->setchip(data[pos].param - 1);
            break;
        case 0xff:
            if (m_data[m_pos].param == 0xff) {
                rewind(0); // auto-rewind song
                m_songend = true;
                return !m_songend;
            }
            break;
        default:
            getOpl()->writeReg(m_data[m_pos].command, m_data[m_pos].param);
            break;
        }
    } while (m_data[m_pos++].command || setspeed);

    return !m_songend;
}

void CrawPlayer::rewind(int) {
    m_pos = m_del = 0;
    m_speed = m_clock;
    m_songend = false;
    getOpl()->writeReg(1, 32); // go to 9 channel mode
}

size_t CrawPlayer::framesUntilUpdate() {
    return SampleRate * (m_speed ? m_speed : 0xffff) /
            1193180.0; // timer oscillator speed / wait register = clock
    // frequency
}
