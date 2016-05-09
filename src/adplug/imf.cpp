/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * imf.cpp - IMF Player by Simon Peter <dn.tlp@gmx.net>
 *
 * FILE FORMAT:
 * There seem to be 2 different flavors of IMF formats out there. One version
 * contains just the raw IMF music data. In this case, the first word of the
 * file is always 0 (because the music data starts this way). This is already
 * the music data! So read in the entire file and play it.
 *
 * If this word is greater than 0, it specifies the size of the following
 * song data in bytes. In this case, the file has a footer that contains
 * arbitrary infos about it. Mostly, this is plain ASCII text with some words
 * of the author. Read and play the specified amount of song data and display
 * the remaining data as ASCII text.
 *
 * NOTES:
 * This player handles the two above mentioned formats, as well as a third
 * type, invented by Martin Fernandez <mfernan@cnba.uba.ar>, that's got a
 * proper header to add title/game name information. After the header starts
 * the normal IMF file in one of the two above mentioned formats.
 *
 * This player also handles a special footer format by Adam Nielsen,
 * which has defined fields of information about the song, the author
 * and more.
 */

#include "stream/filestream.h"

#include "imf.h"

/*** public methods *************************************/

CPlayer *CimfPlayer::factory() { return new CimfPlayer(); }

bool CimfPlayer::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;

    // file validation section
    size_t mfsize = 0;
    {
        char header[5];
        f.read(header, 5);
        uint8_t version;
        f >> version;

        if (strncmp(header, "ADLIB", 5) || version != 1) {
            if(f.extension() != ".imf" && f.extension() != ".wlf") {
                // It's no IMF file at all
                return false;
            }
            else
                f.seek(0); // It's a normal IMF file
        }
        else {
            // It's a IMF file with header
            char c;
            while(f>>c && c)
                m_trackName += c;
            while(f>>c && c)
                m_gameName += c;
            f.seekrel(1);
            mfsize = f.pos() + 2;
        }
    }

    // load section
    uint32_t fsize;
    if (mfsize != 0) {
        f >> fsize;
    }
    else {
        uint16_t tmp;
        f >> tmp;
        fsize = tmp;
    }
    auto flsize = f.size();
    size_t size;
    if (!fsize) { // footerless file (raw music data)
        if (mfsize)
            f.seekrel(-4);
        else
            f.seekrel(-2);
        size = (flsize - mfsize) / 4;
    }
    else // file has got a footer
        size = fsize / 4;

    m_data.resize(size);
    f.read(m_data.data(), size);

    // read footer, if any
    if (fsize && (fsize < flsize - 2 - mfsize)) {
        char c;
        if (f>>c && c == 0x1a) {
            // Adam Nielsen's footer format
            while(f>>c && c)
                m_trackName += c;
            while(f>>c && c)
                m_authorName += c;
            while(f>>c && c)
                m_remarks += c;
        }
        else {
            // Generic footer
            auto footerlen = flsize - fsize - 2 - mfsize;
            char c;
            while(footerlen-- && (f>>c))
                m_footer += c;
        }
    }

    m_rate = getrate(f);
    rewind(0);
    return true;
}

bool CimfPlayer::update() {
    do {
        getOpl()->writeReg(m_data[m_pos].reg, m_data[m_pos].val);
        m_del = m_data[m_pos].time;
        m_pos++;
    } while (!m_del && m_pos < m_data.size());

    if (m_pos >= m_data.size()) {
        m_pos = 0;
        m_songend = true;
    } else
        m_timer = float(m_rate) / m_del;

    return !m_songend;
}

void CimfPlayer::rewind(int) {
    m_pos = 0;
    m_del = 0;
    m_timer = m_rate;
    m_songend = false;
    getOpl()->writeReg(1, 32); // go to OPL2 mode
}

std::string CimfPlayer::title() const
{
    std::string title = m_trackName;
    if (!m_trackName.empty() && !m_gameName.empty())
        title += " - ";

    title += m_gameName;

    return title;
}

std::string CimfPlayer::description() const
{
    std::string desc = m_footer;

    if (!m_remarks.empty() && !m_footer.empty())
        desc += "\n\n";

    desc += m_remarks;

    return desc;
}

/*** private methods *************************************/

int CimfPlayer::getrate(const FileStream& file) {
    if (file.extension() == ".imf")
        return 560;
    else if (file.extension() == ".wlf")
        return 700;
    return 700; // default speed for unknown files that aren't .IMF or .WLF
}
