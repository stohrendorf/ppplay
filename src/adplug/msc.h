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
 * msc.h - MSC Player by Lubomir Bulej (pallas@kadan.cz)
 */

#include "player.h"

class FileStream;

class MscPlayer : public Player
{
    DISABLE_COPY(MscPlayer)
public:
    static Player *factory();

    MscPlayer() = default;
    ~MscPlayer() = default;

    bool load(const std::string &filename) override;
    bool update() override;
    void rewind(int) override;
    size_t framesUntilUpdate() const override;

    std::string type() const override;

    static constexpr auto MSC_SIGN_LEN = 16;
    static constexpr auto MSC_DESC_LEN = 64;

private:
#pragma pack(push,1)
    struct msc_header
    {
        uint8_t mh_sign[MSC_SIGN_LEN];
        uint16_t mh_ver;
        uint8_t mh_desc[MSC_DESC_LEN];
        uint16_t mh_timer;
        uint16_t mh_nr_blocks;
        uint16_t mh_block_len;
    };
#pragma pack(pop)

    // file data
    std::string m_description{};               // song desctiption
    uint16_t m_version = 0;   // file version
    std::vector<std::vector<uint8_t>> m_mscData{ {} };      // compressed music data

    // decoder state
    size_t m_mscDataIndex = 0; // active block
    size_t m_blockPos = 0; // position in block
    size_t m_rawDataPos = 0;   // position in data buffer
    std::vector<uint8_t> m_rawData{};            // decompression buffer

    uint8_t m_decoderPrefix = 0;        // prefix / state
    size_t m_prefixDistance = 0;         // prefix distance
    uint32_t m_prefixLength = 0; // prefix length

    // player state
    uint8_t m_delay = 0;    // active delay

    static bool load_header(FileStream& bf, msc_header *hdr);
    bool decode_octet(uint8_t *output);
};
