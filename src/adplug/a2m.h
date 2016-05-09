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

#include "protrack.h"

class Ca2mLoader : public CmodPlayer
{
    DISABLE_COPY(Ca2mLoader)
public:
    Ca2mLoader() = default;
    static Player *factory();

    bool load(const std::string &filename) override;
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
    uint32_t instrumentCount() const override
    {
        return 250;
    }
    std::string instrumentTitle(size_t n) const override
    {
        return m_instname[n];
    }

private:

    static constexpr auto COPYRANGES = 6;
    static constexpr auto FIRSTCODE = 257;
    static constexpr auto MINCOPY = 3;
    static constexpr auto MAXCOPY = 255;
    static constexpr auto CODESPERRANGE = (MAXCOPY - MINCOPY + 1);
    static constexpr auto MAXCHAR = (FIRSTCODE + COPYRANGES * CODESPERRANGE - 1);
    static constexpr auto TWICEMAX = (2 * MAXCHAR + 1);

    static constexpr auto MAXFREQ = 2000;
    static constexpr auto TERMINATE = 256;
    static constexpr auto SUCCMAX = MAXCHAR + 1;
    static constexpr auto ROOT = 1;
    static constexpr auto MAXBUF = 42 * 1024;
    static constexpr auto MAXDISTANCE = 21389;
    static constexpr auto MAXSIZE = 21389 + MAXCOPY;

    void initTree();
    void updateFreq(uint16_t a, uint16_t b);
    void updateModel(uint16_t code);
    uint16_t inputCode(uint16_t bits);
    uint16_t uncompress();
    void decode();
    size_t sixDepak(uint16_t *source, uint8_t *dest, size_t size);

    std::string m_songname{};
    std::string m_author{};
    std::array<std::string, 250> m_instname{ {} };

    uint16_t m_bitcount = 0;
    uint16_t m_bitbuffer = 0;
    uint16_t m_bufcount = 0;
    uint16_t m_obufcount = 0;
    size_t m_inputSize = 0;
    size_t m_outputSize = 0;
    uint16_t m_leftc[MAXCHAR + 1];
    uint16_t m_rightc[MAXCHAR + 1];
    uint16_t m_dad[TWICEMAX + 1];
    uint16_t m_freq[TWICEMAX + 1];
    uint16_t* m_wdbuf = nullptr;
    uint8_t* m_obuf = nullptr;
    std::vector<uint8_t> m_buf{};
};
