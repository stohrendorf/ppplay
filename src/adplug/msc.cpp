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

/*** public methods *************************************/

CPlayer *CmscPlayer::factory() { return new CmscPlayer(); }

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
    m_version = hdr.mh_ver;
    setInitialTempo(hdr.mh_timer);
    setCurrentTempo(hdr.mh_timer);
    addOrder(0);

    if (!hdr.mh_nr_blocks) {
        return false;
    }

    // load compressed data blocks
    m_rawData.resize(hdr.mh_block_len);

    for (int blk_num = 0; blk_num < hdr.mh_nr_blocks; blk_num++) {
        m_mscData.emplace_back();
        uint16_t blockLength;
        bf >> blockLength;
        m_mscData.back().resize(blockLength);
        bf.read(m_mscData.back().data(), blockLength);
    }

    // clean up & initialize
    rewind(0);

    return true;
}

bool CmscPlayer::update() {
    // output data
    while (!m_delay) {
        // decode data
        uint8_t cmnd;
        if (!decode_octet(&cmnd))
            return false;

        uint8_t data;
        if (!decode_octet(&data))
            return false;

        // check for special commands
        switch (cmnd) {
            case 0xff:
                // delay
                m_delay = data;
                break;

            default:
                // play command & data
                getOpl()->writeReg(cmnd, data);
        } // command switch
    }   // play pass

    // count delays
    if (m_delay)
        m_delay--;

    return true;
}

void CmscPlayer::rewind(int) {
    // reset state
    m_decoderPrefix = 0;
    m_mscDataIndex = 0;
    m_blockPos = 0;
    m_rawDataPos = 0;
    m_delay = 0;

    // init the OPL chip and go to OPL2 mode
    getOpl()->writeReg(1, 32);
}

size_t CmscPlayer::framesUntilUpdate() const {
    // PC timer oscillator frequency / wait register
    return SampleRate * (currentTempo() ? currentTempo() : 0xffff) / 1193180;
}

std::string CmscPlayer::type() const {
    char vstr[40];

    sprintf(vstr, "AdLib MSCplay (version %d)", m_version);
    return vstr;
}

/*** private methods *************************************/

bool CmscPlayer::load_header(FileStream &bf, msc_header *hdr) {
    static const uint8_t msc_signature[MSC_SIGN_LEN] = {
        'C', 'e', 'r', 'e', 's', ' ', '\x13', ' ', 'M', 'S', 'C', 'p', 'l', 'a', 'y',
        ' '
    };

    BOOST_ASSERT(hdr != nullptr);
    bf >> *hdr;

    // check signature
    if (memcmp(msc_signature, hdr->mh_sign, MSC_SIGN_LEN) != 0)
        return false;

    // check version
    if (hdr->mh_ver != 0)
        return false;

    return true;
}

bool CmscPlayer::decode_octet(uint8_t *output) {
    if (m_mscDataIndex >= m_mscData.size())
        return false;

    const std::vector<uint8_t>* blkData = &m_mscData[m_mscDataIndex];
    while (true) {

        // advance to next block if necessary
        if (m_blockPos >= blkData->size() && m_prefixLength == 0) {
            m_mscDataIndex++;
            if (m_mscDataIndex >= m_mscData.size())
                return false;

            blkData = &m_mscData[m_mscDataIndex];
            m_blockPos = 0;
            m_rawDataPos = 0;
        }

        // decode the compressed music data
        uint8_t octet;        // decoded octet
        switch (m_decoderPrefix) {

        // decode prefix
        case 155:
        case 175:
            octet = (*blkData)[m_blockPos++];
            if (octet == 0) {
                // invalid prefix, output original
                octet = m_decoderPrefix;
                m_decoderPrefix = 0;
                break;
            }

            // isolate length and distance
            m_prefixLength = (octet & 0x0F);
            // len_corr = 2;

            m_prefixDistance = (octet & 0xF0) >> 4;
            if (m_decoderPrefix == 155)
                m_prefixDistance++;

            // next decode step for respective prefix type
            m_decoderPrefix++;
            continue;

            // check for extended length
        case 156:
            if (m_prefixLength == 15)
                m_prefixLength += (*blkData)[m_blockPos++];

            // add length correction and go for copy mode
            m_decoderPrefix = 255;
            continue;

            // get extended distance
        case 176:
            m_prefixDistance += 17 + 16 * (*blkData)[m_blockPos++];
            // len_corr = 3;

            // check for extended length
            m_decoderPrefix = 156;
            continue;

            // prefix copy mode
        case 255:
            if (m_rawDataPos >= m_prefixDistance)
                octet = m_rawData.at(m_rawDataPos - m_prefixDistance);
            else {
                AdPlug_LogWrite("error! read before raw_data buffer.\n");
                octet = 0;
            }

            m_prefixLength--;
            if (m_prefixLength == 0) {
                // back to normal mode
                m_decoderPrefix = 0;
            }

            break;

            // normal mode
        default:
            octet = (*blkData)[m_blockPos++];
            if (octet == 155 || octet == 175) {
                // it's a prefix, restart
                m_decoderPrefix = octet;
                continue;
            }
        } // prefix switch

        // output the octet
        if (output != nullptr)
            *output = octet;

        m_rawData.at(m_rawDataPos++) = octet;
        break;
    }
    ; // decode pass

    return true;
}
