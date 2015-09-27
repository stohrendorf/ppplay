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
 * rix.h - Softstar RIX OPL Format Player by palxex <palxex.ys168.com>
 *                                           BSPAL <BSPAL.ys168.com>
 */

#include "player.h"

class CrixPlayer : public CPlayer {
    DISABLE_COPY(CrixPlayer)
public:
    static CPlayer *factory();

    CrixPlayer();
    ~CrixPlayer() = default;

    bool load(const std::string &filename);
    bool update();
    void rewind(int subsong);
    size_t framesUntilUpdate() const;
    uint32_t subSongCount() const;

    std::string type() const
    {
        return "Softstar RIX OPL Music Format";
    }

private:
    struct ADDT {
        ADDT()
        {
            v.fill(0);
        }

        std::array<uint8_t,14> v{{}};
    };

    bool m_flagMkf = false;
    std::vector<uint8_t> m_fileBuffer{};
    const uint8_t* m_bufAddr = nullptr;      /* rix files' f_buffer */
    std::array<uint16_t,300> m_fBuffer{{}}; //9C0h-C18h
    std::array<uint16_t,11> m_a0b0Data2{{}};
    std::array<uint8_t,18> m_a0b0Data3{{}};
    std::array<uint8_t,18> m_a0b0Data4{{}};
    std::array<uint8_t,96> m_a0b0Data5{{}};
    std::array<uint8_t,96> m_addrsHead{{}};
    std::array<uint16_t,28> m_insBuf{{}};
    std::array<uint16_t,11> m_displace{{}};
    std::array<ADDT,18> m_regBufs{{}};
    size_t m_pos = 0;
    size_t m_length = 0;
    uint8_t m_index = 0;

    uint32_t m_i = 0;
    uint32_t m_t = 0;
    uint16_t m_musBlock = 0;
    uint16_t m_insBlock = 0;
    uint8_t m_rhythm = 0;
    bool m_musicOn = false;
    uint8_t m_pauseFlag = 0;
    uint16_t m_band = 0;
    uint8_t m_bandLow = 0;
    uint16_t m_e0RegFlag = 0;
    uint8_t m_bdModify = 0;
    int m_sustain = 0;
    bool m_playEnd = false;

    std::array<uint8_t,18> m_for40reg{{}};

#define ad_08_reg() ad_bop(8, 0)                                            /**/
    inline void ad_20_reg(uint16_t);                                    /**/
    inline void ad_40_reg(uint16_t);                                    /**/
    inline void ad_60_reg(uint16_t);                                    /**/
    inline void ad_80_reg(uint16_t);                                    /**/
    inline void ad_a0b0_reg(uint16_t);                                  /**/
    inline void ad_a0b0l_reg(uint16_t, uint16_t, uint16_t); /**/
    inline void ad_a0b0l_reg_(uint16_t, uint16_t, uint16_t);
    /**/
    inline void ad_bd_reg();                               /**/
    inline void ad_bop(uint16_t, uint16_t);    /**/
    inline void ad_C0_reg(uint16_t);                 /**/
    inline void ad_E0_reg(uint16_t);                 /**/
    inline uint16_t ad_initial();                    /**/
    inline uint16_t ad_test();                       /**/
    inline void crc_trans(uint16_t, uint16_t); /**/
    inline void data_initial();                            /* done */
    inline void init();                                    /**/
    inline void ins_to_reg(uint16_t, const uint16_t *, uint16_t); /**/
    inline void int_08h_entry();                                              /**/
    inline void music_ctrl();                                                 /**/
    inline void Pause();                                                      /**/
    inline void prepare_a0b0(uint16_t, uint16_t);                 /**/
    inline void rix_90_pro(uint16_t);                                   /**/
    inline void rix_A0_pro(uint16_t, uint16_t);                   /**/
    inline void rix_B0_pro(uint16_t, uint16_t);                   /**/
    inline void rix_C0_pro(uint16_t, uint16_t);                   /**/
    inline void rix_get_ins();                                                /**/
    inline uint16_t rix_proc();                                         /**/
    inline void set_new_int();
    inline void switch_ad_bd(uint16_t); /**/
};
