/*
    PeePeePlayer - an old-fashioned module player
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

#include "genmod/genmodule.h"
#include "xmpattern.h"
#include "xminstrument.h"
#include "xmchannel.h"

namespace ppp {
namespace xm {

/**
 * @class XmModule
 * @brief XM module class
 */
class XmModule : public GenModule {
		DISABLE_COPY(XmModule)
	private:
		//! @brief @c true if amiga period table is used
		bool m_amiga;
		//! @brief Module patterns
		XmPattern::Vector m_patterns;
		//! @brief Module instruments
		XmInstrument::Vector m_instruments;
		//! @brief Module channels
		XmChannel::Vector m_channels;
		//! @brief Maps notes including finetune to their periods
		std::array<uint16_t, 121 * 16> m_noteToPeriod;
		//! @brief Contains the row to break to, or @c -1 if no break is intended
		int16_t m_jumpRow;
		//! @brief Contains the order to jump to, or @c -1 if no jump is intended
		int16_t m_jumpOrder;
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
		/**
		 * @brief Increases tick values and processes jumps
		 * @param[in] doStore Set to @c true to store the state on order change
		 * @return @c false when end of song is reached
		 */
		bool adjustPosition(bool doStore);
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
		static GenModule::Ptr factory(const std::string& filename, uint32_t frequency, uint8_t maxRpt);
		//! @brief Class pointer
		typedef std::shared_ptr<XmModule> Ptr;
		/**
		 * @brief Constructor
		 * @param[in] maxRpt maximum repeat count per order
		 */
		XmModule(uint8_t maxRpt);
		/**
		 * @brief Try to load a XM module
		 * @param[in] filename Filename of the module to load
		 * @retval true on success
		 * @retval false on error
		 */
		bool load(const std::string& filename);
		virtual void buildTick(AudioFrameBuffer& buffer);
		virtual void simulateTick(size_t&);
		virtual ppp::GenOrder::Ptr mapOrder(int16_t);
		virtual std::string channelStatus(int16_t);
		virtual bool jumpNextSong();
		virtual bool jumpPrevSong();
		virtual bool jumpNextOrder();
		virtual bool jumpPrevOrder();
		virtual std::string channelCellString(int16_t);
		virtual uint8_t channelCount() const;
		/**
		 * @brief Get an instrument
		 * @param[in] idx 1-based instrument index
		 * @return Instrument pointer or nullptr
		 */
		XmInstrument::Ptr getInstrument(int idx) const;
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
		IArchive& serialize(IArchive* data);
		virtual bool initialize(uint32_t frq);
		bool isRunningPatDelay() const;
		void doPatDelay(uint8_t counter);
	protected:
		static log4cxx::LoggerPtr logger();
};

} // namespace xm
} // namespace ppp

/**
 * @}
 */

#endif // XMMODULE_H
