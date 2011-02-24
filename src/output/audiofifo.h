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

#ifndef audiofifoH
#define audiofifoH

/**
 * @file
 * @ingroup Output
 * @brief Audio FIFO buffer class (declaration)
 */

//#include <cstring>
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
			DISABLE_COPY( AudioFifo )
			AudioFifo() = delete;
		private:
			/**
			 * @brief Makes volume values logarithmic
			 * @param[in,out] leftVol Left channel volume
			 * @param[in,out] rightVol Right channel volume
			 * @see calcVolume
			 */
			void logify( uint16_t& leftVol, uint16_t& rightVol ) throw();
			/**
			 * @brief Calculate the volume of the audio buffer
			 * @param[out] leftVol Left volume
			 * @param[out] rightVol Right volume
			 */
			void calcVolume( uint16_t& leftVol, uint16_t& rightVol ) throw( PppException );
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
			AudioFifo( std::size_t minFrameCount );
			~AudioFifo();
			/**
			 * @brief Get the number of buffered frames
			 * @return Number of buffered frames
			 */
			std::size_t queuedLength() const throw() {
				return m_queuedFrames;
			}
			/**
			 * @brief Get the minimum number of frames that should be queued
			 * @return m_minFrameCount
			 */
			std::size_t minFrameCount() const throw() {
				return m_minFrameCount;
			}
			/**
			 * @brief Get the number of queued chunks
			 * @return Number of queued chunks
			 */
			std::size_t queuedChunkCount() const throw() {
				return m_queue.size();
			}
			/**
			 * @brief Returns @c true if this buffer needs more data
			 * @return @c true if this buffer needs more data
			 */
			bool needsData() const throw() {
				return m_queuedFrames < m_minFrameCount;
			}
			/**
			 * @brief Push data into the buffer
			 * @param[in] data Source of the data
			 */
			void push( const AudioFrameBuffer& data ) throw( PppException );
			/**
			 * @brief Copy the internal FIFO buffer to @a data
			 * @param[out] data Destination buffer
			 * @return Copied frames
			 * @details This also updates the volumes
			 */
			std::size_t pullAll( AudioFrameBuffer& data );
			/**
			 * @brief Copy part of the internal FIFO buffer to @a data
			 * @param[out] data Destination buffer
			 * @param[in] size Number of frames to copy
			 * @return Copied frames
			 * @details This also updates the volumes
			 */
			std::size_t pull( AudioFrameBuffer& data, std::size_t size );
			/**
			 * @brief Copy part of the internal FIFO buffer to @a data
			 * @param[out] data Destination buffer
			 * @param[in] size Number of frames to copy
			 * @return Copied frames
			 * @details This also updates the volumes
			 */
			std::size_t pull( BasicSampleFrame* data, std::size_t size );
			/**
			 * @brief Copy part of the internal FIFO buffer to @a data without removing them from the queue
			 * @param[out] data Destination buffer
			 * @param[in] size Number of frames to copy
			 * @return Copied frames
			 * @details This also updates the volumes
			 */
			std::size_t copy( AudioFrameBuffer& data, std::size_t size );
			/**
			 * @brief Set the FIFO buffer length
			 * @param[in] len The requested buffer length
			 */
			void setMinFrameCount( std::size_t len ) {
				m_minFrameCount = len;
			}
			/**
			 * @brief Get the left volume
			 * @return The left channel's volume
			 */
			uint16_t volumeLeft() const {
				return m_volumeLeft;
			}
			/**
			 * @brief Get the right volume
			 * @return The right channel's volume
			 */
			uint16_t volumeRight() const {
				return m_volumeRight;
			}
			bool empty() const {
				return m_queuedFrames == 0;
			}
	};
}

#endif //audiofifoH
