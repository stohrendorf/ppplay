#pragma once

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
 * a2m.h - A2M Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "mod.h"

class FileStream;

class A2mPlayer
    : public ModPlayer
{
public:
    DISABLE_COPY(A2mPlayer)

    A2mPlayer() = default;

    static Player* factory();

    bool load(const std::string& filename) override;

    size_t framesUntilUpdate() const override;

    std::string type() const override
    {
        return "AdLib Tracker 2";
    }

    std::string title() const override
    {
        return m_songname;
    }

    std::string author() const override
    {
        return m_author;
    }

    size_t instrumentCount() const override
    {
        return 250;
    }

    std::string instrumentTitle(size_t n) const override
    {
        return m_instname[n];
    }

private:
    std::string m_songname{};
    std::string m_author{};
    std::array<std::string, 250> m_instname{{}};

    void readHeader(FileStream& f, uint8_t version, uint16_t* lengths);
};
