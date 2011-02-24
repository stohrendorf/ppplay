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

#ifndef GENMODULE_H
#define GENMODULE_H

#include "genbase.h"
#include "genchannel.h"

#include "output/iaudiosource.h"

/**
 * @file
 * @ingroup GenMod
 * @brief General/common module definitions
 * @note Volumes are from 0x00 to 0x40
 * @note Panning values are from 0x00 to 0x80, where 0x40
 * is the center
 * @note A panning value of 0xa4 indicates surround sound
 *
 * @details
 * Structure is as follows: ::GenPattern contains ::GenTrack's contains
 * ::GenCell's. @n
 * A Track is a column within a Pattern, whereas a Cell is a Row
 * within a Track. @n
 * This structure is caused by the simple fact that a row is accessed
 * through the tracks, and a track is accessed through the patterns.
 */

namespace ppp {

	/**
	 * @struct GenPlaybackInfo
	 * @ingroup GenMod
	 * @brief Playback info
	 */
	typedef struct {
		int16_t tick; //!< @brief Current tick
		int16_t order; //!< @brief Current Order
		int16_t pattern; //!< @brief Current pattern
		int16_t row; //!< @brief Current row
		int16_t speed; //!< @brief Current speed
		int16_t tempo; //!< @brief Current tempo
		int16_t globalVolume; //!< @brief Current global volume
	} GenPlaybackInfo;

	/**
	 * @class GenMultiTrack
	 * @ingroup GenMod
	 * @brief Storage class for multitracks
	 */
	class GenMultiTrack {
		public:
			typedef std::shared_ptr<GenMultiTrack> Ptr; //!< @brief Class pointer
		private:
			IArchive::Vector m_states;
			std::size_t m_stateIndex;
		public:
			std::size_t length; //!< @brief Track length in sample frames
			uint16_t startOrder; //!< @brief Start order of this track
			/**
			 * @brief Constructor
			 */
			GenMultiTrack() : m_states(), m_stateIndex(0), length(0), startOrder(0) {
				m_states.resize(1);
			}
			static const uint16_t stopHere = ~0; //!< @brief Const to define unused track
			IArchive* newState();
			IArchive::Vector states() const { return m_states; }
			std::size_t stateIndex() const { return m_stateIndex; }
			IArchive* nextState();
			IArchive* prevState();
	};

