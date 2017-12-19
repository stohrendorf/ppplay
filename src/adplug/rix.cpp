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
 * rix.cpp - Softstar RIX OPL Format Player by palxex <palxex.ys168.com>
 *                                             BSPAL <BSPAL.ys168.com>
 */

#include "stream/filestream.h"

#include "rix.h"

namespace
{
constexpr uint8_t adflag[] = {0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1};
constexpr uint8_t reg_data[] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21};
constexpr uint8_t ad_C0_offs[] = {0, 1, 2, 0, 1, 2, 3, 4, 5, 3, 4, 5, 6, 7, 8, 6, 7, 8};
constexpr uint8_t modify[] = {0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 13, 16, 14, 17, 12, 15, 16, 0, 14, 0, 17, 0, 13, 0};
constexpr uint8_t bd_reg_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x01,
    0x01, 0x03, 0x0F, 0x05, 0x00, 0x01, 0x03, 0x0F, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x0F, 0x07, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0A, 0x04, 0x00, 0x08, 0x0C, 0x0B, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x04, 0x00, 0x06, 0x0F, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x0C, 0x00, 0x0F, 0x0B, 0x00, 0x08, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0F, 0x0B, 0x00, 0x07, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x0B, 0x00, 0x05, 0x05,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x0B, 0x00, 0x07,
    0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
constexpr uint16_t mus_time = 0x4268;
}

/*** public methods *************************************/

Player* RixPlayer::factory()
{
    return new RixPlayer();
}

RixPlayer::RixPlayer()
    : Player()
{
    m_for40reg.fill(0x7f);
}

bool RixPlayer::load(const std::string& filename)
{
    FileStream f(filename);
    if( !f )
    {
        return false;
    }

    if( f.extension() == ".mkf" )
    {
        m_flagMkf = true;
        f.seek(0);
        uint32_t offset;
        f >> offset;
        f.seek(offset);
    }
    uint16_t tmp;
    f >> tmp;
    if( tmp != 0x55aa )
    {
        return false;
    }
    m_fileBuffer.resize(f.size() + 1);
    f.seek(0);
    f.read(m_fileBuffer.data(), f.size());
    m_length = f.size();
    if( !m_flagMkf )
    {
        m_bufAddr = m_fileBuffer.data();
    }
    rewind(size_t(0));
    return true;
}

bool RixPlayer::update()
{
    int_08h_entry();
    return !m_playEnd;
}

void RixPlayer::rewind(const boost::optional<size_t>& subsong)
{
    m_i = 0;
    m_musBlock = 0;
    m_insBlock = 0;
    m_rhythm = 0;
    m_musicOn = false;
    m_pauseFlag = false;
    m_band = 0;
    m_bandLow = 0;
    m_e0RegFlag = 0;
    m_bdModify = 0;
    m_sustain = 0;
    m_playEnd = false;

    m_fBuffer.fill(0);
    m_a0b0Data2.fill(0);
    m_a0b0Data3.fill(0);
    m_a0b0Data4.fill(0);
    m_a0b0Data5.fill(0);
    m_addrsHead.fill(0);
    m_insBuf.fill(0);
    m_displace.fill(0);
    m_regBufs.fill({});

    if( m_flagMkf )
    {
        const auto* buf_index = reinterpret_cast<const uint32_t*>(m_fileBuffer.data());
        auto idx = *subsong;
        int offset1 = buf_index[idx], offset2;
        while( (offset2 = buf_index[++idx]) == offset1 )
        {
        }
        m_length = offset2 - offset1 + 1;
        m_bufAddr = m_fileBuffer.data() + offset1;
    }
    getOpl()->writeReg(1, 32); // go to OPL2 mode
    set_new_int();
    data_initial();
}

size_t RixPlayer::subSongCount() const
{
    if( m_flagMkf )
    {
        const auto* buf_index = reinterpret_cast<const uint32_t*>(m_fileBuffer.data());
        uint32_t songs = buf_index[0] / 4;
        for( uint32_t i = 0; i < songs; i++ )
        {
            if( buf_index[i + 1] == buf_index[i] )
            {
                songs--;
            }
        }
        return songs;
    }
    else
    {
        return 1;
    }
}

