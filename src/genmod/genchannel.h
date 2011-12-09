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

#ifndef GENCHANNEL_H
#define GENCHANNEL_H

/**
 * @ingroup GenMod
 * @{
 */

#include "stream/iserializable.h"
#include "output/audiotypes.h"
#include "stuff/utils.h"
#include "gensample.h"

#include "light4cxx/logger.h"

#include <mutex>

namespace ppp {
/**
 * @class GenChannel
 * @brief A general channel
 * @details Every module must be compatible with this abstract
 * base class.
 */
class GenChannel : public ISerializable {
		DISABLE_COPY(GenChannel)
	public:
		typedef std::shared_ptr<GenChannel> Ptr; //!< @brief Class pointer
		typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
	private:
		bool m_active; //!< @brief @c true if channel is active
		bool m_disabled; //!< @brief @c true if channel is disabled
		GenSample::PositionType m_position; //!< @brief Current sample position
		std::string m_statusString; //!< @brief Status string
		std::mutex m_statusStringMutex; //!< @brief Mutex for accessing m_statusString
	public:
		/**
		 * @brief The constructor
		 */
		GenChannel();
		/**
		 * @brief The destructor
		 */
		virtual ~GenChannel();
		/**
		 * @brief Check if the channel is active
		 * @return m_active
		 */
		bool isActive() const;
		/**
		 * @brief Check if the channel is disabled
		 * @return m_disabled
		 */
		bool isDisabled() const;
		/**
		 * @brief Get the name of the note
		 * @return String containing note and octave (i.e. "C-3")
		 * @note Return value is empty if channel is disabled or inactive
		 * @see ppp::NoteNames
		 */
		virtual std::string noteName() = 0;
		/**
		 * @brief Get the effect string as shown in the tracker's FX column
		 * @return Effect string
		 * @note Return value is empty if no effect is playing, or if the channel is
		 * disabled or inactive
		 * @see effectDescription()
		*/
		virtual std::string effectName() const = 0;
		/**
		 * @brief Get the playback position
		 * @return Playback position in the channel's sample
		 */
		GenSample::PositionType position() const;
		/**
		 * @brief Disables this channel.
		 */
		void disable();
		/**
		 * @brief Enables this channel.
		 */
		void enable();
		/**
		 * @brief Mix the current channel into @a mixBuffer.
		 * @param[in,out] mixBuffer Reference to the mixer buffer
		 * @note Time-critical
		 */
		virtual void mixTick(MixerFrameBuffer& mixBuffer) = 0;
		/**
		 * @brief Simulates a tick without mixing
		 * @param[in] bufSize Buffer size
		 * @see mixTick(MixerFrameBuffer&)
		 */
		virtual void simTick(size_t bufSize) = 0;
		/**
		 * @brief Updates the status string returned by statusString()
		 */
		virtual void updateStatus() = 0;
		/**
		 * @brief Returns the status string
		 * @return m_statusString
		 */
		std::string statusString();
		/**
		 * @brief Get a short description of the current effect
		 * @return The current effect in the format @c xxxx[xS]S, where @c x is a short description and @c S is a symbol
		 * @see effectName()
		 */
		virtual std::string effectDescription() const = 0;
		/**
		 * @brief Get a string representation of the current cell as displayed in the tracker
		 * @return String representation of the current cell like in the tracker
		 */
		virtual std::string cellString() = 0;
		virtual IArchive& serialize(IArchive* data);
	protected:
		/**
		 * @brief Set the m_active value
		 * @param[in] a The new value
		 */
		void setActive(bool a);
		/**
		 * @brief Set the m_position value
		 * @param[in] p The new value
		 */
		void setPosition( ppp::GenSample::PositionType p );
		/**
		 * @brief Sets m_statusString
		 * @param[in] s The new string
		 */
		void setStatusString(const std::string& s);
		inline void setStatusString(const boost::format& fmt) {
			setStatusString(fmt.str());
		}
		/**
		 * @brief Get the logger
		 * @return Logger with name "channel"
		 */
		static light4cxx::Logger::Ptr logger();
};

} // namespace ppp

/**
 * @}
 */

#endif
