/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef genchannelH
#define genchannelH

#include <cstdint>
#include "gensample.h"
#include "gencell.h"
#include "phaser/phaser.h"
#include "stuff/utils.h"
#include <mutex>

/**
 * @file
 * @ingroup GenMod
 * @brief General channel definitions
 */

namespace ppp {
	/**
	 * @class GenChannel
	 * @ingroup GenMod
	 * @brief A general channel
	 * @details Every module must be compatible with this abstract
	 * base class.
	 */
	class GenChannel : public ISerializable {
			DISABLE_COPY(GenChannel)
			GenChannel() = delete;
		public:
			typedef std::shared_ptr<GenChannel> Ptr; //!< @brief Class pointer
			typedef std::vector<Ptr> Vector;
		private:
			bool m_active; //!< @brief @c true if channel is active
			bool m_disabled; //!< @brief @c true if channel is disabled
			Phaser m_vibrato; //!< @brief Vibrato effect helper
			Phaser m_tremolo; //!< @brief Tremolo effect helper
			uint8_t m_panning; //!< @brief Panning (0x00..0x80)
			uint8_t m_volume; //!< @brief Volume (0x00..0x40)
			uint8_t m_tick; //!< @brief Current tick
			int32_t m_position; //!< @brief Current sample position
			std::string m_statusString; //!< @brief Status string
			uint16_t m_playbackFrequency; //!< @brief Playback frequency, default is 44100 @see GenModule::GenModule
			std::mutex m_statusStringMutex;
		public:
			/**
			 * @brief The constructor
			 * @param[in] frq Playback frequency
			 * @pre @c smp!=NULL
			 * @see GenModule::GenModule()
			 */
			GenChannel( const uint16_t frq ) throw( PppException );
			/**
			 * @brief The destructor
			 */
			virtual ~GenChannel() throw();
			/**
			 * @brief Check if the channel is active
			 * @return #aActive
			 * @note Time-critical
			 */
			bool isActive() const throw() { return m_active; }
			/**
			 * @brief Check if the channel is disabled
			 * @return #aDisabled
			 * @note Time-critical
			 */
			bool isDisabled() const throw() { return m_disabled; }
			/**
			 * @brief Get the name of the note
			 * @return String containing note and octave (i.e. "C-3")
			 * @note Return value is empty if channel is disabled or inactive
			 * @see ppp::NoteNames[]
			 * @note Time-critical
			 */
			virtual std::string getNoteName() throw( PppException ) = 0;
			/**
			 * @brief Get the effect string as shown in the tracker's FX column
			 * @return Effect string
			 * @note Time-critical
			 * @note Return value is empty if no effect is playing, or if the channel is
			 * disabled or inactive
			 * @see #getFxDesc()
			*/
			virtual std::string getFxName() const throw( PppException ) = 0;
			/**
			 * @brief Get the playback position
			 * @return Playback position in the channel's sample
			 * @note Time-critical
			 */
			int32_t getPosition() const throw() { return m_position; }
			/**
			 * @brief Disables this channel.
			 */
			void disable() throw() { m_disabled = true; }
			/**
			 * @brief Enables this channel.
			 */
			void enable() throw() { m_disabled = false; }
			/**
			 * @brief Sets the panning of this channel.
			 * @param[in] pan Panning value
			 * @note @c 0xa4 is surround sound
			 * @pre @c (0x00\<=pan\<=0x80)||(pan==0xa4)
			 */
			virtual void setPanning( uint8_t pan ) throw( PppException ) {
				PPP_TEST( ( pan > 0x80 ) && ( pan != 0xa4 ) );
				m_panning = pan;
			}
			/**
			 * @brief Mix the current channel into @a mixBuffer.
			 * @param[in,out] mixBuffer Pointer to the mixer buffer
			 * @param[in] bufSize Number of frames in the buffer
			 * @param[in] volume Global volume
			 * @pre @c mixBuffer!=NULL
			 * @pre @c bufSize>0
			 * @note Time-critical
			 */
			virtual void mixTick( MixerFrameBuffer &mixBuffer, const uint8_t volume ) throw( PppException ) = 0;
			/**
			 * @brief Simulates a tick without mixing
			 * @param[in] bufSize Buffer size
			 * @param[in] volume Volume
			 * @see #mixTick
			 */
			virtual void simTick( const std::size_t bufSize, const uint8_t volume ) = 0;
			/**
			 * @brief Updates the status string returned by #getStatus()
			 * @note Time-critical
			 */
			virtual void updateStatus() throw( PppException ) = 0;
			/**
			 * @brief Returns the status string
			 * @return #aStatusString
			 */
			std::string getStatus() throw();
			/**
			 * @brief Get a short description of the current effect
			 * @return The current effect in the format @c xxxxxS, where @c x is a short description and @c S is a symbol
			 * @see #getFxName()
			 */
			virtual std::string getFxDesc() const throw( PppException ) = 0;
			virtual IArchive& serialize(IArchive* data);
			/**
			 * @brief Get a string representation of the current cell as displayed in the tracker
			 * @return String representation of the current cell like in the tracker
			 */
			virtual std::string getCellString() = 0;
		protected:
			uint8_t getTick() const throw() { return m_tick; }
			void setTick(uint8_t t) throw() { m_tick = t; }
			void setActive(bool a) throw() { m_active = a; }
			const Phaser &vibrato() const throw() { return m_vibrato; }
			Phaser &vibrato() throw() { return m_vibrato; }
			const Phaser &tremolo() const throw() { return m_tremolo; }
			Phaser &tremolo() throw() { return m_tremolo; }
			uint8_t getVolume() const throw() { return m_volume; }
			void setVolume(uint8_t v) throw() { m_volume = v; }
			void setPosition(int32_t p) throw() { m_position = p; }
			uint8_t getPanning() const throw() { return m_panning; }
			uint16_t getPlaybackFrq() const throw() { return m_playbackFrequency; }
			void setStatusString(const std::string &s);
	};

} // namespace ppp

#endif
