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

#ifndef PPPLAY_AUDIOFIFO_H
#define PPPLAY_AUDIOFIFO_H

#include "stuff/utils.h"
#include "audiotypes.h"
#include "abstractaudiosource.h"

#include "light4cxx/logger.h"

/**
 * @ingroup Output
 * @{
 */

/**
 * @class AudioFifo
 * @brief Audio FIFO buffer
 *
 * @details
 * A simple thread is created that continuously requests data from the connected
 * IAudioSource.
 */
class AudioFifo
{
	DISABLE_COPY( AudioFifo )
	AudioFifo() = delete;
private:
	//! @brief Queued audio chunks
	AudioFrameBufferQueue m_queue;
	//! @brief Number of frames in the queue
	size_t m_queuedFrames;
	//! @brief Minimum number of frames the queue should contain
	size_t m_minFrameCount;
	//! @brief The requester thread that pulls the audio data from the source
	boost::thread m_requestThread;
	//! @brief The audio source to pull the data from
	AbstractAudioSource::WeakPtr m_source;
	//! @brief Sum of all left absolute sample values
	uint64_t m_volLeftSum;
	//! @brief Sum of all right absolute sample values
	uint64_t m_volRightSum;
	const bool m_doVolumeCalc;
	bool m_stopping;
	mutable boost::recursive_mutex m_mutex;
	/**
	 * @brief Audio data pulling thread function
	 * @param[in] fifo The FIFO that owns the thread
	 * @note Declared here to get access to private members of the AudioFifo
	 * @see m_requestThread
	 */
	void requestThread();
	/**
	 * @brief Adds a buffer to the internal queue by copying its contents
	 * @param[in] buf The buffer to add
	 */
	void pushBuffer( const AudioFrameBuffer& buf );
public:
	/**
	 * @brief Initialize the buffer
	 * @param[in] source The audio source that should be buffered
	 * @param[in] minFrameCount Initial value for m_minFrameCount (minimum 256)
	 */
	AudioFifo( const AbstractAudioSource::WeakPtr& source, size_t minFrameCount, bool doVolumeCalc );
	~AudioFifo();
	/**
	 * @brief Get the number of buffered frames
	 * @return Number of buffered frames
	 */
	size_t queuedLength() const;
	/**
	 * @brief Get the minimum number of frames that should be queued
	 * @return m_minFrameCount
	 */
	size_t minFrameCount() const;
	/**
	 * @brief Get the number of queued chunks
	 * @return Number of queued chunks
	 */
	size_t queuedChunkCount() const;
	/**
	 * @brief Set the FIFO buffer length
	 * @param[in] len The requested buffer length (minimum 256)
	 */
	void setMinFrameCount( size_t len );
	/**
	 * @brief Check if the FIFO is empty
	 * @retval true FIFO is empty
	 * @retval false FIFO is not empty
	 */
	bool isEmpty() const;
	uint16_t volumeLeft() const;
	uint16_t volumeRight() const;
	size_t pullData( AudioFrameBuffer& buffer, size_t requestedFrames );
protected:
	/**
	 * @brief Get the logger
	 * @return Logger with name "audio.fifo"
	 */
	static light4cxx::Logger* logger();
};

/**
 * @}
 */

#endif //audiofifoH
