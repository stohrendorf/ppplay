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
 * @brief Audio FIFO buffer class (definition)
 */

//#include <cstring>
#include "stuff/pppexcept.h"
#include "stuff/utils.h"
#include <algorithm>
#include <vector>
#include <memory>
#include <deque>

namespace ppp {
	typedef int16_t BasicSample;
	struct __attribute__((packed)) BasicSampleFrame { BasicSample left,right; };
	typedef int32_t MixerSample;
	struct __attribute__((packed)) MixerSampleFrame { MixerSample left,right; };
	typedef std::shared_ptr< std::vector<BasicSampleFrame> > AudioFrameBuffer; //!< @brief Audio buffer
	typedef std::shared_ptr< std::vector<MixerSampleFrame> > MixerFrameBuffer; //!< @brief Mixer buffer
	typedef std::deque< AudioFrameBuffer > AudioFrameBufferQueue;
	/**
	 * @class AudioFifo
	 * @ingroup Output
	 * @brief Audio FIFO buffer
	 */
	class AudioFifo {
		private:
			/**
			 * @brief Deny copy operator
			 */
			AudioFifo &operator=(const AudioFifo &) = delete;
			/**
			 * @brief Deny default constructor
			 */
			AudioFifo() = delete;
			/**
			 * @brief Deny copy constructor
			 */
			AudioFifo(const AudioFifo&) = delete;
			/**
			 * @brief Adjusts volume values
			 * @param[in,out] leftVol Left channel volume
			 * @param[in,out] rightVol Right channel volume
			 * @see calcVolume
			 */
			void logify(uint16_t &leftVol, uint16_t &rightVol) throw ();
			/**
			 * @brief Calculate the volume of the audio buffer
			 * @param[out] leftVol Left volume
			 * @param[out] rightVol Right volume
			 */
			void calcVolume(uint16_t &leftVol, uint16_t &rightVol) throw(PppException);
			AudioFrameBufferQueue m_queue;
			std::size_t m_queuedFrames;
			std::size_t m_minFrameCount;
			uint16_t m_volumeLeft, m_volumeRight;
		public:
			static const std::size_t nsize = ~0;
			/**
			 * @brief Initialize the buffer
			 * @param[in] frameCount Number of sample frames to allocate
			 */
			AudioFifo(const std::size_t minFrameCount);
			/**
			 * @brief Get the number of buffered frames
			 * @return Number of buffered frames
			 */
			std::size_t getQueuedLength() const throw() { return m_queuedFrames; }
			std::size_t getMinFrameCount() const throw() { return m_minFrameCount; }
			std::size_t getQueuedChunks() const throw() { return m_queue.size(); }
			/**
			 * @brief Returns @c true if this buffer needs more data
			 * @return @c true if this buffer needs more data
			 */
			bool needsData() const throw() { return m_queuedFrames<m_minFrameCount; }
			/**
			 * @brief Feed data into the buffer
			 * @param[in] data Source of the data
			 * @param[in] len Source data length
			 */
			void feedChunk(const ppp::AudioFrameBuffer& data) throw(PppException);
			/**
			 * @brief Copy the internal FIFO buffer to @a data
			 * @param[out] data Destination buffer
			 * @return Copied frames
			 * @details This also updates the volumes
			 */
			std::size_t getAll(AudioFrameBuffer &data);
			std::size_t get(AudioFrameBuffer &data, std::size_t size);
			std::size_t copy(AudioFrameBuffer &data, std::size_t size);
			/**
			 * @brief Set the FIFO buffer length
			 * @param[in] len The requested buffer length
			 */
			void setMinFrameCount(const std::size_t len) { m_minFrameCount = len; }
			uint16_t getVolumeLeft() const { return m_volumeLeft; }
			uint16_t getVolumeRight() const { return m_volumeRight; }
	};
}

#endif //audiofifoH
