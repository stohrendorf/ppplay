/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef XMMODULE_H
#define XMMODULE_H

/**
 * @ingroup XmModule
 * @{
 */

#include "genmod/abstractmodule.h"

#include <array>

namespace ppp
{
namespace xm
{
class XmChannel;
class XmPattern;
class XmInstrument;
/**
 * @class XmModule
 * @brief XM module class
 */
class XmModule : public AbstractModule
{
    DISABLE_COPY(XmModule)
        friend class XmChannel;
private:
    //! @brief @c true if amiga period table is used
    bool m_amiga;
    //! @brief Module patterns
    std::vector<XmPattern*> m_patterns;
    //! @brief Module instruments
    std::vector<XmInstrument*> m_instruments;
    //! @brief Module channels
    std::vector<XmChannel*> m_channels;
    //! @brief Maps notes including finetune to their periods
    std::array<uint16_t, 121 * 16> m_noteToPeriod;
    //! @brief Contains the row to break to
    size_t m_jumpRow;
    //! @brief Contains the order to jump to
    size_t m_jumpOrder;
    //! @brief Is @c true if a pattern loop is running
    bool m_isPatLoop;
    //! @brief Is @c true if a pattern jump is intended
    bool m_doPatJump;
    //! @brief Song restart position
    uint8_t m_restartPos;
    //! @brief The current pattern delay countdown, 0 if unused
    uint8_t m_currentPatternDelay;
    //! @brief The requested pattern delay countdown, 0 if unused
    uint8_t m_requestedPatternDelay;
public:
    /**
     * @brief Factory method
     * @param[in] filename Filename of the module
     * @param[in] frequency Rendering frequency
     * @param[in] maxRpt Maximum repeat count
     * @return Pointer to the loaded module or nullptr on error
     * @details
     * Loads and initializes the module if possible
     */
    static AbstractModule* factory(Stream* stream, uint32_t frequency, int maxRpt, Sample::Interpolation inter);
    virtual ~XmModule();
private:
    size_t internal_buildTick(AudioFrameBuffer* buffer) override;
    ChannelState internal_channelStatus(size_t) const override;
    int internal_channelCount() const override;
    /**
     * @brief Constructor
     * @param[in] maxRpt maximum repeat count per order
     */
    XmModule(int maxRpt, Sample::Interpolation inter);
    /**
     * @brief Try to load a XM module
     * @param[in] filename Filename of the module to load
     * @retval true on success
     * @retval false on error
     */
    bool load(Stream* stream);
    /**
     * @brief Processes jumps
     * @param[in] estimateOnly Used when estimating track length
     * @return @c false when end of song is reached
     */
    bool adjustPosition();
    /**
     * @brief Get an instrument
     * @param[in] idx 1-based instrument index
     * @return Instrument pointer or nullptr
     */
    const XmInstrument* getInstrument(int idx) const;
    /**
     * @brief Map a note and finetune to its base period
     * @param[in] note Note
     * @param[in] finetune Finetune
     * @return Period
     */
    uint16_t noteToPeriod(uint8_t note, int8_t finetune) const;
    /**
     * @brief Calculate the frequency from a period
     * @param[in] period The period
     * @return The calculated frequency
     */
    uint32_t periodToFrequency(uint16_t period) const;
    /**
     * @brief Apply glissando to a period
     * @param[in] period Input period
     * @param[in] finetune Input finetune
     * @param[in] deltaNote Delta note used e.g. for Arpeggio
     * @return Quantisized period
     */
    uint16_t glissando(uint16_t period, int8_t finetune, uint8_t deltaNote = 0) const;
    /**
     * @brief Reverse calculates a period to its fine note index
     * @param[in] period The period to reverse calculate
     * @param[in] finetune The finetune of the @a period
     * @param[in] deltaNote The optional relative note
     * @return The fine note index. It is 16 times finer than the real note index.
     */
    uint16_t periodToFineNoteIndex(uint16_t period, int8_t finetune, uint8_t deltaNote = 0) const;
    /**
     * @brief Request pattern break
     * @param[in] next Row to break to
     */
    void doPatternBreak(int16_t next);
    /**
     * @brief Request order jump
     * @param[in] next Order to jump to
     */
    void doJumpPos(int16_t next);
    /**
     * @brief Request pattern loop
     * @param[in] next Row to jump to
     */
    void doPatLoop(int16_t next);
    AbstractArchive& serialize(AbstractArchive* data) override;
    /**
     * @brief Check if there is a running pattern delay
     * @retval true if there is a running pattern delay
     */
    bool isRunningPatDelay() const;
    /**
     * @brief Request a pattern delay
     * @param[in] counter The number of rows the pattern should be delayed
     * @note Ignored if there is already a running pattern delay
     */
    void doPatDelay(uint8_t counter);
    /**
     * @brief Get the logger
     * @return Child logger with attached ".xm"
     */
    static light4cxx::Logger* logger();
};
} // namespace xm
} // namespace ppp

/**
 * @}
 */

#endif // XMMODULE_H
