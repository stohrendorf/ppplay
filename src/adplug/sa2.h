#pragma once

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
 * sa2.h - SAdT2 Loader by Simon Peter <dn.tlp@gmx.net>
 *         SAdT Loader by Mamiya <mamiya@users.sourceforge.net>
 */

#include "mod.h"

class Sa2Player : public ModPlayer
{
    DISABLE_COPY(Sa2Player)
public:
    static Player *factory();

    Sa2Player() = default;

    bool load(const std::string &filename) override;

    std::string type() const override;
    std::string title() const override;
    size_t instrumentCount() const override
    {
        return 29;
    }
    std::string instrumentTitle(size_t n) const override
    {
        if(n < m_instrumentNames.size())
            return m_instrumentNames[n].substr(1);

        return {};
    }

private:
    struct Header
    {
        char sadt[4] = "";
        uint8_t version = 0;
    };
    Header m_header{};

    std::array<std::string, 29> m_instrumentNames;
};
