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

#ifndef GENMODULE_H
#define GENMODULE_H

/**
 * @ingroup GenMod
 * @{
 */

#include "genorder.h"
#include "stream/stateiterator.h"
#include "output/iaudiosource.h"

namespace ppp {

/**
 * @struct GenPlaybackInfo
 * @brief Playback info
 */
struct GenPlaybackInfo {
	int16_t tick; //!< @brief Current tick
	int16_t order; //!< @brief Current Order
	int16_t pattern; //!< @brief Current pattern
	int16_t row; //!< @brief Current row
	int16_t speed; //!< @brief Current speed
	int16_t tempo; //!< @brief Current tempo
	int16_t globalVolume; //!< @brief Current global volume
};

/**
 * @class GenModule
 * @brief An abstract class for all module classes
 * @todo Create a function to retrieve only the module's title without loading the whole module
 * @todo Multi-song: Reset module/channels on each new song?
 */
class GenModule : public ISerializable, public IAudioSource {
		DISABLE_COPY(GenModule)
		GenModule() = delete;
	public:
		typedef std::shared_ptr<GenModule> Ptr; //!< @brief Class pointer
	private:
		std::string m_filename; //!< @brief Filename of the loaded module, empty if none loaded
		std::string m_title; //!< @brief Title of the module
		std::string m_trackerInfo; //!< @brief Tracker information (Name and Version)
		GenOrder::Vector m_orders; //!< @brief Order list @note @b Not @b initialized @b here!
		uint16_t m_maxRepeat; //!< @brief Maximum module loops if module patterns are played multiple times
		size_t m_playedFrames; //!< @brief Played Sample frames
		std::vector<StateIterator> m_songs; //!< @brief Per-song infos
		std::vector<size_t> m_songLengths; //!< @brief Per-song lengths in sample frames
		uint16_t m_currentSongIndex; //!< @brief The current song index
		GenPlaybackInfo m_playbackInfo; //!< @brief General playback info
	public:
		/**
		 * @brief The constructor
		 * @param[in] maxRpt Maximum repeat count for repeating modules
		 * @pre @c maxRpt>0
		 */
		GenModule(uint8_t maxRpt);
		/**
		 * @brief The destructor
		 */
		virtual ~GenModule();
		/**
		 * @brief Returns the filename without the path
		 * @return Filename, or empty if no module loaded
		 */
		std::string filename();
		/**
		 * @brief Returns the title
		 * @return The title of the module
		 */
		std::string title() const;
		/**
		 * @brief Returns the title without left and right whitespaces
		 * @return The trimmed title of the module
		 */
		std::string trimmedTitle() const;
		/**
		 * @brief Get the frame count of a tick
		 * @return Sample frames per tick
		 */
		uint16_t tickBufferLength() const;
		/**
		 * @brief Get a tick
		 * @param[out] buf Reference to the destination buffer
		 * @note Time-critical
		 */
		virtual void buildTick(AudioFrameBuffer& buf) = 0;
		/**
		 * @brief Get a tick without mixing for length calculation
		 * @param[out] bufLen Number of sample frames in the current tick. If 0 after the call, the end was reached.
		 */
		virtual void simulateTick(size_t& bufLen) = 0;
		/**
		 * @brief Map an order number to a pattern
		 * @param[in] order The order to map
		 * @return Pattern number for @a order
		 */
		virtual GenOrder::Ptr mapOrder(int16_t order) = 0;
		/**
		 * @brief Returns the channel status string for a channel
		 * @param[in] idx Requested channel
		 * @return Status string
		 */
		virtual std::string channelStatus(int16_t idx) = 0;
		/**
		 * @brief Get playback time in seconds
		 * @return Playback time in seconds
		 */
		uint32_t timeElapsed() const;
		/**
		 * @brief Returns the current song's length
		 * @return The current song's length
		 * @see timeElapsed()
		 */
		size_t length() const;
		/**
		 * @brief Get information about the tracker
		 * @return Tracker type and version, i.e. "ScreamTracker v3.20"
		 */
		std::string trackerInfo() const;
		/**
		 * @brief Get playback information
		 * @return m_playbackInfo
		 */
		GenPlaybackInfo playbackInfo() const;
		/**
		 * @brief Returns @c true if this module contains multiple songs
		 * @return @c true if this is a multi-song
		 */
		bool isMultiSong() const;
		/**
		 * @brief Jump to the next song if possible
		 * @return @c false if there are no songs left
		 */
		virtual bool jumpNextSong() = 0;
		/**
		 * @brief Jump to the previous song if possible
		 * @return @c false if this operation fails
		 */
		virtual bool jumpPrevSong() = 0;
		/**
		 * @brief Jump to the next order if possible
		 * @return @c false if the end of the current song is reached
		 */
		virtual bool jumpNextOrder() = 0;
		/**
		 * @brief Jump to the previous order if possible
		 * @return @c false if the current order is already the first one
		 */
		virtual bool jumpPrevOrder() = 0;
		/**
		 * @brief Get the current playback position in sample frames
		 * @return m_playedFrames
		 */
		size_t position() const;
		/**
		 * @brief Get the number of songs in this module
		 * @return Number of songs
		 */
		uint16_t songCount() const;
		/**
		 * @brief Get the currently playing song index
		 * @return m_currentSongIndex
		 */
		uint16_t currentSongIndex() const;
		/**
		 * @brief Get the channel cell string
		 * @param[in] idx Channel index
		 * @return String representation of the channel's cell
		 * @see GenChannel::getCellString
		 */
		virtual std::string channelCellString(int16_t idx) = 0;
		/**
		 * @brief Get the number of actually used channels
		 * @return Number of actually used channels
		 */
		virtual uint8_t channelCount() const = 0;
		virtual size_t getAudioData(AudioFrameBuffer& buffer, size_t size);
		/**
		 * @brief Set the global volume
		 * @param[in] v The new global volume
		 */
		virtual void setGlobalVolume(int16_t v);
	protected:
		virtual IArchive& serialize(IArchive* data);
		/**
		 * @brief Removes empty songs from the song list
		 */
		void removeEmptySongs();
		/**
		 * @brief Set the song's position in sample frames
		 * @param[in] p The new position
		 */
		void setPosition(size_t p);
		/**
		 * @brief Adds an order to m_orders
		 * @param[in] o The new order
		 */
		void addOrder(const GenOrder::Ptr& o);
		/**
		 * @brief Get the filename
		 * @return m_filename
		 */
		std::string filename() const;
		/**
		 * @brief Set the filename
		 * @param[in] f The new filename
		 */
		void setFilename(const std::string& f);
		/**
		 * @brief Set the tracker info
		 * @param[in] t The new tracker info
		 */
		void setTrackerInfo(const std::string& t);
		/**
		 * @brief Get an order pointer
		 * @param[in] idx Index of requested order
		 * @return Order pointer
		 */
		GenOrder::Ptr orderAt(size_t idx) const;
		/**
		 * @brief Get the current pattern index
		 * @return Current pattern index
		 */
		int16_t patternIndex() const;
		/**
		 * @brief Set the current pattern index
		 * @param[in] i The new pattern index
		 */
		void setPatternIndex(int16_t i);
		/**
		 * @brief Get the number of orders
		 * @return Number of orders
		 */
		size_t orderCount() const;
		/**
		 * @brief Set the current song index
		 * @param[in] t The new song index
		 */
		void setCurrentSongIndex(uint16_t t);
		/**
		 * @brief Set the module's title
		 * @param[in] t The new title
		 */
		void setTitle(const std::string& t);
		/**
		 * @brief Get the state iterator for a song
		 * @param[in] idx Song index
		 * @return Reference to the state iterator
		 */
		StateIterator& multiSongAt(size_t idx);
		/**
		 * @brief Get the length of a song
		 * @param[in] idx Song index
		 * @return Reference to the length in sample frames
		 */
		size_t& multiSongLengthAt(size_t idx);
		/**
		 * @brief Add a multi-song state iterator
		 * @param[in] t The new state iterator
		 */
		void addMultiSong(const StateIterator& t);
		/**
		 * @brief Get the maximum repeat count
		 * @return The maximum repeat count
		 */
		uint16_t maxRepeat() const;
		/**
		 * @brief Set the order index
		 * @param[in] o The new order index
		 */
		void setOrder(int16_t o);
		/**
		 * @brief Set the current row index
		 * @param[in] r The new row index
		 */
		void setRow(int16_t r);
		/**
		 * @brief Increase the current tick index
		 */
		void nextTick();
	public:
		/**
		 * @brief Sets the current tempo
		 * @param[in] t The new tempo
		 */
		void setTempo(uint8_t t);
		/**
		 * @brief Sets the current speed
		 * @param[in] s The new speed
		 */
		void setSpeed(uint8_t s);
		/**
		 * @brief Get the current tick index
		 * @return The current tick index
		 */
		uint8_t tick() const;
};

} // namespace ppp

/**
 * @}
 */

#endif