size_t RixPlayer::framesUntilUpdate() const
{
    return SampleRate / 70;
}

/*------------------Implemention----------------------------*/
inline void RixPlayer::set_new_int()
{
    //   if(!ad_initial()) exit(1);
    ad_initial();
}

/*----------------------------------------------------------*/
inline void RixPlayer::ad_a0b0l_reg_(uint16_t index, uint16_t p2, uint16_t p3)
{
    //   uint16_t i = p2+a0b0_data2[index];
    m_a0b0Data4[index] = p3;
    m_a0b0Data3[index] = p2;
}

inline void RixPlayer::data_initial()
{
    m_rhythm = m_bufAddr[2];
    m_musBlock = (m_bufAddr[0x0D] << 8) + m_bufAddr[0x0C];
    m_insBlock = (m_bufAddr[0x09] << 8) + m_bufAddr[0x08];
    m_i = m_musBlock + 1;
    if( m_rhythm != 0 )
    {
        //		ad_a0b0_reg(6);
        //		ad_a0b0_reg(7);
        //		ad_a0b0_reg(8);
        ad_a0b0l_reg_(8, 0x18, 0);
        ad_a0b0l_reg_(7, 0x1F, 0);
    }
    m_bdModify = 0;
    //	ad_bd_reg();
    m_band = 0;
    m_musicOn = true;
}

/*----------------------------------------------------------*/
inline uint16_t RixPlayer::ad_initial()
{
    for( int i = 0; i < 25; i++ )
    {
        m_fBuffer[i * 12] = uint32_t((i * 24 + 10000) * 0.27461678223 + 4) >> 3;
        for( int t = 1; t < 12; t++ )
        {
            m_fBuffer[i * 12 + t] = m_fBuffer[i * 12 + t - 1] * 1.06;
        }
    }
    int k = 0;
    for( int i = 0; i < 8; i++ )
    {
        for( int j = 0; j < 12; j++ )
        {
            m_a0b0Data5[k] = i;
            m_addrsHead[k] = j;
            k++;
        }
    }
    //ad_bd_reg();
    //ad_08_reg();
    //for(i=0;i<9;i++) ad_a0b0_reg(i);
    m_e0RegFlag = 0x20;
    //for(i=0;i<18;i++) writeRegister(0xE0+reg_data[i],0);
    //writeRegister(1,e0_reg_flag);
    return 1; //ad_test();
}

