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

#ifndef IAUDIOSOURCE_H
#define IAUDIOSOURCE_H

#include "stuff/utils.h"
#include "audiotypes.h"

#include <mutex>

/**
 * @file
 * @ingroup Output
 * @brief Audio source interface definition
 */

/**
 * @interface IAudioSource
 * @ingroup Output
 * @brief Audio source for IAudioOutput
 */

class IAudioSource {
	private:
		//! @brief @c true when this source was successfully inited
		bool m_initialized;
		//! @brief Frequency of this source
		uint32_t m_frequency;
		//! @brief Mutex for locking the source
		std::mutex m_lockMutex;
	protected:
		/**
		 * @brief Sets m_initialized to @c false and m_frequency to @c 0
		 * @return @c false
		 */
		bool fail();
	public:
		class LockGuard {
			DISABLE_COPY(LockGuard)
		private:
			std::lock_guard<std::mutex> m_guard;
		public:
			LockGuard(IAudioSource* src) : m_guard(src->m_lockMutex) { }
		};
		typedef std::shared_ptr<IAudioSource> Ptr;
		typedef std::weak_ptr<IAudioSource> WeakPtr;
		//! @brief Constructor
		IAudioSource();
		//! @brief Destructor
		virtual ~IAudioSource();
		/**
		 * @brief Get audio data from the source
		 * @param[out] buffer The buffer containing the retrieved data
		 * @param[in] requestedFrames Number of requested frames
		 * @returns The number of frames actually returned - should be equal to @code buffer->size() @endcode
		 * @note If this function returns 0, the audio output device should stop playback
		 */
		virtual std::size_t getAudioData(AudioFrameBuffer& buffer, std::size_t requestedFrames) = 0;
		/**
		 * @brief Initialized the source with given frequency
		 * @param[in] frequency Requested frequency
		 * @return @c true on success
		 * @note This method has a default implementation that returns @c true. Make sure to call it in derived classes to set m_frequency.
		 */
		virtual bool initialize(uint32_t frequency) = 0;
		/**
		 * @brief Check if this source was successfully initialized
		 * @return m_initialized
		 */
		bool initialized() const;
		/**
		 * @brief Get this source's frequency
		 * @return m_frequency
		 */
		uint32_t frequency() const;
		void waitLock();
		bool tryLock();
		void unlock();
		bool isLocked();
};

#endif // IAUDIOSOURCE_H
