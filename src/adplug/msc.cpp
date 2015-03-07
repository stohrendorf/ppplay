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
 * msc.c - MSC Player by Lubomir Bulej (pallas@kadan.cz)
 */

#include <cstring>
#include <cstdio>

#include "stream/filestream.h"

#include "msc.h"
#include "debug.h"

const unsigned char CmscPlayer::msc_signature[MSC_SIGN_LEN] = {
    'C', 'e', 'r', 'e', 's', ' ', '\x13', ' ', 'M', 'S', 'C', 'p', 'l', 'a', 'y',
    ' '
};

/*** public methods *************************************/

CPlayer *CmscPlayer::factory() { return new CmscPlayer(); }

CmscPlayer::CmscPlayer() : CPlayer() {
    nr_blocks = 0;
}

bool CmscPlayer::load(const std::string &filename) {

    // open and validate the file
    FileStream bf(filename);
    if (!bf)
        return false;

    msc_header hdr;
    if (!load_header(bf, &hdr)) {
        return false;
    }

    // get stuff from the header
    version = hdr.mh_ver;
    timer_div = hdr.mh_timer;
    nr_blocks = hdr.mh_nr_blocks;
    block_len = hdr.mh_block_len;

    if (!nr_blocks) {
        return false;
    }

    // load compressed data blocks
    raw_data.resize(block_len);

    for (int blk_num = 0; blk_num < nr_blocks; blk_num++) {
        msc_data.emplace_back();
        bf >> msc_data.back().mb_length;
        msc_data.back().mb_data.resize(msc_data.back().mb_length);
        bf.read(msc_data.back().mb_data.data(), msc_data.back().mb_length);
    }

    // clean up & initialize
    rewind(0);

    return true;
}

bool CmscPlayer::update() {
    // output data
    while (!delay) {
        uint8_t cmnd;
        uint8_t data;

        // decode data
        if (!decode_octet(&cmnd))
            return false;

        if (!decode_octet(&data))
            return false;

        // check for special commands
        switch (cmnd) {

        // delay
        case 0xff:
            delay = 1 + (uint8_t)(data - 1);
            break;

            // play command & data
        default:
            getOpl()->writeReg(cmnd, data);

        } // command switch
    }   // play pass

    // count delays
    if (delay)
        delay--;

    // advance player position
    play_pos++;
    return true;
}

void CmscPlayer::rewind(int) {
    // reset state
    dec_prefix = 0;
    block_num = 0;
    block_pos = 0;
    play_pos = 0;
    raw_pos = 0;
    delay = 0;

    // init the OPL chip and go to OPL2 mode
    getOpl()->writeReg(1, 32);
}

size_t CmscPlayer::framesUntilUpdate() {
    // PC timer oscillator frequency / wait register
    return SampleRate * (timer_div ? timer_div : 0xffff) / 1193180;
}

std::string CmscPlayer::gettype() {
    char vstr[40];

    sprintf(vstr, "AdLib MSCplay (version %d)", version);
    return std::string(vstr);
}

/*** private methods *************************************/

bool CmscPlayer::load_header(FileStream &bf, msc_header *hdr) {
    // check signature
    bf.read(hdr->mh_sign, sizeof(hdr->mh_sign));
    if (memcmp(msc_signature, hdr->mh_sign, MSC_SIGN_LEN) != 0)
        return false;

    // check version
    bf >> hdr->mh_ver;
    if (hdr->mh_ver != 0)
        return false;

    bf.read(hdr->mh_desc, sizeof(hdr->mh_desc));
    bf >> hdr->mh_timer;
    bf >> hdr->mh_nr_blocks;
    bf >> hdr->mh_block_len;
    return true;
}

bool CmscPlayer::decode_octet(uint8_t *output) {
    msc_block blk; // compressed data block

    if (block_num >= nr_blocks)
        return false;

    blk = msc_data[block_num];
    while (1) {
        uint8_t octet;        // decoded octet
        uint8_t len_corr = 0; // length correction

        // advance to next block if necessary
        if (block_pos >= blk.mb_length && dec_len == 0) {
            block_num++;
            if (block_num >= nr_blocks)
                return false;

            blk = msc_data[block_num];
            block_pos = 0;
            raw_pos = 0;
        }

        // decode the compressed music data
        switch (dec_prefix) {

        // decode prefix
        case 155:
        case 175:
            octet = blk.mb_data[block_pos++];
            if (octet == 0) {
                // invalid prefix, output original
                octet = dec_prefix;
                dec_prefix = 0;
                break;
            }

            // isolate length and distance
            dec_len = (octet & 0x0F);
            // len_corr = 2;

            dec_dist = (octet & 0xF0) >> 4;
            if (dec_prefix == 155)
                dec_dist++;

            // next decode step for respective prefix type
            dec_prefix++;
            continue;

            // check for extended length
        case 156:
            if (dec_len == 15)
                dec_len += blk.mb_data[block_pos++];

            // add length correction and go for copy mode
            dec_len += len_corr;
            dec_prefix = 255;
            continue;

            // get extended distance
        case 176:
            dec_dist += 17 + 16 * blk.mb_data[block_pos++];
            // len_corr = 3;

            // check for extended length
            dec_prefix = 156;
            continue;

            // prefix copy mode
        case 255:
            if ((int) raw_pos >= dec_dist)
                octet = raw_data[raw_pos - dec_dist];
            else {
                AdPlug_LogWrite("error! read before raw_data buffer.\n");
                octet = 0;
            }

            dec_len--;
            if (dec_len == 0) {
                // back to normal mode
                dec_prefix = 0;
            }

            break;

            // normal mode
        default:
            octet = blk.mb_data[block_pos++];
            if (octet == 155 || octet == 175) {
                // it's a prefix, restart
                dec_prefix = octet;
                continue;
            }
        } // prefix switch

        // output the octet
        if (output != NULL)
            *output = octet;

        raw_data[raw_pos++] = octet;
        break;
    }
    ; // decode pass

    return true;
}
