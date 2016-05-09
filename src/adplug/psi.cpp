/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * [xad] PSI player, by Riven the Mage <riven@ok.ru>
 */

/*
    - discovery -

  file(s) : 4BIDDEN.COM, PGRID.EXE
     type : Forbidden Dreams BBStro
            Power Grid BBStro
     tune : by Friar Tuck [Shadow Faction/ICE]
   player : by Psi [Future Crew]
  comment : seems to me what 4bidden tune & player was ripped from pgrid

  file(s) : MYSTRUNE.COM
     type : Mystical Runes BBStro
     tune : by ?
   player : by Psi [Future Crew]
*/

#include "psi.h"

namespace {
constexpr uint8_t psi_adlib_registers[99] = {
    0x20, 0x23, 0x40, 0x43, 0x60, 0x63, 0x80, 0x83, 0xE0, 0xE3, 0xC0, 0x21, 0x24,
    0x41, 0x44, 0x61, 0x64, 0x81, 0x84, 0xE1, 0xE4, 0xC1, 0x22, 0x25, 0x42, 0x45,
    0x62, 0x65, 0x82, 0x85, 0xE2, 0xE5, 0xC2, 0x28, 0x2B, 0x48, 0x4B, 0x68, 0x6B,
    0x88, 0x8B, 0xE8, 0xEB, 0xC3, 0x29, 0x2C, 0x49, 0x4C, 0x69, 0x6C, 0x89, 0x8C,
    0xE9, 0xEC, 0xC4, 0x2A, 0x2D, 0x4A, 0x4D, 0x6A, 0x6D, 0x8A, 0x8D, 0xEA, 0xED,
    0xC5, 0x30, 0x33, 0x50, 0x53, 0x70, 0x73, 0x90, 0x93, 0xF0, 0xF3, 0xC6, 0x31,
    0x34, 0x51, 0x54, 0x71, 0x74, 0x91, 0x94, 0xF1, 0xF4, 0xC7, 0x32, 0x35, 0x52,
    0x55, 0x72, 0x75, 0x92, 0x95, 0xF2, 0xF5, 0xC8
};

constexpr uint16_t psi_notes[16] = {
    0x216B, 0x2181, 0x2198, 0x21B0, 0x21CA, 0x21E5, 0x2202, 0x2220, 0x2241,
    0x2263, 0x2287, 0x2364, 0x0000, 0x0000, 0x0000, 0x0000 // by riven
};
}

CPlayer *CxadpsiPlayer::factory() { return new CxadpsiPlayer(); }

void CxadpsiPlayer::xadplayer_rewind(int) {
    getOpl()->writeReg(0x01, 0x20);
    getOpl()->writeReg(0x08, 0x00);
    getOpl()->writeReg(0xBD, 0x00);

    // get header
    m_header.instr_ptr = (tune()[1] << 8) + tune()[0];
    m_header.seq_ptr = (tune()[3] << 8) + tune()[2];

    // define instruments
    m_psi.instr_table = &tune()[m_header.instr_ptr];

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 11; j++) {
            unsigned short inspos =
                    (m_psi.instr_table[i * 2 + 1] << 8) + m_psi.instr_table[i * 2];

            getOpl()->writeReg(psi_adlib_registers[i * 11 + j], tune()[inspos + j]);
        }

        getOpl()->writeReg(0xA0 + i, 0x00);
        getOpl()->writeReg(0xB0 + i, 0x00);

        m_psi.note_delay[i] = 1;
        m_psi.note_curdelay[i] = 1;
    }
    m_psi.looping.fill(false);

    // calculate sequence pointer
    m_psi.seq_table = &tune()[m_header.seq_ptr];
}

void CxadpsiPlayer::xadplayer_update() {
    for (int i = 0; i < 8; i++) {
        auto ptr = (m_psi.seq_table[(i << 1) * 2 + 1] << 8) + m_psi.seq_table[(i << 1) * 2];

        m_psi.note_curdelay[i]--;

        if (!m_psi.note_curdelay[i]) {
            getOpl()->writeReg(0xA0 + i, 0x00);
            getOpl()->writeReg(0xB0 + i, 0x00);

            auto event = tune()[ptr++];

            // end of sequence ?
            if (!event) {
                ptr = (m_psi.seq_table[(i << 1) * 2 + 3] << 8) +
                        m_psi.seq_table[(i << 1) * 2 + 2];

                event = tune()[ptr++];

                // set sequence loop flag
                m_psi.looping[i] = true;

                // module loop ?
                if( std::find(m_psi.looping.begin(), m_psi.looping.end(), false) == m_psi.looping.end() )
                    setXadLooping();
            }

            // new note delay ?
            if (event & 0x80) {
                m_psi.note_delay[i] = (event & 0x7F);

                event = tune()[ptr++];
            }

            m_psi.note_curdelay[i] = m_psi.note_delay[i];

            // play note
            unsigned short note = psi_notes[event & 0x0F];

            getOpl()->writeReg(0xA0 + i, note & 0xFF);
            getOpl()->writeReg(0xB0 + i, (note >> 8) + ((event >> 2) & 0xFC));

            // save position
            const_cast<uint8_t*>(m_psi.seq_table)[(i << 1) * 2] = ptr & 0xff;
            const_cast<uint8_t*>(m_psi.seq_table)[(i << 1) * 2 + 1] = ptr >> 8;
        }
    }
}

size_t CxadpsiPlayer::framesUntilUpdate() const { return SampleRate / 70.0f; }

std::string CxadpsiPlayer::type() const {
    return "xad: psi player";
}

uint32_t CxadpsiPlayer::instrumentCount() const { return 8; }
