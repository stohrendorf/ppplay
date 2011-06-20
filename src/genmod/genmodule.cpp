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

#include "genbase.h"
#include "genmodule.h"
#include "stream/memarchive.h"

/**
* @file
* @ingroup GenMod
* @brief General/common module definitions
*/

namespace ppp {

IArchive::Ptr GenMultiTrack::newState() {
	IArchive::Ptr p(new MemArchive());
	LOG_MESSAGE("Creating new state %u", m_states.size());
	m_states.push_back(p);
	return p;
}

IArchive::Ptr GenMultiTrack::nextState() {
	if(m_stateIndex >= m_states.size() - 1) {
		LOG_ERROR("%d >= %d - 1", m_stateIndex, m_states.size());
		return IArchive::Ptr();
	}
	m_stateIndex++;
	LOG_MESSAGE("Loading state %u", m_stateIndex);
	IArchive::Ptr result = m_states[m_stateIndex];
	return result;
}

IArchive::Ptr GenMultiTrack::prevState() {
	if(m_stateIndex <= 1) {
		LOG_ERROR("m_stateIndex <= 1");
		return IArchive::Ptr();
	}
	m_stateIndex--;
	LOG_MESSAGE("Loading state %u", m_stateIndex);
	IArchive::Ptr result = m_states[m_stateIndex];
	return result;
}

GenModule::GenModule(uint8_t maxRpt) :
	m_fileName(), m_title(), m_trackerInfo(), m_orders(), m_maxRepeat(maxRpt),
	m_playedFrames(0), m_tracks(),
	m_currentTrack(0), m_multiTrack(false), m_playbackInfo() {
	PPP_TEST(maxRpt == 0);
	m_playbackInfo.tick = m_playbackInfo.order = m_playbackInfo.pattern = 0;
	m_playbackInfo.row = m_playbackInfo.speed = m_playbackInfo.tempo = 0;
	m_playbackInfo.globalVolume = 0x40;
	GenMultiTrack nulltrack;
	m_tracks.push_back(nulltrack);
}

GenModule::~GenModule() {
}

IArchive& GenModule::serialize(IArchive* data) {
	data->array(reinterpret_cast<char*>(&m_playbackInfo), sizeof(m_playbackInfo)) & m_playedFrames& m_currentTrack;
	/*	for ( uint_fast16_t i = 0; i < m_orders.size(); i++ ) {
			if ( !m_orders[i] )
				continue;
			data->archive(m_orders[i].get());
		}*/
	return *data;
}

std::string GenModule::filename() {
	try {
		std::size_t lastPos = m_fileName.find_last_of("/\\");
		if(lastPos != std::string::npos)
			return m_fileName.substr(lastPos + 1);
		return m_fileName;
	}
	PPP_CATCH_ALL();
}

std::string GenModule::title() const {
	return m_title;
}

std::string GenModule::trimmedTitle() const {
	return trimString(m_title);
}

uint32_t GenModule::timeElapsed() const {
	PPP_TEST(frequency() == 0);
	return static_cast<uint32_t>(m_playedFrames / frequency());
}

uint32_t GenModule::length() const {
	return m_tracks[m_currentTrack].length;
}

std::string GenModule::trackerInfo() const {
	return m_trackerInfo;
}

GenPlaybackInfo GenModule::playbackInfo() const {
	return m_playbackInfo;
}

bool GenModule::isMultiTrack() const {
	return m_multiTrack;
}

void GenModule::removeEmptyTracks() {
	std::vector<GenMultiTrack> nTr;
	std::for_each(
	    m_tracks.begin(), m_tracks.end(),
	[&nTr](const GenMultiTrack & mt) {
		if(mt.length != 0 && mt.startOrder != GenMultiTrack::stopHere) nTr.push_back(mt);
	}
	);
	/*	for(std::vector<GenMultiTrack>::iterator it = m_tracks.begin(); it!=m_tracks.end(); it++) {
			if (( it->length != 0 ) && ( it->startOrder != GenMultiTrack::stopHere ) )
				nTr.push_back( *it );
		}*/
	m_tracks = nTr;
	m_multiTrack = trackCount() > 1;
}

std::size_t GenModule::getAudioData(AudioFrameBuffer& buffer, std::size_t size) {
	if(!buffer)
		buffer.reset(new AudioFrameBuffer::element_type);
	while(buffer->size() < size) {
		AudioFrameBuffer tmpBuf;
		buildTick(tmpBuf);
		if(!tmpBuf || tmpBuf->size() == 0)
			return 0;
		buffer->insert(buffer->end(), tmpBuf->begin(), tmpBuf->end());
	}
	return buffer->size();
}

IArchive::Vector GenMultiTrack::states() const {
	return m_states;
}

std::size_t GenMultiTrack::stateIndex() const {
	return m_stateIndex;
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
	return m_fileName;
}

void GenModule::setFilename(const std::string& f) {
	m_fileName = f;
}

void GenModule::setTrackerInfo(const std::string& t) {
	m_trackerInfo = t;
}

GenOrder::Ptr GenModule::orderAt(size_t idx) const {
	return m_orders[idx];
}

int16_t GenModule::patternIndex() const {
	return m_playbackInfo.pattern;
}

void GenModule::setPatternIndex(int16_t i) {
	m_playbackInfo.pattern = i;
}

int GenModule::orderCount() const {
	return m_orders.size();
}

void GenModule::setCurrentTrack(uint16_t t) {
	m_currentTrack = t;
}

void GenModule::setTitle(const std::string& t) {
	m_title = t;
}

void GenModule::setMultiTrack(bool m) {
	m_multiTrack = m;
}

GenMultiTrack& GenModule::multiTrackAt(size_t idx) {
	return m_tracks[idx];
}

void GenModule::addMultiTrack(const GenMultiTrack& t) {
	m_tracks.push_back(t);
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

uint16_t GenModule::trackCount() const {
	return m_tracks.size();
}

uint16_t GenModule::currentTrackIndex() const {
	return m_currentTrack;
}

}
