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

/**
 * @ingroup GenMod
 * @{
 */

#include "genbase.h"
#include "genmodule.h"
#include "stream/memarchive.h"
#include "logger/logger.h"

#include <boost/filesystem.hpp>

namespace ppp {

GenModule::GenModule(uint8_t maxRpt) :
	m_filename(), m_title(), m_trackerInfo(), m_orders(), m_maxRepeat(maxRpt),
	m_playedFrames(0), m_songs(), m_songLengths(),
	m_currentSongIndex(0), m_playbackInfo() {
	BOOST_ASSERT(maxRpt != 0);
	m_playbackInfo.tick = m_playbackInfo.order = m_playbackInfo.pattern = 0;
	m_playbackInfo.row = m_playbackInfo.speed = m_playbackInfo.tempo = 0;
	m_playbackInfo.globalVolume = 0x40;
}

GenModule::~GenModule() = default;

IArchive& GenModule::serialize(IArchive* data) {
	data->array(reinterpret_cast<char*>(&m_playbackInfo), sizeof(m_playbackInfo)) % m_playedFrames; // % m_currentSongIndex;
	return *data;
}

std::string GenModule::filename() {
	boost::filesystem::path path(m_filename);
	BOOST_ASSERT( path.has_filename() );
	return path.filename().native();
}

std::string GenModule::title() const {
	return m_title;
}

std::string GenModule::trimmedTitle() const {
	return trimString(m_title);
}

uint32_t GenModule::timeElapsed() const {
	BOOST_ASSERT(frequency() != 0);
	return static_cast<uint32_t>(m_playedFrames / frequency());
}

std::size_t GenModule::length() const {
	return m_songLengths.at(m_currentSongIndex);
}

std::string GenModule::trackerInfo() const {
	return m_trackerInfo;
}

GenPlaybackInfo GenModule::playbackInfo() const {
	return m_playbackInfo;
}

bool GenModule::isMultiSong() const {
	return m_songs.size()>1;
}

void GenModule::removeEmptySongs() {
	LOG_MESSAGE("Removing empty songs");
	std::vector<StateIterator> nTr;
	std::vector<std::size_t> nTrLen;
	for(std::size_t i = 0; i < m_songs.size(); i++) {
		if(m_songLengths.at(i) != 0) {
			nTr.push_back(m_songs.at(i));
			nTrLen.push_back(m_songLengths.at(i));
		}
	}
	m_songs = nTr;
	m_songLengths = nTrLen;
	m_currentSongIndex = 0;
}

std::size_t GenModule::getAudioData(AudioFrameBuffer& buffer, std::size_t size) {
	IAudioSource::LockGuard guard(this);
	if(!buffer) {
		buffer.reset(new AudioFrameBuffer::element_type);
	}
	while(buffer->size() < size) {
		AudioFrameBuffer tmpBuf;
		buildTick(tmpBuf);
		if(!tmpBuf || tmpBuf->size() == 0)
			return 0;
		buffer->insert(buffer->end(), tmpBuf->begin(), tmpBuf->end());
	}
	return buffer->size();
}

void GenModule::setGlobalVolume(int16_t v) {
	m_playbackInfo.globalVolume = v;
}

void GenModule::setPosition(size_t p) {
	m_playedFrames = p;
}

void GenModule::addOrder(const GenOrder::Ptr& o) {
	m_orders.push_back(o);
}

std::string GenModule::filename() const {
	return m_filename;
}

void GenModule::setFilename(const std::string& f) {
	m_filename = f;
}

void GenModule::setTrackerInfo(const std::string& t) {
	m_trackerInfo = t;
}

GenOrder::Ptr GenModule::orderAt(size_t idx) const {
	return m_orders.at(idx);
}

int16_t GenModule::patternIndex() const {
	return m_playbackInfo.pattern;
}

void GenModule::setPatternIndex(int16_t i) {
	m_playbackInfo.pattern = i;
}

std::size_t GenModule::orderCount() const {
	return m_orders.size();
}

void GenModule::setCurrentSongIndex(uint16_t t) {
	m_currentSongIndex = t;
}

void GenModule::setTitle(const std::string& t) {
	m_title = t;
}

StateIterator& GenModule::multiSongAt(std::size_t idx) {
	return m_songs.at(idx);
}

std::size_t& GenModule::multiSongLengthAt(std::size_t idx) {
	return m_songLengths.at(idx);
}

void GenModule::addMultiSong(const StateIterator& t) {
	m_songs.push_back(t);
	m_songLengths.push_back(0);
}

uint16_t GenModule::maxRepeat() const {
	return m_maxRepeat;
}

void GenModule::setOrder(int16_t o) {
	m_playbackInfo.order = o;
}

void GenModule::setRow(int16_t r) {
	m_playbackInfo.row = r;
}

void GenModule::nextTick() {
	m_playbackInfo.tick = (m_playbackInfo.tick + 1) % m_playbackInfo.speed;
}

void GenModule::setTempo(uint8_t t) {
	if(t == 0) return;
	m_playbackInfo.tempo = t;
}

void GenModule::setSpeed(uint8_t s) {
	if(s == 0) return;
	m_playbackInfo.speed = s;
}

size_t GenModule::position() const {
	return m_playedFrames;
}

uint16_t GenModule::songCount() const {
	return m_songs.size();
}

uint16_t GenModule::currentSongIndex() const {
	return m_currentSongIndex;
}

}

/**
 * @}
 */
