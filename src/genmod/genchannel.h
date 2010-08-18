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
#include "genpattern.h"
#include "phaser.h"

/**
 * @file
 * @ingroup GenMod
 * @brief General channel definitions
 */

namespace ppp {
	/**
	 * @ingroup GenMod
	 * @brief Vibrato types
	 */
	enum class GenVibratoType : uint8_t {
		vtSine = 0x00, //!< @brief Sine wave, retriggered with each new note
		vtRamp = 0x01, //!< @brief Ramp wave, retriggered with each new note
		vtSquare = 0x02, //!< @brief Square wave, retriggered with each new note
		vtRandom = 0x03, //!< @brief Random wave, retriggered with each new note
		vtSineNoRetrigger = 0x04, //!< @brief Sine wave, @b not retriggered with each new note
		vtRampNoRetrigger = 0x05, //!< @brief Ramp wave, @b not retriggered with each new note
		vtSquareNoRetrigger = 0x06, //!< @brief Square wave, @b not retriggered with each new note
		vtRandomNoRetrigger = 0x07 //!< @brief Random wave, @b not retriggered with each new note
	};
	/**
	 * @class GenChannel
	 * @ingroup GenMod
	 * @brief A general channel
	 * @details Every module must be compatible with this abstract
	 * base class.
	 */
	class GenChannel {
		public:
			typedef std::shared_ptr<GenChannel> Ptr; //!< @brief Class pointer
		private:
			bool m_active; //!< @brief @c true if channel is active
			bool m_disabled; //!< @brief @c true if channel is disabled
			Phaser m_vibrato; //!< @brief Vibrato effect helper
			Phaser m_tremolo; //!< @brief Tremolo effect helper
			uint8_t m_panning; //!< @brief Panning (0x00..0x80)
			uint8_t m_volume; //!< @brief Volume (0x00..0x40)
			Frequency m_frequency; //!< @brief Frequency
			uint8_t m_tick; //!< @brief Current tick
			int32_t m_position; //!< @brief Current sample position
			GenSampleList::Ptr m_sampleList; //!< @brief Pointer to the sample list
			std::string m_statusString; //!< @brief Status string
			int32_t m_currSmpIndex; //!< @brief Sample index
			GenCell::Ptr m_currCell; //!< @brief Copy of the currently playing cell
			Frequency m_playbackFrequency; //!< @brief Playback frequency, default is 44100 @see GenModule::GenModule
			virtual void updateSamplePtr( int n ) throw( PppException ); //!< @brief Updates the sample pointer if the sample index has changed
		public:
			/**
			 * @brief The constructor
			 * @param[in] frq Playback frequency
			 * @param[in] smp Pointer to the sample list used in this channel
			 * @pre @c smp!=NULL
			 * @see GenModule::GenModule()
			 */
			GenChannel( const Frequency frq, const GenSampleList::Ptr &smp ) throw( PppException );
			/**
			 * @brief Member list initialization constructor
			 * @param[in] src Source instance
			 */
			GenChannel( const GenChannel &src ) throw();
			/**
			 * @brief Assignment operator
			 * @param[in] src Source instance
			 * @return Reference to this instance
			 * @warning No clean-up steps are made
			 */
			GenChannel &operator=( const GenChannel &src ) throw();
			/**
			 * @brief The destructor
			 */
			virtual ~GenChannel() throw();
			/**
			 * @brief Check if the channel is active
			 * @return #aActive
			 * @note Time-critical
			 */
			inline bool isActive() const throw() { return m_active; }
			/**
			 * @brief Check if the channel is disabled
			 * @return #aDisabled
			 * @note Time-critical
			 */
			inline bool isDisabled() const throw() { return m_disabled; }
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
			 * @brief Get the bare base frequency
			 * @return Frequency, 0 if channel is disabled or inactive
			 * @note Time-critical
			 */
			virtual Frequency getBareFrq() const throw() { return m_frequency; }
			/**
			 * @brief Update the channel
			 * @param[in] cell Pointer to a note cell
			 * @param[in] tick Current tick
			 * @param[in] noRetrigger Don't trigger new notes, only apply effects (i.e. for Pattern Delays)
			 */
			virtual void update( GenCell::Ptr const cell, const uint8_t tick, bool noRetrigger = false ) throw( PppException ) = 0;
			/**
			 * @brief Disables this channel.
			 */
			inline void disable() throw() { m_disabled = true; }
			/**
			 * @brief Enables this channel.
			 */
			inline void enable() throw() { m_disabled = false; }
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
			inline std::string getStatus() const throw() { return m_statusString; }
			/**
			 * @brief Get a short description of the current effect
			 * @return The current effect in the format @c xxxxxS, where @c x is a short description and @c S is a symbol
			 * @see #getFxName()
			 */
			virtual std::string getFxDesc() const throw( PppException ) = 0;
			/**
			 * @brief Save the channel's state to a BinStream
			 * @param[in,out] str Stream to save the state to
			 * @return @a str for pipelining
			 */
			virtual BinStream &saveState( BinStream &str ) const throw( PppException );
			/**
			 * @brief Load the channel's state from a BinStream
			 * @param[in,out] str Stream to load the state from
			 * @return @a str for pipelining
			 */
			virtual BinStream &restoreState( BinStream &str ) throw( PppException );
			/**
			 * @brief Get a string representation of the current cell as displayed in the tracker
			 * @return String representation of the current cell like in the tracker
			 */
			std::string getCellString() {
				if ( !m_currCell )
					return "";
				return m_currCell->trackerString();
			}
		protected:
			void setCurrentCell(const GenCell::Ptr &cell) { m_currCell = cell; }
			GenCell::Ptr getCurrentCell() const throw() { return m_currCell; }
			GenSample::Ptr getCurrentSample() throw( PppException );
			uint8_t getTick() const throw() { return m_tick; }
			void setTick(uint8_t t) throw() { m_tick = t; }
			void setActive(bool a) throw() { m_active = a; }
			const Phaser &vibrato() const throw() { return m_vibrato; }
			Phaser &vibrato() throw() { return m_vibrato; }
			const Phaser &tremolo() const throw() { return m_tremolo; }
			Phaser &tremolo() throw() { return m_tremolo; }
			void setSampleIndex(int idx) { updateSamplePtr(idx); }
			uint8_t getVolume() const throw() { return m_volume; }
			void setVolume(uint8_t v) throw() { m_volume = v; }
			void setBareFrq(Frequency f) throw() { m_frequency = f; }
			void setPosition(int32_t p) throw() { m_position = p; }
			uint8_t getPanning() const throw() { return m_panning; }
			Frequency getPlaybackFrq() const throw() { return m_playbackFrequency; }
			void setStatusString(const std::string &s) { m_statusString = s; }
			int32_t getCurrentSmpIdx() const throw() { return m_currSmpIndex; }
	};

	/**
	* @brief Contains the channels
	* @ingroup GenMod
	*/
	typedef PVector<GenChannel> GenChannelList;
} // namespace ppp

PVECTOR_TEMPLATE_DECL(ppp::GenChannel)

SHARED_PTR_DECL(ppp::GenChannel)

#endif
