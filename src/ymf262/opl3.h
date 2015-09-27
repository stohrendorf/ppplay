/*
 * PPPlay - an old-fashioned module player
 * Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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
 * Original Java Code: Copyright (C) 2008 Robson Cozendey <robson@cozendey.com>
 *
 * Some code based on forum posts in: http://forums.submarine.org.uk/phpBB/viewforum.php?f=9,
 * Copyright (C) 2010-2013 by carbon14 and opl3
 */

#ifndef PPP_OPL_OPL3_H
#define PPP_OPL_OPL3_H

#include <cstdint>
#include <array>

#include "operator.h"
#include "channel.h"
#include "oplfilter.h"
#include <stream/iserializable.h>

#include <ymf262/ppplay_opl_export.h>

namespace opl
{

class OperatorView;
class SlotView;

class PPPLAY_OPL_EXPORT Opl3 : public ISerializable
{
    friend class OperatorView;
    friend class SlotView;
public:
    static constexpr int MasterClock = 14.31818e6;
    static constexpr int SampleRate = MasterClock / 288;

private:
    uint8_t m_registers[0x200];

    // The YMF262 has 36 operators:
    /**
     * @brief The operators.
     *
     * @details
     * Operators 1 to 18 are in m_operators[0], and
     * operators 19 to 36 are in m_operators[1]. Within each
     * sub-array, the 18 operators are subdivided into groups
     * of 6 operators, starting at offsets 0, 8, and 16.
     */
    Operator::Ptr m_operators[2][0x16];
    /**
     * @brief The 2-operator channels
     *
     * @details
     * Channels 1 to 9 are in m_channels2op[0], and
     * channels 10 to 18 are in m_channels2op[1].
     */
    Channel::Ptr m_channels2op[2][9];
    /**
     * @brief The 4-operator channels
     *
     * @details
     * Channels 1 to 3 are in m_channels4op[0], and
     * channels 4 to 6 are in m_channels4op[1].
     */
    Channel::Ptr m_channels4op[2][3];
    Channel::Ptr m_channels[2][9];
    Channel::Ptr m_disabledChannel;
    MultiplexFilter<4,HighPassFilter<SampleRate,2,5>> m_filters{};

    bool m_nts = false;
    //! @brief Depth of amplitude
    bool m_dam = false;
    //! @brief Depth of vibrato
    bool m_dvb = false;
    bool m_ryt = false;
    bool m_bd=false, m_sd=false, m_tc=false, m_hh=false;
    //! @brief OPL2/OPL3 mode selection
    bool m_new = false;
    //! @brief 13 bits
    uint16_t m_vibratoIndex = 0;
    //! @brief 14 bits, wraps around after 13*1024
    uint16_t m_tremoloIndex = 0;
    //! @brief Random number generator
    uint32_t m_rand = 1;

public:
    uint32_t randBit() const {
        return m_rand & 1;
    }
    bool isNew() const {
        return m_new;
    }
    uint8_t readReg( uint16_t index ) const {
        BOOST_ASSERT( index < 0x200 );
        return m_registers[index];
    }
    void writeReg( uint16_t index, uint8_t val ) {
        write( ( index >> 8 ) & 1, index & 0xff, val );
    }
    bool nts() const {
        return m_nts;
    }
    Operator* bassDrumOp1() const {
        return m_operators[0][0x10].get();
    }
    Operator* highHatOperator() const {
        return m_operators[0][0x11].get();
    }
    Operator* tomTomOperator() const {
        return m_operators[0][0x12].get();
    }
    Operator* bassDrumOp2() const {
        return m_operators[0][0x13].get();
    }
    Operator* topCymbalOperator() const {
        return m_operators[0][0x14].get();
    }
    Operator* snareDrumOperator() const {
        return m_operators[0][0x15].get();
    }
    bool dvb() const {
        return m_dvb;
    }
    uint16_t vibratoIndex() const {
        return m_vibratoIndex;
    }
    bool dam() const {
        return m_dam;
    }
    bool ryt() const {
        return m_ryt;
    }
    uint16_t tremoloIndex() const {
        return m_tremoloIndex;
    }

    void read( std::array< int16_t, 4 >* dest ) ;

    Opl3();

    AbstractArchive& serialize( AbstractArchive* archive );

    OperatorView getOperatorView(size_t index, bool second);
    SlotView getSlotView(size_t index);
private:
    void update_DAM1_DVB1_RYT1_BD1_SD1_TOM1_TC1_HH1();
    void setEnabledChannels();
    void set4opConnections();
    void write( int array, int address, uint8_t data );
    void replaceRegBits(int address, uint8_t ofs, uint8_t count, uint8_t value) {
        BOOST_ASSERT(value < (1<<count));
        BOOST_ASSERT(ofs+count <= 8);
        const uint8_t mask = ((1<<count)-1)<<ofs;
        uint8_t old = readReg(address) & ~mask;
        value &= ((1<<count)-1);
        writeReg(address, old | (value<<ofs));
    }
};


class PPPLAY_OPL_EXPORT OperatorView
{
private:
    friend class Opl3;

