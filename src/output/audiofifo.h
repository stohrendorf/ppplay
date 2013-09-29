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

#include "ppplay_core_export.h"

#include <stuff/utils.h>
#include "audiotypes.h"
#include "abstractaudiosource.h"

#include <light4cxx/logger.h>
#include <boost/circular_buffer.hpp>
#include <boost/signals2.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>

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
 * AbstractAudioSource.
 */
class PPPLAY_CORE_EXPORT AudioFifo
{
    DISABLE_COPY( AudioFifo )
    AudioFifo() = delete;
private:
    //! @brief Buffered audio frames
    boost::circular_buffer<BasicSampleFrame> m_buffer;
    //! @brief Threshold to tell when the buffer needs data
    typename boost::circular_buffer<BasicSampleFrame>::size_type m_threshold;
    //! @brief The requester thread that pulls the audio data from the source
    std::thread m_requestThread;
    //! @brief The audio source to pull the data from
    AbstractAudioSource::WeakPtr m_source;

    //! @brief @c true when the destructor is running, stops the thread
    bool m_stopping;
    //! @brief Buffer modification mutex
    mutable std::mutex m_bufferMutex;
    //! @brief Buffer modification notifier
    std::condition_variable m_bufferChanged;

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
    void pushData( const AudioFrameBuffer& buf );

public:
    /**
     * @brief Initialize the buffer
     * @param[in] source The audio source that should be buffered
     * @param[in] threshold Initial value for m_threshold (minimum 256)
     */
    AudioFifo( const AbstractAudioSource::WeakPtr& source, size_t threshold );
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
    size_t capacity() const;

    /**
     * @brief Set the FIFO buffer length
     * @param[in] len The requested buffer length (minimum 256)
     */
    void setCapacity( size_t len );

    /**
     * @brief Check if the FIFO is empty
     * @retval true FIFO is empty
     * @retval false FIFO is not empty
     */
    bool isEmpty() const;

    size_t pullData( AudioFrameBuffer& buffer, size_t requestedFrames );

    // These are needed to replace boost::mutex (or boost::signals2::mutex) with std::mutex,
    // otherwise boost::thread would become a dependency.
    typedef void DataSignalSignature( const AudioFrameBuffer& );
    typedef boost::signals2::signal <
        DataSignalSignature,
        boost::signals2::optional_last_value<void>,
        int,
        std::less<int>,
        boost::function<DataSignalSignature>,
        typename boost::signals2::detail::extended_signature<0, DataSignalSignature>::function_type,
        std::mutex
        > DataSignal;

    DataSignal dataPushed;
    DataSignal dataPulled;
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