	/**
	 * @class GenModule
	 * @ingroup GenMod
	 * @brief An abstract class for all module classes
	 * @todo Create a function to retrieve only the module's title without loading the whole module
	 * @todo Multi-track: Reset module/channels on each new track?
	 * @todo Multi-track: Get length of each track, not only the first one
	 */
	class GenModule : public ISerializable, public IAudioSource {
			DISABLE_COPY(GenModule)
			GenModule() = delete;
		public:
			typedef std::shared_ptr<GenModule> Ptr; //!< @brief Class pointer
		private:
			std::string m_fileName; //!< @brief Filename of the loaded module, empty if none loaded
			std::string m_title; //!< @brief Title of the module
			std::string m_trackerInfo; //!< @brief Tracker information (Name and Version)
			GenOrder::Vector m_orders; //!< @brief Order list @note @b Not @b initialized @b here!
			uint16_t m_maxRepeat; //!< @brief Maximum module loops if module patterns are played multiple times
			uint16_t m_playbackFrequency; //!< @brief Playback frequency
			std::size_t m_playedFrames; //!< @brief Played Sample frames
			std::vector<GenMultiTrack> m_tracks; //!< @brief Per-track infos
			uint16_t m_currentTrack; //!< @brief The current track index
			bool m_multiTrack; //!< @brief @c true if module could be a multi-track one
			GenPlaybackInfo m_playbackInfo; //!< @brief General playback info
		public:
			/**
			 * @brief The constructor
			 * @param[in] frq Playback frequency, clipped to a value between 11025 and 44800
			 * @param[in] maxRpt Maximum repeat count for repeating modules
			 * @pre @c maxRpt>0
			 * @see GenChannel::GenChannel
			 */
			GenModule(uint32_t frq = 44100, uint8_t maxRpt = 1) throw(PppException);
			/**
			 * @brief The destructor
			 */
			virtual ~GenModule();
			/**
			 * @brief Returns the filename without the path
			 * @return Filename, or empty if no module loaded
			 * @todo Implement OS-independent filename splitting
			 */
			std::string getFileName() throw(PppException);
			/**
			 * @brief Returns the title
			 * @return The title of the module
			 */
			std::string getTitle() const throw();
			/**
			 * @brief Returns the title without left and right spaces
			 * @return The trimmed title of the module
			 */
			std::string getTrimTitle() const throw();
			/**
			 * @brief Get the frame count of a tick
			 * @return Sample frames per tick
			 */
			virtual uint16_t getTickBufLen() const throw(PppException) = 0;
			/**
			 * @brief Get a tick
			 * @param[out] buf Reference to the destination buffer
			 * @note Time-critical
			 */
			virtual void getTick(AudioFrameBuffer &buf) throw(PppException) = 0;
			/**
			 * @brief Get a tick without mixing for length calculation
			 * @param[out] bufLen Number of sample frames in the current tick
			 */
			virtual void getTickNoMixing(std::size_t &bufLen) throw(PppException) = 0;
			/**
			 * @brief Map an order number to a pattern
			 * @param[in] order The order to map
			 * @return Pattern number for @a order
			 */
			virtual GenOrder::Ptr mapOrder(int16_t order) throw(PppException) = 0;
			/**
			 * @brief Returns the channel status string for a channel
			 * @param[in] idx Requested channel
			 */
			virtual std::string getChanStatus(int16_t idx) throw() = 0;
			/**
			 * @brief Get playback time in seconds
			 * @return Playback time in seconds
			 */
			uint32_t timeElapsed() const throw(PppException);
			/**
			 * @brief Returns the current track's length
			 * @return The current track's length
			 * @see timeElapsed()
			 */
			uint32_t getLength() const throw();
			/**
			 * @brief Get information about the tracker
			 * @return Tracker type and version, i.e. "ScreamTracker v3.20"
			 */
			std::string getTrackerInfo() const throw();
			/**
			 * @brief Get playback information
			 * @return m_playbackInfo
			 */
			GenPlaybackInfo getPlaybackInfo() const throw();
			/**
			 * @brief Returns @c true if this module contains additional tracks
			 * @return m_multiTrack
			 */
			bool isMultiTrack() const throw();
			/**
			 * @brief Jump to the next track if possible
			 * @return @c false if there are no tracks left
			 */
			virtual bool jumpNextTrack() throw(PppException) = 0;
			/**
			 * @brief Jump to the previous track if possible
			 * @return @c false if this operation fails
			 */
			virtual bool jumpPrevTrack() throw(PppException) = 0;
			/**
			 * @brief Jump to the next order if possible
			 * @return @c false if the end of the current track is reached
			 */
			virtual bool jumpNextOrder() throw() = 0;
			/**
			 * @brief Jump to the previous order if possible
			 * @return @c false if the current order is already the first one
			 */
			virtual bool jumpPrevOrder() throw() = 0;
			/**
			 * @brief Get the current playback position in sample frames
			 * @return m_playedFrames
			 */
			std::size_t getPosition() const throw() { return m_playedFrames; }
			/**
			 * @brief Get the number of tracks in this module
			 * @return Number of tracks
			 */
			uint16_t getTrackCount() const throw() { return m_tracks.size(); }
			/**
			 * @brief Get the currently playing track index
			 * @return m_currentTrack
			 */
			uint16_t getCurrentTrack() const throw() { return m_currentTrack; }
			/**
			 * @brief Get the channel cell string
			 * @param[in] idx Channel index
			 * @return String representation of the channel's cell
			 * @see GenChannel::getCellString
			 */
			virtual std::string getChanCellString(int16_t idx) throw() = 0;
			/**
			 * @brief Get the number of actually used channels
			 * @return Number of actually used channels
			 */
			virtual uint8_t channelCount() const = 0;
			virtual std::size_t getAudioData(AudioFrameBuffer& buffer, std::size_t size);
		protected:
			/**
			 * @brief Serialize this module to an archive
			 * @param[in,out] data The archive to serialize to
			 * @return Reference to @c *data
			 * @pre @c data!=NULL
			 */
			virtual IArchive& serialize(IArchive* data);
			/**
			 * @brief Removes empty tracks from the track list and resets the orders' repeat count
			 */
			void removeEmptyTracks();
			void setPlaybackFrq(uint16_t f) throw() { m_playbackFrequency = f; }
			uint16_t getPlaybackFrq() const throw() { return m_playbackFrequency; }
			void setPosition(std::size_t p) throw() { m_playedFrames = p; }
			void addOrder(const GenOrder::Ptr &o) { m_orders.push_back(o); }
			std::string getFilename() const { return m_fileName; }
			void setFilename(const std::string &f) { m_fileName = f; }
			void setTrackerInfo(const std::string &t) { m_trackerInfo = t; }
			GenOrder::Ptr getOrder(size_t idx) const { return m_orders[idx]; }
			int16_t getPatternIndex() const { return m_playbackInfo.pattern; }
			void setPatternIndex(int16_t i) { m_playbackInfo.pattern=i; }
			int getOrderCount() const { return m_orders.size(); }
			void setCurrentTrack(uint16_t t) { m_currentTrack=t; }
			void setTitle(const std::string &t) { m_title = t; }
			void setMultiTrack(bool m) { m_multiTrack = m; }
			GenMultiTrack &getMultiTrack(size_t idx) { return m_tracks[idx]; }
			void addMultiTrack(const GenMultiTrack &t) { m_tracks.push_back(t); }
			uint16_t getMaxRepeat() const { return m_maxRepeat; }
			void setSpeed(uint8_t s) { if(s==0) return; m_playbackInfo.speed=s; }
			void setTempo(uint8_t t) { if(t==0) return; m_playbackInfo.tempo=t; }
			void setOrder(int16_t o) { m_playbackInfo.order=o; }
			void setRow(int16_t r) { m_playbackInfo.row=r; }
			void nextTick() { m_playbackInfo.tick = (m_playbackInfo.tick+1) % m_playbackInfo.speed; }
			void setGlobalVolume(int16_t v) { m_playbackInfo.globalVolume=v; }
	};
} // namespace ppp

#endif