    Opl3* m_opl;
    size_t m_baseRegister;
    OperatorView(Opl3* opl, size_t baseRegister) : m_opl(opl), m_baseRegister(baseRegister)
    {
        BOOST_ASSERT( (baseRegister&0xff)<=21 );
        BOOST_ASSERT( (baseRegister>>8)<=1 );
        BOOST_ASSERT( opl != nullptr );
    }

public:
    void setTotalLevel(uint8_t level) {
        m_opl->replaceRegBits(m_baseRegister+0x40, 0, 6, level);
    }
    void setKsl(uint8_t level) {
        m_opl->replaceRegBits(m_baseRegister+0x40, 6, 2, level);
    }
    void setAttackRate(uint8_t rate) {
        m_opl->replaceRegBits(m_baseRegister+0x60, 4, 4, rate);
    }
    void setDecayRate(uint8_t rate) {
        m_opl->replaceRegBits(m_baseRegister+0x60, 0, 4, rate);
    }
    void setSustainLevel(uint8_t level) {
        m_opl->replaceRegBits(m_baseRegister+0x80, 4, 4, level);
    }
    void setReleaseRate(uint8_t rate) {
        m_opl->replaceRegBits(m_baseRegister+0x80, 0, 4, rate);
    }
    void setWave(uint8_t wave) {
        m_opl->replaceRegBits(m_baseRegister+0xe0, 0, 3, wave);
    }
    void setAm(bool value) {
        m_opl->replaceRegBits(m_baseRegister+0x20, 7, 1, value?1:0);
    }
    void setVib(bool value) {
        m_opl->replaceRegBits(m_baseRegister+0x20, 6, 1, value?1:0);
    }
    void setEgt(bool value) {
        m_opl->replaceRegBits(m_baseRegister+0x20, 5, 1, value?1:0);
    }
    void setKsr(bool value) {
        m_opl->replaceRegBits(m_baseRegister+0x20, 4, 1, value?1:0);
    }
    void setMult(uint8_t mult) {
        m_opl->replaceRegBits(m_baseRegister+0x20, 0, 4, mult);
    }
};

class PPPLAY_OPL_EXPORT SlotView
{
    friend class Opl3;

    Opl3* m_opl;
    size_t m_baseRegister;
    OperatorView m_modulator;
    OperatorView m_carrier;

    SlotView(Opl3* opl, size_t baseRegister, OperatorView&& modulator, OperatorView&& carrier)
        : m_opl(opl)
        , m_baseRegister(baseRegister)
        , m_modulator(std::move(modulator))
        , m_carrier(std::move(carrier))
    {
        BOOST_ASSERT( (baseRegister&0xff)<=9 );
        BOOST_ASSERT( (baseRegister>>8)<=1 );
        BOOST_ASSERT( opl != nullptr );
    }

public:
    void setFnum(uint16_t fnum) {
        m_opl->replaceRegBits(m_baseRegister+0xa0, 0, 8, fnum);
        m_opl->replaceRegBits(m_baseRegister+0xb0, 0, 2, fnum>>8);
    }
    void setBlock(uint8_t block) {
        m_opl->replaceRegBits(m_baseRegister+0xb0, 2, 3, block);
    }
    void setKeyOn(bool value) {
        m_opl->replaceRegBits(m_baseRegister+0xb0, 5, 1, value?1:0);
    }
    void setFeedback(uint8_t fb) {
        m_opl->replaceRegBits(m_baseRegister+0xc0, 1, 3, fb);
    }
    void setCnt(bool value) {
        m_opl->replaceRegBits(m_baseRegister+0xc0, 0, 1, value?1:0);
    }
    void setOutput(bool cha, bool chb, bool chc, bool chd) {
        m_opl->replaceRegBits(m_baseRegister+0xc0, 7, 1, cha?1:0);
        m_opl->replaceRegBits(m_baseRegister+0xc0, 6, 1, chb?1:0);
        m_opl->replaceRegBits(m_baseRegister+0xc0, 5, 1, chc?1:0);
        m_opl->replaceRegBits(m_baseRegister+0xc0, 4, 1, chd?1:0);
    }

    OperatorView& modulator() noexcept {
        return m_modulator;
    }
    OperatorView& carrier() noexcept {
        return m_carrier;
    }
};

inline OperatorView Opl3::getOperatorView(size_t index, bool second)
{
    BOOST_ASSERT( index<18 );
    static constexpr std::array<uint8_t,18> slotRegisterOffsets = {{
        0,  1,  2,  3,  4,  5,
        8,  9, 10, 11, 12, 13,
        16, 17, 18, 19, 20, 21
    }};
    return OperatorView(this, slotRegisterOffsets.at(index) | (second?0x100:0));
}

inline SlotView Opl3::getSlotView(size_t index)
{
    BOOST_ASSERT( index<18 );
    static constexpr std::array<std::array<uint8_t,2>,9> voiceSlotOffsets = {{
        {{ 0, 3 }}, {{ 1, 4 }}, {{ 2, 5 }}, {{ 6, 9 }}, {{ 7, 10 }}, {{ 8, 11 }}, {{ 12, 15 }}, {{ 13, 16 }}, {{ 14, 17 }}
    }};
    return SlotView(this,
                    (index%9) | ((index/9)<<8),
                    getOperatorView(voiceSlotOffsets.at(index%9)[0], index>=9),
                    getOperatorView(voiceSlotOffsets.at(index%9)[1], index>=9));
}

}

#endif
