/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef XMPATTERN_H
#define XMPATTERN_H

#include "genmod/gencell.h"

namespace ppp
{
namespace xm
{
enum class Effect : uint8_t
{
    Arpeggio = 0,
    PortaUp = 1,
    PortaDown = 2,
    Porta = 3,
    Vibrato = 4,
    PortaVolSlide = 5,
    VibratoVolSlide = 6,
    Tremolo = 7,
    SetPanning = 8,
    Offset = 9,
    VolSlide = 0x0a,
    PosJump = 0x0b, //!< @todo Implement!
    SetVolume = 0x0c,
    PatBreak = 0x0d, //!< @todo Implement!
    Extended = 0x0e,
    SetTempoBpm = 0x0f,
    SetGlobalVol = 0x10,
    GlobalVolSlide = 0x11,
	KeyOff = 0x14,
    SetEnvPos = 0x15, //!< @todo Implement!
    PanSlide = 0x19,
    Retrigger = 0x1b,
    Tremor = 0x1d,
    ExtraFinePorta = 0x21,
    None = 0xff
};

class XmCell : public GenCell
{
public:
    typedef std::shared_ptr<XmCell> Ptr;
    typedef std::vector<Ptr> Vector;
private:
    uint8_t m_note; //!< @brief Note value
    uint8_t m_instr; //!< @brief Instrument value
    uint8_t m_volume; //!< @brief Volume value
    Effect m_effect; //!< @brief Effect
    uint8_t m_effectValue; //!< @brief Effect value
public:
    XmCell() throw();
    virtual ~XmCell() throw();
    virtual bool load(BinStream &str) throw(PppException);
    virtual void reset() throw();
    virtual std::string trackerString() const throw();
    uint8_t getNote() const throw();
    uint8_t getInstr() const throw();
    uint8_t getVolume() const throw();
    Effect getEffect() const throw();
    uint8_t getEffectValue() const throw();
};

class XmPattern
{
    DISABLE_COPY(XmPattern)
public:
    typedef std::shared_ptr<XmPattern> Ptr;
    typedef std::vector<Ptr> Vector;
private:
    std::vector<XmCell::Vector> m_tracks;
    XmCell::Ptr createCell(uint16_t trackIndex, uint16_t row) throw(PppException);
public:
    XmPattern() = delete;
    XmPattern(int16_t chans) throw(PppException);
    ~XmPattern() throw();
    bool load(BinStream &str) throw(PppException);
    XmCell::Ptr getCell(uint16_t trackIndex, uint16_t row) throw();
    std::size_t numRows() const;
    std::size_t numChannels() const;
};
} // namespace xm
} // namespace ppp

#endif // XMPATTERN_H
