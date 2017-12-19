#pragma once

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

class MtkPlayer
    : public HscPlayer
{
public:
    DISABLE_COPY(MtkPlayer)

    static Player* factory();

    MtkPlayer()
        : HscPlayer(true)
    {
    }

    bool load(const std::string& filename) override;

    std::string type() const override
    {
        return "MPU-401 Trakker";
    }

    std::string title() const override
    {
        return m_title;
    }

    std::string author() const override
    {
        return m_composer;
    }

    size_t instrumentCount() const override
    {
        return 128;
    }

    std::string instrumentTitle(size_t n) const override
    {
        return m_instrumentNames[n];
    }

private:
    std::string m_title{};
    std::string m_composer{};
    std::string m_instrumentNames[0x80]{};
};
