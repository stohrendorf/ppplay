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
 * a2m.cpp - A2M Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * This loader detects and loads version 1, 4, 5 & 8 files.
 *
 * version 1-4 files:
 * Following commands are ignored: FF1 - FF9, FAx - FEx
 *
 * version 5-8 files:
 * Instrument panning is ignored. Flags byte is ignored.
 * Following commands are ignored: Gxy, Hxy, Kxy - &xy
 */

#include <cstring>

#include "stream/filestream.h"
#include "a2m.h"

namespace {
constexpr uint16_t bitvalue[14] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };

constexpr std::array<int16_t,6> copybits = {{ 4, 6, 8, 10, 12, 14 }};

constexpr std::array<int16_t,6> copymin = {{ 0, 16, 80, 336, 1360, 5456 }};
}

CPlayer *Ca2mLoader::factory() { return new Ca2mLoader(); }

bool Ca2mLoader::load(const std::string &filename) {
    FileStream f(filename);
    if(!f)
        return false;
    const unsigned char convfx[16] = { 0, 1, 2, 23, 24, 3, 5, 4, 6, 9, 17, 13, 11,
                                       19, 7, 14 };
    const unsigned char convinf1[16] = { 0, 1, 2, 6, 7, 8, 9, 4, 5, 3, 10, 11, 12,
                                         13, 14, 15 };
    const unsigned char newconvfx[] = { 0, 1, 2, 3, 4, 5, 6, 23, 24, 21, 10, 11,
                                        17, 13, 7, 19, 255, 255, 22, 25, 255, 15,
                                        255, 255, 255, 255, 255, 255, 255, 255,
                                        255, 255, 255, 255, 255, 14, 255 };

    // read header
    char id[10];
    f.read(id, 10);
    f.seekrel(4);
    uint8_t version;
    f >> version;
    uint8_t numpats;
    f >> numpats;

    // file validation section
    if (strncmp(id, "_A2module_", 10) || (version != 1 && version != 5 && version != 4 && version != 8)) {
        return false;
    }

    // load, depack & convert section
    numberOfPatterns = numpats;
    m_length = 128;
    m_restartpos = 0;
    uint16_t len[9];
    int t = 0;
    if (version < 5) {
        f.read(len, 5);
        t = 9;
    } else { // version >= 5
        f.read(len, 9);
        t = 18;
    }

    // block 0
    std::vector<uint16_t> m_secData;
    m_secData.resize(len[0] / 2);
    std::vector<uint8_t> m_org;
    size_t m_orgPos = 0;
    if (version == 1 || version == 5) {
        f.read(m_secData.data(), len[0]/2);
        m_org.resize(MAXBUF);
        sixdepak(m_secData.data(), m_org.data(), len[0]);
    }
    else {
        f.read(m_org.data(), len[0]);
    }
    m_songname.assign(&m_org[m_orgPos], &m_org[m_orgPos] + 42);
    m_orgPos += 43;
    m_author.assign(&m_org[m_orgPos], &m_org[m_orgPos] + 42);
    m_orgPos += 43;
    for (int i = 0; i < 250; ++i) {
        m_songname.assign(&m_org[m_orgPos], &m_org[m_orgPos] + 32);
        m_orgPos += 33;
    }

    for (int i = 0; i < 250; i++) { // instruments
        m_instruments[i].data[0] = m_org[m_orgPos + 10];
        m_instruments[i].data[1] = m_org[m_orgPos + 0];
        m_instruments[i].data[2] = m_org[m_orgPos + 1];
        m_instruments[i].data[3] = m_org[m_orgPos + 4];
        m_instruments[i].data[4] = m_org[m_orgPos + 5];
        m_instruments[i].data[5] = m_org[m_orgPos + 6];
        m_instruments[i].data[6] = m_org[m_orgPos + 7];
        m_instruments[i].data[7] = m_org[m_orgPos + 8];
        m_instruments[i].data[8] = m_org[m_orgPos + 9];
        m_instruments[i].data[9] = m_org[m_orgPos + 2];
        m_instruments[i].data[10] = m_org[m_orgPos + 3];

        if (version < 5)
            m_instruments[i].misc = m_org[m_orgPos + 11];
        else { // version >= 5 -> OPL3 format
            int pan = m_org[m_orgPos + 11];

            if (pan)
                m_instruments[i].data[0] |= (pan & 3) << 4; // set pan
            else
                m_instruments[i].data[0] |= 48; // enable both speakers
        }

        m_instruments[i].slide = m_org[m_orgPos + 12];
        m_orgPos += 13;
    }

    m_order.clear();
    std::copy_n(m_org.begin()+m_orgPos, 128, std::back_inserter(m_order));
    m_orgPos += 128;
    m_bpm = m_org[m_orgPos++];
    m_initspeed = m_org[m_orgPos++];
    uint8_t flags = 0;
    if (version >= 5)
        flags = m_org[m_orgPos];
    if (version == 1 || version == 5)
        m_org.clear();
    m_secData.clear();

    // blocks 1-4 or 1-8
    size_t alength = len[1];
    for (int i = 0; i < (version < 5 ? numpats / 16 : numpats / 8); i++)
        alength += len[i + 2];

    m_secData.resize(alength / 2);
    if (version == 1 || version == 5) {

        f.read(m_secData.data(), alength/2);
        m_org.resize(MAXBUF * (numpats / (version == 1 ? 16 : 8) + 1));
        m_orgPos = 0;
        size_t secPos = 0;
        m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[1]);
        secPos += len[1] / 2;
        if (version == 1) {
            if (numpats > 16)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[2]);
            secPos += len[2] / 2;
            if (numpats > 32)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[3]);
            secPos += len[3] / 2;
            if (numpats > 48)
                sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[4]);
        } else {
            if (numpats > 8)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[2]);
            secPos += len[2] / 2;
            if (numpats > 16)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[3]);
            secPos += len[3] / 2;
            if (numpats > 24)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[4]);
            secPos += len[4] / 2;
            if (numpats > 32)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[5]);
            secPos += len[5] / 2;
            if (numpats > 40)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[6]);
            secPos += len[6] / 2;
            if (numpats > 48)
                m_orgPos += sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[7]);
            secPos += len[7] / 2;
            if (numpats > 56)
                sixdepak(&m_secData[secPos], &m_org[m_orgPos], len[8]);
        }
        m_secData.clear();
    }
    else {
        m_org.assign(
                    reinterpret_cast<const uint8_t *>(m_secData.data()),
                    reinterpret_cast<const uint8_t *>(m_secData.data() + m_secData.size()));
        f.read(m_org.data(), alength);
    }

    if (version < 5) {
        for (int i = 0; i < numpats; i++) {
            for (int j = 0; j < 64; j++) {
                for (int k = 0; k < 9; k++) {
                    struct Track *track = &m_tracks.at(i * 9 + k, j);
                    unsigned char *o = &m_org[i * 64 * t * 4 + j * t * 4 + k * 4];

                    track->note = o[0] == 255 ? 127 : o[0];
                    track->inst = o[1];
                    track->command = convfx[o[2]];
                    track->param2 = o[3] & 0x0f;
                    if (track->command != 14)
                        track->param1 = o[3] >> 4;
                    else {
                        track->param1 = convinf1[o[3] >> 4];
                        if (track->param1 == 15 && !track->param2) { // convert key-off
                            track->command = 8;
                            track->param1 = 0;
                            track->param2 = 0;
                        }
                    }
                    if (track->command == 14) {
                        switch (track->param1) {
                        case 2: // convert define waveform
                            track->command = 25;
                            track->param1 = track->param2;
                            track->param2 = 0xf;
                            break;
                        case 8: // convert volume slide up
                            track->command = 26;
                            track->param1 = track->param2;
                            track->param2 = 0;
                            break;
                        case 9: // convert volume slide down
                            track->command = 26;
                            track->param1 = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
    else { // version >= 5
        realloc_patterns(64, 64, 18);

        for (int i = 0; i < numpats; i++) {
            for (int j = 0; j < 18; j++) {
                for (int k = 0; k < 64; k++) {
                    struct Track *track = &m_tracks.at(i * 18 + j, k);
                    unsigned char *o = &m_org[i * 64 * t * 4 + j * 64 * 4 + k * 4];

                    track->note = o[0] == 255 ? 127 : o[0];
                    track->inst = o[1];
                    track->command = newconvfx[o[2]];
                    track->param1 = o[3] >> 4;
                    track->param2 = o[3] & 0x0f;

                    // Convert '&' command
                    if (o[2] == 36)
                        switch (track->param1) {
                        case 0: // pattern delay (frames)
                            track->command = 29;
                            track->param1 = 0;
                            // param2 already set correctly
                            break;

                        case 1: // pattern delay (rows)
                            track->command = 14;
                            track->param1 = 8;
                            // param2 already set correctly
                            break;
                        }
                }
            }
        }
    }

    init_trackord();

    if (version == 1 || version == 5)
        m_org.clear();
    else
        m_secData.clear();

    // Process flags
    if (version >= 5) {
        CmodPlayer::m_flags |= Opl3; // All versions >= 5 are OPL3
        if (flags & 8)
            CmodPlayer::m_flags |= Tremolo; // Tremolo depth
        if (flags & 16)
            CmodPlayer::m_flags |= Vibrato; // Vibrato depth
    }

    rewind(0);
    return true;
}

size_t Ca2mLoader::framesUntilUpdate() {
    if (m_tempo != 18)
        return SampleRate / m_tempo;
    else
        return SampleRate / 18.2;
}

/*** private methods *************************************/

void Ca2mLoader::inittree() {
    unsigned short i;

    for (i = 2; i <= TWICEMAX; i++) {
        m_dad[i] = i / 2;
        m_freq[i] = 1;
    }

    for (i = 1; i <= MAXCHAR; i++) {
        m_leftc[i] = 2 * i;
        m_rightc[i] = 2 * i + 1;
    }
}

void Ca2mLoader::updatefreq(unsigned short a, unsigned short b) {
    do {
        m_freq[m_dad[a]] = m_freq[a] + m_freq[b];
        a = m_dad[a];
        if (a != ROOT) {
            if (m_leftc[m_dad[a]] == a)
                b = m_rightc[m_dad[a]];
            else
                b = m_leftc[m_dad[a]];
        }
    } while (a != ROOT);

    if (m_freq[ROOT] == MAXFREQ)
        for (a = 1; a <= TWICEMAX; a++)
            m_freq[a] >>= 1;
}

void Ca2mLoader::updatemodel(unsigned short code) {
    unsigned short a = code + SUCCMAX, b, c, code1, code2;

    m_freq[a]++;
    if (m_dad[a] != ROOT) {
        code1 = m_dad[a];
        if (m_leftc[code1] == a)
            updatefreq(a, m_rightc[code1]);
        else
            updatefreq(a, m_leftc[code1]);

        do {
            code2 = m_dad[code1];
            if (m_leftc[code2] == code1)
                b = m_rightc[code2];
            else
                b = m_leftc[code2];

            if (m_freq[a] > m_freq[b]) {
                if (m_leftc[code2] == code1)
                    m_rightc[code2] = a;
                else
                    m_leftc[code2] = a;

                if (m_leftc[code1] == a) {
                    m_leftc[code1] = b;
                    c = m_rightc[code1];
                } else {
                    m_rightc[code1] = b;
                    c = m_leftc[code1];
                }

                m_dad[b] = code1;
                m_dad[a] = code2;
                updatefreq(b, c);
                a = b;
            }

            a = m_dad[a];
            code1 = m_dad[a];
        } while (code1 != ROOT);
    }
}

unsigned short Ca2mLoader::inputcode(unsigned short bits) {
    unsigned short i, code = 0;

    for (i = 1; i <= bits; i++) {
        if (!m_bitcount) {
            if (m_bitcount == MAXBUF)
                m_bufcount = 0;
            m_bitbuffer = m_wdbuf[m_bufcount];
            m_bufcount++;
            m_bitcount = 15;
        } else
            m_bitcount--;

        if (m_bitbuffer > 0x7fff)
            code |= bitvalue[i - 1];
        m_bitbuffer <<= 1;
    }

    return code;
}

unsigned short Ca2mLoader::uncompress() {
    unsigned short a = 1;

    do {
        if (!m_bitcount) {
            if (m_bufcount == MAXBUF)
                m_bufcount = 0;
            m_bitbuffer = m_wdbuf[m_bufcount];
            m_bufcount++;
            m_bitcount = 15;
        } else
            m_bitcount--;

        if (m_bitbuffer > 0x7fff)
            a = m_rightc[a];
        else
            a = m_leftc[a];
        m_bitbuffer <<= 1;
    } while (a <= MAXCHAR);

    a -= SUCCMAX;
    updatemodel(a);
    return a;
}

void Ca2mLoader::decode() {
    unsigned short i, j, k, t, c, count = 0, dist, len, index;

    inittree();
    c = uncompress();

    while (c != TERMINATE) {
        if (c < 256) {
            m_obuf[m_obufcount] = static_cast<uint8_t>(c);
            m_obufcount++;
            if (m_obufcount == MAXBUF) {
                m_outputSize = MAXBUF;
                m_obufcount = 0;
            }

            m_buf[count] = static_cast<uint8_t>(c);
            count++;
            if (count == MAXSIZE)
                count = 0;
        } else {
            t = c - FIRSTCODE;
            index = t / CODESPERRANGE;
            len = t + MINCOPY - index * CODESPERRANGE;
            dist = inputcode(copybits[index]) + len + copymin[index];

            j = count;
            k = count - dist;
            if (count < dist)
                k += MAXSIZE;

            for (i = 0; i <= len - 1; i++) {
                m_obuf[m_obufcount] = m_buf[k];
                m_obufcount++;
                if (m_obufcount == MAXBUF) {
                    m_outputSize = MAXBUF;
                    m_obufcount = 0;
                }

                m_buf[j] = m_buf[k];
                j++;
                k++;
                if (j == MAXSIZE)
                    j = 0;
                if (k == MAXSIZE)
                    k = 0;
            }

            count += len;
            if (count >= MAXSIZE)
                count -= MAXSIZE;
        }
        c = uncompress();
    }
    m_outputSize = m_obufcount;
}

size_t Ca2mLoader::sixdepak(uint16_t *source, uint8_t *dest, size_t size) {
    if (size + 4096 > MAXBUF)
        return 0;

    m_buf.resize(MAXSIZE);
    m_inputSize = size;
    m_bitcount = 0;
    m_bitbuffer = 0;
    m_obufcount = 0;
    m_bufcount = 0;
    m_wdbuf = source;
    m_obuf = dest;

    decode();
    m_buf.clear();
    return m_outputSize;
}
