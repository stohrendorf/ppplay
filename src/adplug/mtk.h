/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * mtk.h - MPU-401 Trakker Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "hsc.h"

class CmtkLoader : public ChscPlayer {
    DISABLE_COPY(CmtkLoader)
    public:
        static CPlayer *factory();

    CmtkLoader() : ChscPlayer(true)
    {
    }

    bool load(const std::string &filename);

    std::string type() const
    {
        return "MPU-401 Trakker";
    }
    std::string title() const
    {
        return m_title;
    }
    std::string author() const
    {
        return composer;
    }
    uint32_t instrumentCount() const
    {
        return 128;
    }
    std::string instrumentTitle(size_t n) const
    {
        return instname[n];
    }

private:
    char m_title[34];
    char composer[34];
    char instname[0x80][34]{};
};
