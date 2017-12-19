#pragma once
/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
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
 * players.h - Players enumeration, by Simon Peter <dn.tlp@gmx.net>
 */

#include "ymf262/opl3.h"
#include "player.h"

#include <list>

class PlayerDesc
{
public:
    typedef Player* (* Factory)();

    Factory factory = nullptr;
    std::string filetype{};

    PlayerDesc() = default;

    PlayerDesc(Factory f, const std::string& type, const std::vector<std::string>& ext);

    ~PlayerDesc() = default;

    std::string get_extension(size_t n) const;

private:
    std::vector<std::string> extensions{};
};

class Players
{
private:
    std::list<const PlayerDesc*> m_descriptions{};
public:
    void addPlayerDescription(const PlayerDesc* desc)
    {
        m_descriptions.push_back(desc);
    }

    std::list<const PlayerDesc*>::const_iterator begin() const
    {
        return m_descriptions.cbegin();
    }

    std::list<const PlayerDesc*>::const_iterator end() const
    {
        return m_descriptions.cend();
    }
};
