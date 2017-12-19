/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * bam.cpp - Bob's Adlib Music Player, by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * In my player, the loop counter is stored with the label. This can be
 * dangerous for some situations (see below), but there shouldn't be any BAM
 * files triggering this situation.
 *
 * From SourceForge Bug #476088:
 * -----------------------------
 * Using just one loop counter for each label, my player can't
 * handle files that loop twice to the same label (if that's at
 * all possible with BAM). Imagine the following situation:
 *
 * ... [*] ---- [<- *] ---- [<- *] ...
 *  ^   ^    ^     ^     ^     ^    ^
 *  |   |    |     |     |     |    |
 *  +---|----+-----|-----+-----|----+--- normal song data
 *      +----------|-----------|-------- label 1
 *                 +-----------+-------- loop points to label 1
 *
 * both loop points loop to the same label. Storing the loop
 * count with the label would cause chaos with the counter,
 * when the player executes the inner jump.
 * ------------------
 * Not to worry. my reference implementation of BAM does not
 * support the multiple loop situation you describe, and
 * neither do any BAM-creation programs. Then both loops point
 * to the same label, the inner loop's counter is just allowed
 * to clobber the outer loop's counter. No stack is neccisary.
 */

#include "stream/filestream.h"

#include "bam.h"

namespace
{
constexpr uint16_t s_frequencies[] = {
    172, 182, 193, 205, 217, 230, 243, 258, 274, 290, 307, 326, 345, 365, 387,
    410, 435, 460, 489, 517, 547, 580, 614, 651, 1369, 1389, 1411, 1434, 1459,
    1484, 1513, 1541, 1571, 1604, 1638, 1675, 2393, 2413, 2435, 2458, 2483, 2508,
    2537, 2565, 2595, 2628, 2662, 2699, 3417, 3437, 3459, 3482, 3507, 3532, 3561,
    3589, 3619, 3652, 3686, 3723, 4441, 4461, 4483, 4506, 4531, 4556, 4585, 4613,
    4643, 4676, 4710, 4747, 5465, 5485, 5507, 5530, 5555, 5580, 5609, 5637, 5667,
    5700, 5734, 5771, 6489, 6509, 6531, 6554, 6579, 6604, 6633, 6661, 6691, 6724,
    6758, 6795, 7513, 7533, 7555, 7578, 7603, 7628, 7657, 7685, 7715, 7748, 7782,
    7819, 7858, 7898, 7942, 7988, 8037, 8089, 8143, 8191, 8191, 8191, 8191, 8191,
    8191, 8191, 8191, 8191, 8191, 8191, 8191
};
}

Player* BamPlayer::factory()
{
    return new BamPlayer();
}

bool BamPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }

    char id[4];
    f.read(id, 4);
    if( !std::equal(id, id + 4, "CBMF") )
    {
        return false;
    }

    m_song.resize(f.size() - 4u);
    f.read(m_song.data(), m_song.size());

    addOrder(0); // dummy

    rewind(size_t(0));
    return true;
}

bool BamPlayer::update()
{
    if( m_delay )
    {
        m_delay--;
        return !m_songEnd;
    }

    if( m_position >= m_song.size() )
    { // EOF detection
        m_position = 0;
        m_songEnd = true;
    }

    while( m_song[m_position] < 128 )
    {
        const auto effectValue = m_song[m_position] & 0x0f;
        switch( m_song[m_position] & 0xf0 )
        {
            case 0x00: // stop song
                m_position = 0;
                m_songEnd = true;
                break;
            case 0x10: // start note
                if( effectValue < 9 )
                {
                    getOpl()->writeReg(0xa0 + effectValue, s_frequencies[m_song[++m_position]] & 255);
                    getOpl()->writeReg(0xb0 + effectValue, (s_frequencies[m_song[m_position]] >> 8) + 32);
                }
                else
                {
                    m_position++;
                }
                m_position++;
                break;
            case 0x20: // stop note
                if( effectValue < 9 )
                {
                    getOpl()->writeReg(0xb0 + effectValue, 0);
                }
                m_position++;
                break;
            case 0x30: // define instrument
                if( effectValue < 9 )
                {
                    getOpl()->writeReg(0x20 + s_opTable[effectValue], m_song[m_position + 1]);
                    getOpl()->writeReg(0x23 + s_opTable[effectValue], m_song[m_position + 2]);
                    getOpl()->writeReg(0x40 + s_opTable[effectValue], m_song[m_position + 3]);
                    getOpl()->writeReg(0x43 + s_opTable[effectValue], m_song[m_position + 4]);
                    getOpl()->writeReg(0x60 + s_opTable[effectValue], m_song[m_position + 5]);
                    getOpl()->writeReg(0x63 + s_opTable[effectValue], m_song[m_position + 6]);
                    getOpl()->writeReg(0x80 + s_opTable[effectValue], m_song[m_position + 7]);
                    getOpl()->writeReg(0x83 + s_opTable[effectValue], m_song[m_position + 8]);
                    getOpl()->writeReg(0xe0 + s_opTable[effectValue], m_song[m_position + 9]);
                    getOpl()->writeReg(0xe3 + s_opTable[effectValue], m_song[m_position + 10]);
                    getOpl()->writeReg(0xc0 + effectValue, m_song[m_position + 11]);
                }
                m_position += 12;
                break;
            case 0x50: // set label
                m_labels[effectValue].target = ++m_position;
                m_labels[effectValue].defined = true;
                break;
            case 0x60: // jump
                if( m_labels[effectValue].defined )
                {
                    switch( m_song[m_position + 1] )
                    {
                        case 0xfe: // infinite loop
                            if( m_labels[effectValue].defined )
                            {
                                m_position = m_labels[effectValue].target;
                                m_songEnd = true;
                                break;
                            }
                            // fall through...
                        case 0xff: // chorus
                            if( !m_chorus && m_labels[effectValue].defined )
                            {
                                m_chorus = true;
                                m_goSub = m_position + 2;
                                m_position = m_labels[effectValue].target;
                                break;
                            }
                            // fall through...
                        case 0: // end of loop
                            m_position += 2;
                            break;
                        default: // finite loop
                            if( !m_labels[effectValue].count )
                            { // loop elapsed
                                m_labels[effectValue].count = 255;
                                m_position += 2;
                                break;
                            }
                            if( m_labels[effectValue].count < 255 )
                            { // loop defined
                                m_labels[effectValue].count--;
                            }
                            else
                            { // loop undefined
                                m_labels[effectValue].count = m_song[m_position + 1] - 1;
                            }
                            m_position = m_labels[effectValue].target;
                            break;
                    }
                }
                break;
            case 0x70: // end of chorus
                if( m_chorus )
                {
                    m_position = m_goSub;
                    m_chorus = false;
                }
                else
                {
                    m_position++;
                }
                break;
            default: // reserved command (skip)
                m_position++;
                break;
        }
    }
    if( m_song[m_position] >= 0x80 )
    { // wait
        m_delay = m_song[m_position] - 0x7f;
        m_position++;
    }
    return !m_songEnd;
}

void BamPlayer::rewind(const boost::optional<size_t>&)
{
    m_position = 0;
    m_songEnd = false;
    m_delay = 0;
    m_goSub = 0;
    m_chorus = false;
    m_labels.fill(Label());
    getOpl()->writeReg(1, 32);
}
