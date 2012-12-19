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

#ifndef PPPLAY_ABSTRACTCHANNEL_H
#define PPPLAY_ABSTRACTCHANNEL_H

#include "ppplay_core_export.h"

#include <stream/iserializable.h>
#include <output/audiotypes.h>
#include <light4cxx/logger.h>

#include <boost/thread.hpp>

namespace ppp
{
/**
 * @ingroup GenMod
 * @{
 */

/**
 * @class AbstractChannel
 * @brief A general channel
 * @details Every module must be compatible with this abstract
 * base class.
 */
class PPPLAY_CORE_EXPORT AbstractChannel : public ISerializable
{
	DISABLE_COPY( AbstractChannel )
private:
	//! @brief @c true if channel is active
	bool m_active;
	//! @brief @c true if channel is disabled
	bool m_disabled;
	//! @brief Status string
	std::string m_statusString;
	mutable boost::recursive_mutex m_mutex;
public:
	/**
	 * @brief The constructor
	 */
	explicit AbstractChannel();
	/**
	 * @brief The destructor
	 */
	virtual ~AbstractChannel() = default;
	/**
	 * @brief Check if the channel is active
	 * @return m_active
	 */
	bool isActive() const noexcept;
	/**
	 * @brief Check if the channel is disabled
	 * @return m_disabled
	 */
	bool isDisabled() const noexcept;
	/**
	 * @brief Disables this channel.
	 */
	void disable() noexcept;
	/**
	 * @brief Enables this channel.
	 */
	void enable() noexcept;
	/**
	 * @brief Returns the status string
	 * @return m_statusString
	 */
	std::string statusString() const;
	virtual AbstractArchive& serialize( AbstractArchive* data );
	//! @copydoc internal_noteName
	std::string noteName() const;
	//! @copydoc internal_effectName
	std::string effectName() const;
	//! @copydoc internal_mixTick
	void mixTick( MixerFrameBuffer* mixBuffer );
	//! @copydoc internal_updateStatus
	void updateStatus();
	//! @copydoc internal_effectDescription
	std::string effectDescription() const;
	//! @copydoc internal_cellString
	std::string cellString() const;
protected:
	/**
	 * @brief Set the m_active value
	 * @param[in] a The new value
	 */
	void setActive( bool a ) noexcept;
	/**
	 * @brief Sets m_statusString
	 * @param[in] s The new string
	 */
	void setStatusString( const std::string& s );
	/**
	 * @brief Get the logger
	 * @return Logger with name "channel"
	 */
	static light4cxx::Logger* logger();
private:
	/**
	 * @brief Get the name of the note
	 * @return String containing note and octave (i.e. "C-3")
	 * @note Return value is empty if channel is disabled or inactive
	 * @see ppp::NoteNames
	 */
	virtual std::string internal_noteName() const = 0;
	/**
	 * @brief Get the effect string as shown in the tracker's FX column
	 * @return Effect string
	 * @note Return value is empty if no effect is playing, or if the channel is
	 * disabled or inactive
	 * @see effectDescription()
	*/
	virtual std::string internal_effectName() const = 0;
	/**
	 * @brief Mix the current channel into @a mixBuffer.
	 * @param[in,out] mixBuffer Pointer to the mixer buffer
	 * @note Time-critical
	 * @see GenModule::buildTick()
	 * 
	 * @details
	 * When @a mixBuffer is @c NULL, only effects shall be executed that are
	 * necessary for length estimation.
	 * 
	 * This behaviour is necessary here in the channel, because some effects may
	 * cause callbacks to the GenModule or its derived children.
	 */
	virtual void internal_mixTick( MixerFrameBuffer* mixBuffer ) = 0;
	/**
	 * @brief Updates the status string returned by statusString()
	 */
	virtual void internal_updateStatus() = 0;
	/**
	 * @brief Get a short description of the current effect
	 * @return The current effect in the format @c xxxx[xS]S, where @c x is a short description and @c S is a symbol
	 * @see effectName()
	 */
	virtual std::string internal_effectDescription() const = 0;
	/**
	 * @brief Get a string representation of the current cell as displayed in the tracker
	 * @return String representation of the current cell like in the tracker
	 */
	virtual std::string internal_cellString() const = 0;
};

/**
 * @}
 */

} // namespace ppp

#endif