/*----------------------------------------------------------*/
inline void RixPlayer::writeRegister(uint16_t reg, uint16_t value)
{
    getOpl()->writeReg(reg & 0xff, value & 0xff);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::int_08h_entry()
{
    uint16_t band_sus = 1;
    while( band_sus != 0 )
    {
        if( m_sustain <= 0 )
        {
            band_sus = rix_proc();
            if( band_sus )
            {
                m_sustain += band_sus;
            }
            else
            {
                m_playEnd = true;
                break;
            }
        }
        else
        {
            if( band_sus != 0 )
            {
                m_sustain -= 14;
            } /* aging */
            break;
        }
    }
}

/*--------------------------------------------------------------*/
inline uint16_t RixPlayer::rix_proc()
{
    if( !m_musicOn || m_pauseFlag )
    {
        return 0;
    }
    m_band = 0;
    while( m_i < m_length - 1 && m_bufAddr[m_i] != 0x80 )
    {
        m_bandLow = m_bufAddr[m_i - 1];
        const auto ctrl = m_bufAddr[m_i];
        m_i += 2;
        switch( ctrl & 0xF0 )
        {
            case 0x90:
                rix_get_ins();
                rix_90_pro(ctrl & 0x0F);
                break;
            case 0xA0:
                rix_A0_pro(ctrl & 0x0F, uint16_t(m_bandLow) << 6);
                break;
            case 0xB0:
                rix_B0_pro(ctrl & 0x0F, m_bandLow);
                break;
            case 0xC0:
                switch_ad_bd(ctrl & 0x0F);
                if( m_bandLow != 0 )
                {
                    rix_C0_pro(ctrl & 0x0F, m_bandLow);
                }
                break;
            default:
                m_band = (ctrl << 8) + m_bandLow;
                break;
        }
        if( m_band != 0 )
        {
            return m_band;
        }
    }
    music_ctrl();
    m_i = m_musBlock + 1;
    m_band = 0;
    m_musicOn = true;
    return 0;
}

/*--------------------------------------------------------------*/
inline void RixPlayer::rix_get_ins()
{
    const uint8_t* baddr = &m_bufAddr[m_insBlock] + (m_bandLow << 6);

    for( int i = 0; i < 28; i++ )
    {
        m_insBuf[i] = (baddr[i * 2 + 1] << 8) + baddr[i * 2];
    }
}

/*--------------------------------------------------------------*/
inline void RixPlayer::rix_90_pro(uint16_t ctrl_l)
{
    if( m_rhythm == 0 || ctrl_l < 6 )
    {
        ins_to_reg(modify[ctrl_l * 2], m_insBuf.data(), m_insBuf[26]);
        ins_to_reg(modify[ctrl_l * 2 + 1], m_insBuf.data() + 13, m_insBuf[27]);
    }
    else if( ctrl_l > 6 )
    {
        ins_to_reg(modify[ctrl_l * 2 + 6], m_insBuf.data(), m_insBuf[26]);
    }
    else
    {
        ins_to_reg(12, m_insBuf.data(), m_insBuf[26]);
        ins_to_reg(15, m_insBuf.data() + 13, m_insBuf[27]);
    }
}

/*--------------------------------------------------------------*/
inline void RixPlayer::rix_A0_pro(uint16_t ctrl_l, uint16_t index)
{
    if( m_rhythm != 0 && ctrl_l > 6 )
    {
        return;
    }

    prepare_a0b0(ctrl_l, index > 0x3FFF ? 0x3FFF : index);
    ad_a0b0l_reg(ctrl_l, m_a0b0Data3[ctrl_l], m_a0b0Data4[ctrl_l]);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::prepare_a0b0(uint16_t index, uint16_t v) /* important !*/
{
    const int res1 = (v - 0x2000) * 0x19;
    if( res1 == 0xff )
    {
        return;
    }
    int16_t low = res1 / 0x2000;
    if( low < 0 )
    {
        low = 0x18 - low;
        int16_t high = low < 0 ? -1 : 0;
        int res = high;
        res <<= 16;
        res += low;
        low = res / 0xFFE7;
        m_a0b0Data2[index] = low;
        low = res;
        res = low - 0x18;
        high = res % 0x19;
        low = res / 0x19;
        if( high != 0 )
        {
            low = 0x19;
            low = low - high;
        }
    }
    else
    {
        uint32_t res = low;
        uint16_t high = low;
        low = res / 0x19;
        m_a0b0Data2[index] = low;
        res = high;
        low = res % 0x19;
    }
    low = low * 0x18;
    m_displace[index] = low;
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_a0b0l_reg(uint16_t index, uint16_t p2, uint16_t p3)
{
    uint16_t i = p2 + m_a0b0Data2[index];
    m_a0b0Data4[index] = p3;
    m_a0b0Data3[index] = p2;
    int16_t i2 = static_cast<int16_t>(i);
    i2 = (i2 <= 0x5F ? i2 : 0x5F);
    i2 = (i2 >= 0 ? i2 : 0);
    uint16_t data = m_fBuffer[m_addrsHead[i2] + m_displace[index] / 2];
    writeRegister(0xA0 + index, data);
    data = m_a0b0Data5[i2] * 4 + (p3 < 1 ? 0 : 0x20) + ((data >> 8) & 3);
    writeRegister(0xB0 + index, data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::rix_B0_pro(uint16_t ctrl_l, uint16_t index)
{
    int temp;
    if( m_rhythm == 0 || ctrl_l < 6 )
    {
        temp = modify[ctrl_l * 2 + 1];
    }
    else
    {
        temp = ctrl_l > 6 ? ctrl_l * 2 : ctrl_l * 2 + 1;
        temp = modify[temp + 6];
    }
    m_for40reg[temp] = index > 0x7F ? 0x7F : index;
    ad_40_reg(temp);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::rix_C0_pro(uint16_t ctrl_l,
                                  uint16_t index)
{
    uint16_t i = index >= 12 ? index - 12 : 0;
    if( ctrl_l < 6 || m_rhythm == 0 )
    {
        ad_a0b0l_reg(ctrl_l, i, 1);
        return;
    }

    if( ctrl_l != 6 )
    {
        if( ctrl_l == 8 )
        {
            ad_a0b0l_reg(ctrl_l, i, 0);
            ad_a0b0l_reg(7, i + 7, 0);
        }
    }
    else
    {
        ad_a0b0l_reg(ctrl_l, i, 0);
    }
    m_bdModify |= bd_reg_data[ctrl_l];
    ad_bd_reg();
}

/*--------------------------------------------------------------*/
inline void RixPlayer::switch_ad_bd(uint16_t index)
{
    if( m_rhythm == 0 || index < 6 )
    {
        ad_a0b0l_reg(index, m_a0b0Data3[index], 0);
    }
    else
    {
        m_bdModify &= (~bd_reg_data[index]), ad_bd_reg();
    }
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ins_to_reg(uint16_t index, const uint16_t* insb, uint16_t value)
{
    for( int i = 0; i < 13; i++ )
    {
        m_regBufs[index].v[i] = insb[i];
    }
    m_regBufs[index].v[13] = value & 3;
    ad_bd_reg();
    getOpl()->writeReg(8, 0);
    ad_40_reg(index);
    ad_C0_reg(index);
    ad_60_reg(index);
    ad_80_reg(index);
    ad_20_reg(index);
    ad_E0_reg(index);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_E0_reg(uint16_t index)
{
    const uint16_t data = m_e0RegFlag == 0 ? 0 : (m_regBufs[index].v[13] & 3);
    writeRegister(0xE0 + reg_data[index], data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_20_reg(uint16_t index)
{
    uint16_t data = (m_regBufs[index].v[9] < 1 ? 0 : 0x80);
    data += (m_regBufs[index].v[10] < 1 ? 0 : 0x40);
    data += (m_regBufs[index].v[5] < 1 ? 0 : 0x20);
    data += (m_regBufs[index].v[11] < 1 ? 0 : 0x10);
    data += (m_regBufs[index].v[1] & 0x0F);
    writeRegister(0x20 + reg_data[index], data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_80_reg(uint16_t index)
{
    uint16_t data = (m_regBufs[index].v[7] & 0x0F),
        temp = m_regBufs[index].v[4];
    data |= (temp << 4);
    writeRegister(0x80 + reg_data[index], data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_60_reg(uint16_t index)
{
    uint16_t data = m_regBufs[index].v[6] & 0x0F,
        temp = m_regBufs[index].v[3];
    data |= (temp << 4);
    writeRegister(0x60 + reg_data[index], data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_C0_reg(uint16_t index)
{
    uint16_t data = m_regBufs[index].v[2];
    if( adflag[index] == 1 )
    {
        return;
    }
    data *= 2;
    data |= (m_regBufs[index].v[12] < 1 ? 1 : 0);
    writeRegister(0xC0 + ad_C0_offs[index], data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_40_reg(uint16_t index)
{
    uint16_t temp = m_regBufs[index].v[0];
    uint16_t data = 0x3F - (0x3F & m_regBufs[index].v[8]);
    data *= m_for40reg[index];
    data *= 2;
    data += 0x7F;
    auto res = data;
    data = res / 0xFE;
    data -= 0x3F;
    data = -data;
    data |= temp << 6;
    writeRegister(0x40 + reg_data[index], data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::ad_bd_reg()
{
    uint16_t data = m_rhythm < 1 ? 0 : 0x20;
    data |= m_bdModify;
    writeRegister(0xBD, data);
}

/*--------------------------------------------------------------*/
inline void RixPlayer::music_ctrl()
{
    for( int i = 0; i < 11; i++ )
    {
        switch_ad_bd(i);
    }
}