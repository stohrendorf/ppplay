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
    FxArpeggio = 0,
    FxPortaUp = 1,
    FxPortaDown = 2,
    FxPorta = 3,
    FxVibrato = 4,
    FxPortaVolSlide = 5,
    FxVibratoVolSlide = 6,
    FxTremolo = 7, //!< @todo Implement!
    FxSetPanning = 8,
    FxOffset = 9,
    FxVolSlide = 0x0a,
    FxPosJump = 0x0b, //!< @todo Implement!
    FxSetVolume = 0x0c,
    FxPatBreak = 0x0d, //!< @todo Implement!
    FxExtended = 0x0e,
    FxSetTempoBpm = 0x0f,
    FxSetGlobalVol = 0x10,
    FxGlobalVolSlide = 0x11,
    FxSetEnvPos = 0x15, //!< @todo Implement!
    FxPanSlide = 0x19,
    FxRetrigger = 0x1b, //!< @todo Implement!
    FxTremor = 0x1d, //!< @todo Implement!
    FxExtraFinePorta = 0x21,
    FxNone = 0xff
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
