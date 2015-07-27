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
 * player.h - Replayer base class, by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_PLAYER
#define H_ADPLUG_PLAYER

#include <string>

#include "ymf262/opl3.h"
#include "stuff/utils.h"

class CPlayer {
    DISABLE_COPY(CPlayer)
    public:
        static constexpr auto SampleRate = opl::Opl3::SampleRate;
    CPlayer();
    virtual ~CPlayer() = default;

    /***** Operational methods *****/
    virtual bool load(const std::string &filename) = 0;
    virtual bool update() = 0;                 // executes replay code for 1 tick
    virtual void rewind(int subsong = -1) = 0; // rewinds to specified subsong
    virtual size_t framesUntilUpdate() const = 0;

    /***** Informational methods *****/
    virtual std::string type() const = 0; // returns file type
    virtual std::string title() const     // returns song title
    {
        return std::string();
    }
    virtual std::string author() const // returns song author name
    {
        return std::string();
    }
    virtual std::string description() const // returns song description
    {
        return std::string();
    }
    uint8_t currentRow() const // returns currently playing row
    {
        return m_currentRow;
    }
    uint16_t initialSpeed() const // returns current song speed
    {
        return m_initialSpeed;
    }
    uint16_t currentSpeed() const // returns current song speed
    {
        return m_currentSpeed;
    }
    uint16_t currentTempo() const // returns current song speed
    {
        return m_currentTempo;
    }
    uint16_t initialTempo() const // returns current song speed
    {
        return m_initialTempo;
    }
    virtual uint32_t subSongCount() const // returns number of subsongs
    {
        return 1;
    }
    virtual uint32_t currentSubSong() const // returns current subsong
    {
        return 0;
    }
    virtual uint32_t instrumentCount() const // returns number of instruments
    {
        return 0;
    }
    virtual std::string instrumentTitle(size_t) const // returns n-th instrument name
    {
        return std::string();
    }

    uint32_t orderCount() const
    {
        return m_order.size();
    }

    size_t currentOrder() const
    {
        return m_currentOrder;
    }

    uint8_t currentPattern() const
    {
        BOOST_ASSERT( m_currentOrder < m_order.size() );
        return m_order[m_currentOrder];
    }

    opl::Opl3 *getOpl() { return &m_oplChip; }
    virtual void read(std::array<int16_t, 4> *data) { m_oplChip.read(data); }

private:
    opl::Opl3 m_oplChip;
    std::vector<uint8_t> m_order{};
    size_t m_currentOrder = 0;
    uint8_t m_currentRow = 0;
    uint16_t m_initialSpeed = 6;
    uint16_t m_currentSpeed = 6;
    uint16_t m_initialTempo = 0;
    uint16_t m_currentTempo = 0;

protected:
    void addOrder(uint8_t order)
    {
        m_order.emplace_back(order);
    }

    void setCurrentOrder(size_t idx) {
        //BOOST_ASSERT( idx < m_order.size() );
        m_currentOrder = idx;
    }

    void setCurrentRow(uint8_t idx) {
        m_currentRow = idx;
    }

    void setInitialSpeed(uint16_t spd) {
        m_initialSpeed = spd;
    }

    void setCurrentSpeed(uint16_t spd) {
        m_currentSpeed = spd;
    }

    void setInitialTempo(uint16_t tempo) {
        m_initialTempo = tempo;
    }

    void setCurrentTempo(uint16_t tempo) {
        m_currentTempo = tempo;
    }

    static const uint16_t s_noteTable[12]; // standard adlib note table
    static const uint8_t s_opTable[9]; // the 9 operators as expected by the OPL
};

#endif
