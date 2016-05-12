#pragma once

/*
  AdPlug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2006 Simon Peter <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  cff.h - BoomTracker loader by Riven the Mage <riven@ok.ru>
*/

#include "mod.h"

class CffPlayer : public ModPlayer
{
    DISABLE_COPY(CffPlayer)
public:
    static Player *factory();

    CffPlayer() = default;

    bool load(const std::string &filename) override;
    void rewind(int subsong) override;

    std::string type() const override;
    std::string title() const override;
    std::string author() const override;
    std::string instrumentTitle(size_t n) const override;
    size_t instrumentCount() const override;

private:

    class cff_unpacker
    {
    public:

        size_t unpack(uint8_t *ibuf, uint8_t *obuf);

    private:

        uint32_t get_code();
        static void translate_code(unsigned long code, uint8_t *string, const std::vector<std::vector<uint8_t> > &dictionary);

        void cleanup();
        bool startup(const std::vector<std::vector<uint8_t> > &dictionary);

        static void expand_dictionary(uint8_t *string, std::vector<std::vector<uint8_t> > &dictionary);

        uint8_t *m_input;
        uint8_t *m_output;

        size_t m_outputLength;

        uint8_t m_codeLength;

        unsigned long m_bitsBuffer;
        unsigned int m_bitsLeft;

        uint8_t m_theString[256];
    };

#pragma pack(push,1)
    struct cff_header
    {
        char id[16] = "";
        uint8_t version = 0;
        uint16_t size = 0;
        uint8_t packed = 0;
        uint8_t reserved[12] = { 0 };
    };
    cff_header header{};
#pragma pack(pop)

    struct cff_instrument
    {
        uint8_t data[12];
        char name[21];
    } instruments[47];

    std::string song_title;
    std::string song_author;

    struct cff_event
    {
        uint8_t byte0;
        uint8_t byte1;
        uint8_t byte2;
    };
};
