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

#ifndef PPPLAY_ABSTRACTAUDIOSOURCE_H
#define PPPLAY_ABSTRACTAUDIOSOURCE_H

#include "ppplay_core_export.h"
#include <stuff/utils.h>
#include "audiotypes.h"

#include <light4cxx/logger.h>

#include <mutex>

/**
 * @ingroup Output
 * @{
 */

/**
 * @interface IAudioSource
 * @brief Audio source for IAudioOutput
 */

class PPPLAY_CORE_EXPORT AbstractAudioSource
{
private:
	//! @brief @c true when this source was successfully inited
	bool m_initialized;
	//! @brief Frequency of this source
	uint32_t m_frequency;
	bool m_paused;
    mutable std::recursive_mutex m_mutex;
protected:
	/**
	 * @brief Sets m_initialized to @c false and m_frequency to @c 0
	 * @return @c false
	 */
	bool fail();
private:
	/**
	 * @brief Get audio data from the source
	 * @param[out] buffer The buffer containing the retrieved data
	 * @param[in] requestedFrames Number of requested frames
	 * @returns The number of frames actually returned - should be equal to @c buffer->size()
	 * @note If this function returns 0, the audio output device should stop playback
	 * @see preferredBufferSize()
	 * @see isBusy()
	 */
	virtual size_t internal_getAudioData( AudioFrameBuffer& buffer, size_t requestedFrames ) = 0;
	/**
	 * @brief Get the preferred buffer size to prevent unnecessary calls to getAudioData()
	 * @retval 0 if no size is specified
	 */
	virtual size_t internal_preferredBufferSize() const;
	/**
	 * @brief Initializes the source with given frequency
	 * @param[in] frequency Requested frequency
	 * @return @c true on success
	 */
	virtual bool internal_initialize( uint32_t frequency ) = 0;
	/**
	 * @brief Get the left channel's volume
	 * @return Left channel's volume, default is 0
	 */
	virtual uint16_t internal_volumeLeft() const ;
	/**
	 * @brief Get the right channel's volume
	 * @return Right channel's volume, default is 0
	 */
	virtual uint16_t internal_volumeRight() const;
public:
	//! @brief Class pointer
	typedef std::shared_ptr<AbstractAudioSource> Ptr;
	//! @brief Weak class pointer
	typedef std::weak_ptr<AbstractAudioSource> WeakPtr;
	//! @brief Constructor
	AbstractAudioSource() noexcept;
	//! @brief Destructor
	virtual ~AbstractAudioSource() = default;
	/**
	 * @brief Check whether this source was successfully initialized
	 * @return m_initialized
	 */
	bool initialized() const noexcept;
	/**
	 * @brief Get this source's frequency
	 * @return m_frequency
	 */
	uint32_t frequency() const noexcept;
	inline bool paused() const noexcept;
	inline void setPaused( bool p = true ) noexcept;
	//! @copydoc internal_getAudioData
	size_t getAudioData( AudioFrameBuffer& buffer, size_t requestedFrames );
	//! @copydoc internal_preferredBufferSize
	size_t preferredBufferSize() const;
	//! @copydoc internal_initialize
	bool initialize( uint32_t frequency );
	//! @copydoc internal_volumeLeft
	uint16_t volumeLeft() const;
	//! @copydoc internal_volumeRight
	uint16_t volumeRight() const;
protected:
	/**
	 * @brief Get the logger
	 * @return Logger with name "audio.source"
	 */
	static light4cxx::Logger* logger();
};

bool AbstractAudioSource::paused() const noexcept
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
	return m_paused;
}
void AbstractAudioSource::setPaused( bool p ) noexcept
{
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
	m_paused = p;
}

/**
 * @}
 */

#endif // IAUDIOSOURCE_H
