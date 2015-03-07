/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2006 Simon Peter, <dn.tlp@gmx.net>, et al.

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

  dtm.cpp - DTM loader by Riven the Mage <riven@ok.ru>
*/
/*
  NOTE: Panning (Ex) effect is ignored.
*/

#include <cstring>

#include "stream/filestream.h"

#include "dtm.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CdtmLoader::factory() { return new CdtmLoader(); }

bool CdtmLoader::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
    const unsigned char conv_inst[11] = { 2, 1, 10, 9, 4, 3, 6, 5, 0, 8, 7 };
    const std::array<uint16_t,12> conv_note = { 0x16B, 0x181, 0x198, 0x1B0, 0x1CA,
                                                0x1E5, 0x202, 0x220, 0x241, 0x263,
                                                0x287, 0x2AE };
    // read header
    f.read(header.id, 12);
    f >> header.version;
    f.read(header.title, 20);
    f.read(header.author, 20);
    f >>header.numpat >> header.numinst;

    // signature exists ? good version ?
    if (memcmp(header.id, "DeFy DTM ", 9) || header.version != 0x10) {
        return false;
    }

    header.numinst++;

    // load description
    memset(desc, 0, 80 * 16);

    char bufstr[80];

    for (int i = 0; i < 16; i++) {
        // get line length
        uint8_t bufstr_length;
        f >> bufstr_length;

        if (bufstr_length > 80) {
            return false;
        }

        // read line
        if (bufstr_length) {
            f.read(bufstr, bufstr_length);

            for (int j = 0; j < bufstr_length; j++)
                if (!bufstr[j])
                    bufstr[j] = 0x20;

            bufstr[bufstr_length] = 0;

            strcat(desc, bufstr);
        }

        strcat(desc, "\n");
    }

    // init CmodPlayer
    m_instruments.clear();
    m_instruments.resize(header.numinst);
    m_order.resize(100);
    realloc_patterns(header.numpat, 64, 9);
    init_notetable(conv_note);
    init_trackord();

    // load instruments
    for (int i = 0; i < header.numinst; i++) {
        uint8_t name_length;
        f >> name_length;

        if (name_length)
            f.read(instruments[i].name, name_length);

        instruments[i].name[name_length] = 0;

        f.read(instruments[i].data, 12);

        for (int j = 0; j < 11; j++)
            m_instruments[i].data[conv_inst[j]] = instruments[i].data[j];
    }

    // load order
    f.read(m_order.data(), 100);

    numberOfPatterns = header.numpat;

    std::vector<uint8_t> pattern(0x480);

    // load tracks
    for (int i = 0; i < numberOfPatterns; i++) {
        uint16_t packed_length;
        f >> packed_length;

        std::vector<uint8_t> packed_pattern(packed_length);
        f.read(packed_pattern.data(), packed_length);

        auto unpacked_length = unpack_pattern(packed_pattern.data(), packed_length, pattern.data(), 0x480);

        if (!unpacked_length) {
            return false;
        }

        // convert pattern
        int t = 0;
        for (int j = 0; j < 9; j++) {
            for (int k = 0; k < 64; k++) {
                dtm_event *event = (dtm_event *)&pattern[(k * 9 + j) * 2];

                if (event->byte0 == 0x80) {
                    // instrument
                    if (event->byte1 <= 0x80)
                        m_tracks.at(t,k).inst = event->byte1 + 1;
                }
                else {
                    // note + effect
                    m_tracks.at(t,k).note = event->byte0;

                    if ((event->byte0 != 0) && (event->byte0 != 127))
                        m_tracks.at(t,k).note++;

                    // convert effects
                    switch (event->byte1 >> 4) {
                    case 0x0: // pattern break
                        if ((event->byte1 & 15) == 1)
                            m_tracks.at(t,k).command = 13;
                        break;

                    case 0x1: // freq. slide up
                        m_tracks.at(t,k).command = 28;
                        m_tracks.at(t,k).param1 = event->byte1 & 15;
                        break;

                    case 0x2: // freq. slide down
                        m_tracks.at(t,k).command = 28;
                        m_tracks.at(t,k).param2 = event->byte1 & 15;
                        break;

                    case 0xA: // set carrier volume
                    case 0xC: // set instrument volume
                        m_tracks.at(t,k).command = 22;
                        m_tracks.at(t,k).param1 = (0x3F - (event->byte1 & 15)) >> 4;
                        m_tracks.at(t,k).param2 = (0x3F - (event->byte1 & 15)) & 15;
                        break;

                    case 0xB: // set modulator volume
                        m_tracks.at(t,k).command = 21;
                        m_tracks.at(t,k).param1 = (0x3F - (event->byte1 & 15)) >> 4;
                        m_tracks.at(t,k).param2 = (0x3F - (event->byte1 & 15)) & 15;
                        break;

                    case 0xE: // set panning
                        break;

                    case 0xF: // set speed
                        m_tracks.at(t,k).command = 13;
                        m_tracks.at(t,k).param2 = event->byte1 & 15;
                        break;
                    }
                }
            }

            t++;
        }
    }

    // order length
    for (int i = 0; i < 100; i++) {
        if (m_order[i] >= 0x80) {
            m_length = i;

            if (m_order[i] == 0xFF)
                m_restartpos = 0;
            else
                m_restartpos = m_order[i] - 0x80;

            break;
        }
    }

    // initial speed
    m_initspeed = 2;

    rewind(0);

    return true;
}

void CdtmLoader::rewind(int subsong) {
    CmodPlayer::rewind(subsong);

    // default instruments
    for (int i = 0; i < 9; i++) {
        channel[i].inst = i;

        channel[i].vol1 = 63 - (m_instruments[i].data[10] & 63);
        channel[i].vol2 = 63 - (m_instruments[i].data[9] & 63);
    }
}

size_t CdtmLoader::framesUntilUpdate() { return SampleRate / 18.2; }

std::string CdtmLoader::gettype() { return std::string("DeFy Adlib Tracker"); }

std::string CdtmLoader::gettitle() { return std::string(header.title); }

std::string CdtmLoader::getauthor() { return std::string(header.author); }

std::string CdtmLoader::getdesc() { return std::string(desc); }

std::string CdtmLoader::getinstrument(unsigned int n) {
    return std::string(instruments[n].name);
}

unsigned int CdtmLoader::getinstruments() { return header.numinst; }

/* -------- Private Methods ------------------------------- */

long CdtmLoader::unpack_pattern(unsigned char *ibuf, long ilen,
                                unsigned char *obuf, long olen) {
    unsigned char *input = ibuf;
    unsigned char *output = obuf;

    long input_length = 0;
    long output_length = 0;

    unsigned char repeat_byte, repeat_counter;

    // RLE
    while (input_length < ilen) {
        repeat_byte = input[input_length++];

        if ((repeat_byte & 0xF0) == 0xD0) {
            repeat_counter = repeat_byte & 15;
            repeat_byte = input[input_length++];
        } else
            repeat_counter = 1;

        for (int i = 0; i < repeat_counter; i++) {
            if (output_length < olen)
                output[output_length++] = repeat_byte;
        }
    }

    return output_length;
}
