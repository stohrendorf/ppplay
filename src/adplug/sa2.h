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

#include "protrack.h"

class Csa2Loader : public CmodPlayer
{
    DISABLE_COPY(Csa2Loader)
public:
    static Player *factory();

    Csa2Loader() = default;

    bool load(const std::string &filename) override;

    std::string type() const override;
    std::string title() const override;
    size_t instrumentCount() const override
    {
        return 29;
    }
    std::string instrumentTitle(size_t n) const override
    {
        if(n < 29)
            return std::string(m_instrumentNames[n], 1, 16);
        else
            return "-broken-";
    }

private:
    struct sa2header
    {
        char sadt[4] = "";
        uint8_t version = 0;
    };
    sa2header m_header{};

    char m_instrumentNames[29][17];
};
