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

class CmscPlayer : public CPlayer {
    DISABLE_COPY(CmscPlayer)
    public:
        static CPlayer *factory();

    CmscPlayer();
    ~CmscPlayer() = default;

    bool load(const std::string &filename);
    bool update();
    void rewind(int);
    size_t framesUntilUpdate();

    std::string gettype();

    static constexpr auto MSC_SIGN_LEN = 16;
    static constexpr auto MSC_DESC_LEN = 64;

protected:

    struct msc_header {
        uint8_t mh_sign[MSC_SIGN_LEN];
        uint16_t mh_ver;
        uint8_t mh_desc[MSC_DESC_LEN];
        uint16_t mh_timer;
        uint16_t mh_nr_blocks;
        uint16_t mh_block_len;
    };

    struct msc_block {
        uint16_t mb_length;
        std::vector<uint8_t> mb_data;
    };

    // file data
    std::string desc;               // song desctiption
    unsigned short version;   // file version
    unsigned short nr_blocks; // number of music blocks
    unsigned short block_len; // maximal block length
    unsigned short timer_div; // timer divisor
    std::vector<msc_block> msc_data;      // compressed music data

    // decoder state
    unsigned long block_num; // active block
    unsigned long block_pos; // position in block
    unsigned long raw_pos;   // position in data buffer
    std::vector<uint8_t> raw_data;            // decompression buffer

    uint8_t dec_prefix;        // prefix / state
    int dec_dist;         // prefix distance
    unsigned int dec_len; // prefix length

    // player state
    unsigned char delay;    // active delay
    unsigned long play_pos; // player position

private:
    static const uint8_t msc_signature[MSC_SIGN_LEN];

    bool load_header(FileStream& bf, msc_header *hdr);
    bool decode_octet(uint8_t *output);
};
