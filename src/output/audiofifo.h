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

#ifndef AUDIOFIFO_H
#define AUDIOFIFO_H

/**
 * @file
 * @ingroup Output
 * @brief Audio FIFO buffer class (declaration)
 */

#include "stuff/pppexcept.h"
#include "stuff/utils.h"
#include "audiotypes.h"

#include <algorithm>
#include <memory>
#include <mutex>

namespace ppp {
/**
 * @class AudioFifo
 * @ingroup Output
 * @brief Audio FIFO buffer
 */
class AudioFifo {
		DISABLE_COPY(AudioFifo)
		AudioFifo() = delete;
	private:
		/**
		 * @brief Makes volume values logarithmic
		 * @param[in,out] leftVol Left channel volume
		 * @param[in,out] rightVol Right channel volume
		 * @see calcVolume
		 */
		void logify(uint16_t& leftVol, uint16_t& rightVol);
		/**
		 * @brief Calculate the volume of the audio buffer
		 * @param[out] leftVol Left volume
		 * @param[out] rightVol Right volume
		 */
		void calcVolume(uint16_t& leftVol, uint16_t& rightVol);
		AudioFrameBufferQueue m_queue; //!< @brief Queued audio chunks
		std::size_t m_queuedFrames; //!< @brief Number of frames in the queue
		std::size_t m_minFrameCount; //!< @brief Minimum number of frames the queue should contain
		uint16_t m_volumeLeft; //!< @brief Left volume
		uint16_t m_volumeRight; //!< @brief Right volume
		std::mutex m_queueMutex; //!< @brief Mutex to lock queue access
	public:
		/**
		 * @brief Alias for "No size specified" for where a size parameter is needed
		 */
		static const std::size_t nsize = ~0;
		/**
		 * @brief Initialize the buffer
		 * @param[in] minFrameCount Initial value for m_minFrameCount
		 */
		AudioFifo(std::size_t minFrameCount);
		~AudioFifo();
		/**
		 * @brief Get the number of buffered frames
		 * @return Number of buffered frames
		 */
		std::size_t queuedLength() const;
		/**
		 * @brief Get the minimum number of frames that should be queued
		 * @return m_minFrameCount
		 */
		std::size_t minFrameCount() const;
		/**
		 * @brief Get the number of queued chunks
		 * @return Number of queued chunks
		 */
		std::size_t queuedChunkCount() const;
		/**
		 * @brief Returns @c true if this buffer needs more data
		 * @return @c true if this buffer needs more data
		 */
		bool needsData() const;
		/**
		 * @brief Push data into the buffer
		 * @param[in] data Source of the data
		 */
		void push(const AudioFrameBuffer& data);
		/**
		 * @brief Copy the internal FIFO buffer to @a data
		 * @param[out] data Destination buffer
		 * @return Copied frames
		 * @details This also updates the volumes
		 */
		std::size_t pullAll(AudioFrameBuffer& data);
		/**
		 * @brief Copy part of the internal FIFO buffer to @a data
		 * @param[out] data Destination buffer
		 * @param[in] size Number of frames to copy
		 * @return Copied frames
		 * @details This also updates the volumes
		 */
		std::size_t pull(AudioFrameBuffer& data, std::size_t size);
		/**
		 * @brief Copy part of the internal FIFO buffer to @a data
		 * @param[out] data Destination buffer
		 * @param[in] size Number of frames to copy
		 * @return Copied frames
		 * @details This also updates the volumes
		 */
		std::size_t pull(BasicSampleFrame* data, std::size_t size);
		/**
		 * @brief Copy part of the internal FIFO buffer to @a data without removing them from the queue
		 * @param[out] data Destination buffer
		 * @param[in] size Number of frames to copy
		 * @return Copied frames
		 * @details This also updates the volumes
		 */
		std::size_t copy(AudioFrameBuffer& data, std::size_t size);
		/**
		 * @brief Set the FIFO buffer length
		 * @param[in] len The requested buffer length
		 */
		void setMinFrameCount(std::size_t len);
		/**
		 * @brief Get the left volume
		 * @return The left channel's volume
		 */
		uint16_t volumeLeft() const;
		/**
		 * @brief Get the right volume
		 * @return The right channel's volume
		 */
		uint16_t volumeRight() const;
		bool isEmpty() const;
};
}

#endif //audiofifoH
