#pragma once

/*
    PPPlay - an old-fashioned module player
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

#include "modulestate.h"
#include <output/abstractaudiosource.h>
#include <stuff/trackingcontainer.h>
#include "songinfo.h"
#include "sample.h"

#include <mutex>

namespace ppp
{
struct ChannelState;

/**
 * @ingroup GenMod
 * @{
 */

class OrderEntry;

/**
 * @class GenModule
 * @brief An abstract class for all module classes
 * @todo Create a function to retrieve only the module's title without loading the whole module
 * @todo Multi-song: Reset module/channels on each new song?
 */
class AbstractModule
  : public ISerializable, public AbstractAudioSource
{
public:
  DISABLE_COPY( AbstractModule )

  AbstractModule() = delete;

  typedef std::shared_ptr<AbstractModule> Ptr;

  /**
   * @class MetaInfo
   * @brief Meta information about a module
   */
  struct MetaInfo
  {
    explicit MetaInfo() noexcept
      :
      filename(), title(), trackerInfo()
    {
    }

    //! @brief Filename of the module
    std::string filename;
    //! @brief Title of the module
    std::string title;
    //! @brief Tracker information (Name and Version)
    std::string trackerInfo;
  };

private:
  MetaInfo m_metaInfo;
  //! @brief Order list
  std::vector<std::unique_ptr<OrderEntry>> m_orders;
  ModuleState m_state;
  TrackingContainer<std::unique_ptr<SongInfo>> m_songs;
  //! @brief Maximum module loops if module patterns are played multiple times
  const int m_maxRepeat;
  bool m_isPreprocessing;
  mutable std::recursive_mutex m_mutex;
  Sample::Interpolation m_interpolation;
public:
  //BEGIN Construction/destruction
  /**
   * @name Construction/destruction
   * @{
   */
  /**
   * @brief The constructor
   * @param[in] maxRpt Maximum repeat count for repeating modules
   * @pre @c maxRpt>0
   */
  explicit AbstractModule(int maxRpt, Sample::Interpolation inter = Sample::Interpolation::None);

  /**
   * @brief The destructor
   */
  ~AbstractModule() override;
  /**
   * @}
   */
  //END
  //BEGIN Meta information
  /**
   * @name Meta information
   * @{
   */
  /**
   * @brief Get playback time in seconds for the current song
   * @return Playback time in seconds
   * @see length()
   * @see frequency()
   * @see position()
   */
  uint32_t timeElapsed() const;

  /**
   * @brief Returns the current song's length in sample frames
   * @return The current song's length
   * @see length()
   * @see timeElapsed()
   * @see frequency()
   */
  size_t length() const;

  /**
   * @brief Get the number of songs in this module
   * @return Number of songs
   * @see isMultiSong()
   * @see currentSongIndex()
   */
  size_t songCount() const noexcept;

  /**
   * @brief Get the currently playing song index
   * @return m_currentSongIndex
   * @see isMultiSong()
   * @see songCount()
   */
  size_t currentSongIndex() const;
  /**
   * @}
   */
  //END
  //BEGIN Playback control
  /**
   * @name Playback control
   * @{
   */
  /**
   * @brief Jump to the next song if possible
   * @return @c false if there are no songs left
   */
  bool jumpNextSong();

  /**
   * @brief Jump to the previous song if possible
   * @return @c false if this operation fails
   */
  bool jumpPrevSong();

  /**
   * @brief Jump to the next order if possible
   * @return @c false if the end of the current song is reached
   */
  bool seekForward();

  /**
   * @brief Jump to the previous order if possible
   * @return @c false if the current order is already the first one
   */
  bool seekBackward();

  /**
   * @}
   */
  //END
  MetaInfo metaInfo() const
  {
    return m_metaInfo;
  }

  ModuleState state() const
  {
    return m_state;
  }

  //! @copydoc internal_channelStatus
  ChannelState channelStatus(size_t idx) const;

  //! @copydoc internal_channelCount
  int channelCount() const;

  inline Sample::Interpolation interpolation() const
  {
    return m_interpolation;
  }

  inline void setInterpolation(Sample::Interpolation inter)
  {
    m_interpolation = inter;
  }

protected:
  /**
   * @brief Get the frame count of a tick
   * @return Sample frames per tick
   * @pre m_playbackInfo.tempo != 0
   */
  uint16_t tickBufferLength() const;

  MetaInfo& noConstMetaInfo()
  {
    return m_metaInfo;
  }

  ModuleState& state()
  {
    return m_state;
  }

  /**
   * @copydoc ISerializable::serialize()
   * @details
   * Here's the current procedure used when data serialization is needed.
   * @image html serialization.png
   */
  AbstractArchive& serialize(AbstractArchive* data) override;

  /**
   * @brief Adds an order to m_orders
   * @param[in] o The new order
   */
  void addOrder(std::unique_ptr<OrderEntry>&& o);

  /**
   * @brief Get an order pointer
   * @param[in] idx Index of requested order
   * @return Order pointer
   */
  OrderEntry* orderAt(size_t idx);

  /**
   * @brief Get the number of orders
   * @return Number of orders
   */
  size_t orderCount() const noexcept;

  /**
   * @brief Get the maximum repeat count
   * @return The maximum repeat count
   */
  int maxRepeat() const noexcept;

  /**
   * @brief Set the order index and handle preprocessing states
   * @param[in] newOrder The new order index
   * @param[in] estimateOnly Decide whether to store or to load states
   * @param[in] forceSave Forces to save the state even if the order did not change
   * @retval true if the new order index is valid
   */
  bool setOrder(size_t newOrder);

  /**
   * @brief Set the current row index
   * @param[in] r The new row index
   * @retval false when the new row has been played more than 255 times
   */
  void setRow(int16_t r) noexcept;

  /**
   * @brief Increase the current tick index
   * @pre m_playbackInfo.speed > 0
   * @post m_tick < m_playbackInfo.speed
   */
  void nextTick();

  /**
   * @brief Sets the current tempo
   * @param[in] t The new tempo
   */
  void setTempo(uint8_t t) noexcept;

  /**
   * @brief Sets the current speed
   * @param[in] s The new speed
   */
  void setSpeed(uint8_t s) noexcept;

  /**
   * @brief Get the logger
   * @return Logger with name "module"
   */
  static light4cxx::Logger* logger();

private:
  //! @copydoc internal_buildTick
  size_t buildTick(const AudioFrameBufferPtr& buffer);

  bool internal_initialize(uint32_t frq) override final;

  /**
   * @brief Returns the channel status string for a channel
   * @param[in] idx Requested channel
   * @return Status string
   */
  virtual ChannelState internal_channelStatus(size_t idx) const = 0;

  /**
   * @brief Get the number of actually used channels
   * @return Number of actually used channels
   */
  virtual int internal_channelCount() const = 0;

  /**
   * @brief Get a tick
   * @param[out] buffer Pointer to the destination buffer or @c NULL to to only length estimation
   * @return Length of the current tick, or 0 when end of song is reached.
   * @note Time-critical
   * @see AbstractChannel::mixTick()
   *
   * @details
   * When @a buf is @c NULL, the implementation and its callees shall only execute
   * code/effects that are necessary to estimate the length of the current tick, such
   * as:
   * @li speed and tempo changes
   * @li pattern loops and delays
   */
  virtual size_t internal_buildTick(const AudioFrameBufferPtr& buffer) = 0;

  /**
   * @copydoc IAudioSource::getAudioData()
   * @note The buffer will contain full ticks, so the buffer will generally
   * have a different size as requested with @a requestedFrames.
   */
  size_t internal_getAudioData(AudioFrameBufferPtr& buffer, size_t requestedFrames) override;

  /**
   * @copydoc IAudioSource::preferredBufferSize()
   * @note This should generally return tickBufferLength()
   */
  size_t internal_preferredBufferSize() const override;
};

/**
 * @}
 */
} // namespace ppp
