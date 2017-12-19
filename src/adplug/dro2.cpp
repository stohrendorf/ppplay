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
 * dro2.cpp - DOSBox Raw OPL v2.0 player by Adam Nielsen
 * <malvineous@shikadi.net>
 */

#include <cstdio>

#include "stream/filestream.h"

#include "dro2.h"

Player* Dro2Player::factory()
{
    return new Dro2Player();
}

bool Dro2Player::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }

    char id[8];
    f.read(id, 8);
    if( strncmp(id, "DBRAWOPL", 8) != 0 )
    {
        return false;
    }
    uint32_t version;
    f >> version;
    if( version != 0x2 )
    {
        return false;
    }

    uint32_t length;
    f >> length;
    length *= 2; // stored in file as number of byte pairs
    f.seekrel(4); // Length in milliseconds
    f.seekrel(1); /// OPL type (0 == OPL2, 1 == Dual OPL2, 2 == OPL3)
    uint8_t iFormat;
    f >> iFormat;
    if( iFormat != 0 )
    {
        return false;
    }
    uint8_t iCompression;
    f >> iCompression;
    if( iCompression != 0 )
    {
        return false;
    }
    uint8_t convTableLength;
    f >> m_commandDelayS >> m_commandDelay >> convTableLength;

    m_convTable.resize(convTableLength);
    f.read(m_convTable.data(), convTableLength);

    m_data.resize(length);
    f.read(m_data.data(), length);

    rewind(size_t(0));

    return true;
}

bool Dro2Player::update()
{
    while( m_pos < m_data.size() )
    {
        auto iIndex = m_data[m_pos++];
        const auto iValue = m_data[m_pos++];

        // Short delay
        if( iIndex == m_commandDelayS )
        {
            m_delay = iValue + 1;
            return true;
        }

        // Long delay
        if( iIndex == m_commandDelay )
        {
            m_delay = (iValue + 1) << 8;
            return true;
        }

        // Normal write
        if( iIndex & 0x80 )
        {
            // High bit means use second chip in dual-OPL2 config
            //FIXME sto opl->setchip(1);
            m_chipSelector = 0x100;
            iIndex &= 0x7F;
        }
        else
        {
            //FIXME sto opl->setchip(0);
            m_chipSelector = 0;
        }
        if( iIndex > m_convTable.size() )
        {
            std::cout << "DRO2: Error - index beyond end of codemap table!  Corrupted .dro?\n";
            return false; // EOF
        }
        int iReg = m_convTable[iIndex];
        getOpl()->writeReg(m_chipSelector + iReg, iValue);
    }

    // This won't result in endless-play using Adplay, but IMHO that code belongs
    // in Adplay itself, not here.
    return m_pos < m_data.size();
}

void Dro2Player::rewind(const boost::optional<size_t>&)
{
    m_delay = 0;
    m_pos = 0;
}

size_t Dro2Player::framesUntilUpdate() const
{
    if( m_delay > 0 )
    {
        return SampleRate * m_delay / 1000;
    }
    else
    {
        return SampleRate / 1000;
    }
}
