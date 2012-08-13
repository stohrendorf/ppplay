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

#ifndef PPPLAY_ABSTRACTAUDIOOUTPUT_H
#define PPPLAY_ABSTRACTAUDIOOUTPUT_H

#include "ppplay_core_export.h"
#include "abstractaudiosource.h"

#include <boost/thread.hpp>

/**
 * @ingroup Output
 * @{
 */

/**
 * @interface IAudioOutput
 * @brief Abstract base class for sound output
 */
class PPPLAY_CORE_EXPORT AbstractAudioOutput
{
	DISABLE_COPY( AbstractAudioOutput )
	AbstractAudioOutput() = delete;
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
	typedef std::shared_ptr<AbstractAudioOutput> Ptr;
	//! @brief Weak class pointer
	typedef std::weak_ptr<AbstractAudioOutput> WeakPtr;
	/**
	 * @brief Constructor
	 * @param[in] src Pointer to an audio data source
	 */
	inline explicit AbstractAudioOutput( const AbstractAudioSource::WeakPtr& src ) : m_source( src ), m_errorCode( NoError ), m_mutex() {
	}
	//! @brief Destructor
	virtual ~AbstractAudioOutput();
	/**
	 * @brief Get the internal error code
	 * @return Internal error code
	 */
	ErrorCode errorCode() const;
	//! @copydoc internal_init
	int init( int desiredFrq );
	//! @copydoc internal_playing
	bool playing() const;
	//! @copydoc internal_paused
	bool paused() const;
	//! @copydoc internal_play
	void play();
	//! @copydoc internal_pause
	void pause();
	//! @copydoc internal_volumeLeft
	uint16_t volumeLeft() const;
	//! @copydoc internal_volumeRight
	uint16_t volumeRight() const;
	/**
	 * @brief Get the attached audio source
	 * @return Pointer to the attached audio source
	 */
	AbstractAudioSource::WeakPtr source() const;
protected:
	/**
	 * @brief Set the internal error code
	 * @param[in] ec New error code
	 */
	void setErrorCode( ErrorCode ec );
	/**
	 * @brief Get the logger
	 * @return Logger with name "audio.output"
	 */
	static light4cxx::Logger* logger();
private:
	AbstractAudioSource::WeakPtr m_source; //!< @brief The audio source
	ErrorCode m_errorCode; //!< @brief Internal error code
	//ReadWriteLockable m_readWriteLock;
	mutable boost::mutex m_mutex;
	/**
	 * @brief Initialize output device
	 * @param[in] desiredFrq Desired output frequency
	 * @retval 0 Initialization failed
	 * @retval other Real output frequency
	 */
	virtual int internal_init( int desiredFrq ) = 0;
	/**
	 * @brief Check if the output is in playing state
	 * @return @c true if the output is in playing state
	 */
	virtual bool internal_playing() const = 0;
	/**
	 * @brief Check if the output is in paused state
	 * @return @c true if the output is in paused state
	 */
	virtual bool internal_paused() const = 0;
	/**
	 * @brief Start playback
	 */
	virtual void internal_play() = 0;
	/**
	 * @brief Pause playback
	 */
	virtual void internal_pause() = 0;
	/**
	 * @brief Get the left channel's volume
	 * @return Left channel's volume, defaults to the source's volume
	 */
	virtual uint16_t internal_volumeLeft() const;
	/**
	 * @brief Get the right channel's volume
	 * @return Right channel's volume, defaults to the source's volume
	 */
	virtual uint16_t internal_volumeRight() const;
};

/**
 * @}
 */

#endif
