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

#ifndef IAUDIOOUTPUT_H
#define IAUDIOOUTPUT_H

/**
 * @ingroup Output
 * @{
 */

#include "iaudiosource.h"

/**
 * @interface IAudioOutput
 * @brief Abstract base class for sound output
 */
class IAudioOutput {
		DISABLE_COPY(IAudioOutput)
		IAudioOutput() = delete;
	public:
		//! @brief Audio output device error codes
		enum ErrorCode {
			NoError, //!< @brief No error
			InputError, //!< @brief General input error
			InputDry, //!< @brief Audio source can't provide data
			OutputError, //!< @brief General output error
			OutputDry, //!< @brief Output device needs more data than available
			OutputUnavailable //!< @brief Output device is unavailable
		};
		//! @brief Class pointer
		typedef std::shared_ptr<IAudioOutput> Ptr;
		//! @brief Weak class pointer
		typedef std::weak_ptr<IAudioOutput> WeakPtr;
		/**
		 * @brief Constructor
		 * @param[in] src Pointer to an audio data source
		 */
		explicit IAudioOutput(const IAudioSource::WeakPtr& src) : m_source(src), m_errorCode(NoError) {}
		//! @brief Destructor
		virtual ~IAudioOutput();
		/**
		 * @brief Initialize output device
		 * @param[in] desiredFrq Desired output frequency
		 * @retval 0 Initialization failed
		 * @retval other Real output frequency
		 */
		virtual int init(int desiredFrq) = 0;
		/**
		 * @brief Check if the output is in playing state
		 * @return @c true if the output is in playing state
		 */
		virtual bool playing() = 0;
		/**
		 * @brief Check if the output is in paused state
		 * @return @c true if the output is in paused state
		 */
		virtual bool paused() = 0;
		/**
		 * @brief Start playback
		 */
		virtual void play() = 0;
		/**
		 * @brief Pause playback
		 */
		virtual void pause() = 0;
		/**
		 * @brief Get the attached audio source
		 * @return Pointer to the attached audio source
		 */
		IAudioSource::WeakPtr source() const;
		/**
		 * @brief Get the left channel's volume
		 * @return Left channel's volume, defaults to the source's volume
		 */
		virtual uint16_t volumeLeft() const;
		/**
		 * @brief Get the right channel's volume
		 * @return Right channel's volume, defaults to the source's volume
		 */
		virtual uint16_t volumeRight() const;
		/**
		 * @brief Get the internal error code
		 * @return Internal error code
		 */
		ErrorCode errorCode() const;
	protected:
		/**
		 * @brief Set the internal error code
		 * @param[in] ec New error code
		 */
		void setErrorCode(ErrorCode ec);
		/**
		 * @brief Get the logger
		 * @return Logger with name "audio.output"
		 */
		static light4cxx::Logger::Ptr logger();
	private:
		IAudioSource::WeakPtr m_source; //!< @brief The audio source
		ErrorCode m_errorCode; //!< @brief Internal error code
};

/**
 * @}
 */

#endif
