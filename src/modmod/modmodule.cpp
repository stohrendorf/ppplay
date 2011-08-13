#include "modmodule.h"

#include "stream/iarchive.h"
#include "stream/fbinstream.h"
#include "logger/logger.h"

#include <boost/exception/all.hpp>

namespace ppp {
namespace mod {

GenModule::Ptr ModModule::factory(const std::string& filename, uint32_t frequency, uint8_t maxRpt)
{
	ModModule::Ptr result(new ModModule(maxRpt));
	if(!result->load(filename)) {
		return GenModule::Ptr();
	}
	if(!result->initialize(frequency)) {
		return GenModule::Ptr();
	}
	return result;
}

ModModule::ModModule(uint8_t maxRpt): GenModule(maxRpt),
	m_samples(), m_patterns(), m_channels(), m_patLoopRow(-1),
	m_patLoopCount(-1), m_breakRow(-1), m_patDelayCount(-1), m_breakOrder(-1)
{
}

ModModule::~ModModule() = default;

ModSample::Ptr ModModule::sampleAt(size_t idx) const
{
	if(idx==0) {
		return ModSample::Ptr();
	}
	idx--;
	if(idx>=m_samples.size()) {
		return ModSample::Ptr();
	}
	return m_samples.at(idx);
}

/**
 * @brief Maps module IDs to their respective channel counts
 */
struct IdMetaInfo {
	const std::string id;
	const uint8_t channels;
	const std::string tracker;
};
 
static const std::array<IdMetaInfo, 31> idMetaData  = {{
	{"M.K.", 4, "ProTracker"},
	{"M!K!", 4, "ProTracker"},
	{"FLT4", 4, "Startrekker"},
	{"FLT8", 8, "Startrekker"},
	{"CD81", 8, "Falcon"}, //< @todo Check tracker name
	{"TDZ1", 1, "TakeTracker"},
	{"TDZ2", 2, "TakeTracker"},
	{"TDZ3", 3, "TakeTracker"},
	{"5CHN", 1, "TakeTracker"},
	{"7CHN", 1, "TakeTracker"},
	{"9CHN", 1, "TakeTracker"},
	{"11CH", 11, "TakeTracker"},
	{"13CH", 13, "TakeTracker"},
	{"15CH", 15, "TakeTracker"},
	{"2CHN", 2, "FastTracker"},
	{"4CHN", 4, "FastTracker"},
	{"6CHN", 6, "FastTracker"},
	{"8CHN", 8, "FastTracker"},
	{"10CH", 10, "FastTracker"},
	{"12CH", 12, "FastTracker"},
	{"14CH", 14, "FastTracker"},
	{"16CH", 16, "FastTracker"},
	{"18CH", 18, "FastTracker"},
	{"20CH", 20, "FastTracker"},
	{"22CH", 22, "FastTracker"},
	{"24CH", 24, "FastTracker"},
	{"26CH", 26, "FastTracker"},
	{"28CH", 28, "FastTracker"},
	{"30CH", 30, "FastTracker"},
	{"32CH", 32, "FastTracker"},
	{"OCTA", 8, "Octalyzer"}, //< @todo Check tracker name
}};

static IdMetaInfo findMeta(BinStream* stream) {
	static const IdMetaInfo none = {"", 0, ""};
	char id[5];
	stream->read(id, 4);
	id[4] = '\0';
	for(const IdMetaInfo& mi : idMetaData) {
		if(id == mi.id) {
			return mi;
		}
	}
	// revert...
	stream->seekrel(-4);
	return none;
}

bool ModModule::load(const std::string& filename)
{
	LOG_MESSAGE("Opening '%s'", filename.c_str());
	FBinStream stream(filename);
	if(!stream.isOpen()) {
		LOG_WARNING("%s could not be opened", filename.c_str());
		return false;
	}
	setFilename(filename);
	setTempo(125);
	setSpeed(6);
	setGlobalVolume(0x40);
	char modName[20];
	stream.read(modName, 20);
	setTitle( stringncpy(modName, 20) );
	// check 31-sample mod
	LOG_MESSAGE("Probing meta-info for 31-sample mod...");
	stream.seek(1080);
	IdMetaInfo meta = findMeta(&stream);
	if(meta.channels == 0) {
		LOG_WARNING("Could not find a valid module ID");
		return false;
	}
	LOG_MESSAGE("%d-channel, ID '%s', Tracker '%s'", meta.channels, meta.id.c_str(), meta.tracker.c_str());
	setTrackerInfo(meta.tracker);
	for(int i=0; i<meta.channels; i++) {
		m_channels.push_back( ModChannel::Ptr(new ModChannel(this)) );
	}
	stream.seek(20);
	for(uint8_t i=0; i<31; i++) {
		ModSample::Ptr smp(new ModSample());
		if(!smp->loadHeader(stream)) {
			LOG_WARNING("Sample header could not be loaded");
			return false;
		}
		m_samples.push_back(smp);
	}
	uint8_t maxPatNum = 0;
	{
		// load orders
		uint8_t songLen;
		LOG_DEBUG("Song len @ 0x%x", stream.pos());
		stream.read(&songLen);
		if(songLen>128) {
			songLen = 128;
		}
		LOG_DEBUG("Song length: %u", songLen);
		uint8_t tmp;
		stream.read(&tmp); // skip the restart pos
		for(uint8_t i=0; i<128; i++) {
			stream.read(&tmp);
			if(i>=songLen) {
				continue;
			}
			else if(tmp>maxPatNum) {
				maxPatNum = tmp;
			}
			if(maxPatNum > 63) {
				LOG_WARNING("Pattern number out of range: %u", maxPatNum);
				return false;
			}
			LOG_DEBUG("Order: %u", tmp);
			addOrder( GenOrder::Ptr(new GenOrder(tmp)) );
		}
	}
	stream.seekrel(4); // skip the ID
	LOG_DEBUG("Patterns @ %u", stream.pos());
	for(uint8_t i=0; i<=maxPatNum; i++) {
		ModPattern::Ptr pat(new ModPattern());
		if(!pat->load(stream, meta.channels)) {
			LOG_WARNING("Could not load pattern");
			return false;
		}
		m_patterns.push_back(pat);
	}
	for(const ModSample::Ptr& smp : m_samples) {
		if(!smp->loadData(stream)) {
			LOG_WARNING("Could not load sample data");
		}
	}
	return stream.good();
}

void ModModule::buildTick(AudioFrameBuffer& buf)
{
	try {
		//PPP_TEST(!buf);
		if(!buf)
			buf.reset(new AudioFrameBuffer::element_type);
		if(tick() == 0)
			checkGlobalFx();
		//buf->resize(getTickBufLen());
		//buf->clear();
		if(!adjustPosition(false, false)) {
			LOG_MESSAGE("Song end reached: adjustPosition() failed");
			buf.reset();
			return;
		}
		if(orderAt(playbackInfo().order)->playbackCount() >= maxRepeat()) {
			LOG_MESSAGE("Song end reached: Maximum repeat count reached");
			buf.reset();
			return;
		}
		// update channels...
		setPatternIndex(mapOrder(playbackInfo().order)->index());
		ModPattern::Ptr currPat = getPattern(playbackInfo().pattern);
		if(!currPat)
			return;
		MixerFrameBuffer mixerBuffer(new MixerFrameBuffer::element_type(tickBufferLength(), {0, 0}));
		for(unsigned short currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModChannel::Ptr chan = m_channels.at(currTrack);
			BOOST_ASSERT(chan.use_count()>0);
			ModCell::Ptr cell = currPat->cellAt(currTrack, playbackInfo().row);
			chan->update(cell, false);// m_patDelayCount != -1);
			chan->mixTick(mixerBuffer);
// 			break;
		}
		buf->resize(mixerBuffer->size());
		MixerSample* mixerBufferPtr = &mixerBuffer->front().left;
		BasicSample* bufPtr = &buf->front().left;
		for(size_t i = 0; i < mixerBuffer->size(); i++) {    // postprocess...
			*(bufPtr++) = clipSample(*(mixerBufferPtr++) >> 2);
			*(bufPtr++) = clipSample(*(mixerBufferPtr++) >> 2);
		}
		adjustPosition(true, false);
		setPosition(position() + mixerBuffer->size());
	}
	catch( boost::exception& e) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
	catch(...) {
		BOOST_THROW_EXCEPTION( std::runtime_error("Unknown exception") );
	}
}

bool ModModule::adjustPosition(bool increaseTick, bool doStore)
{
	BOOST_ASSERT(orderCount() != 0);
	bool orderChanged = false;
	if(increaseTick) {
		nextTick();
	}
	if((tick() == 0) && increaseTick) {
		m_patDelayCount = -1;
		if(m_breakOrder != -1) {
			orderAt(playbackInfo().order)->increasePlaybackCount();
			if(m_breakOrder < orderCount()) {
				setOrder(m_breakOrder);
				orderChanged = true;
			}
			setRow(0);
		}
		if(m_breakRow != -1) {
			if(m_breakRow <= 63) {
				setRow(m_breakRow);
			}
			if(m_breakOrder == -1) {
				if(m_patLoopCount == -1) {
					orderAt(playbackInfo().order)->increasePlaybackCount();
					setOrder(playbackInfo().order + 1);
					orderChanged = true;
				}
				//else {
				//	LOG_MESSAGE(stringf("oO... aPatLoopCount=%d",aPatLoopCount));
				//}
			}
		}
		if((m_breakRow == -1) && (m_breakOrder == -1) && (m_patDelayCount == -1)) {
			setRow((playbackInfo().row + 1) & 0x3f);
			if(playbackInfo().row == 0) {
				orderAt(playbackInfo().order)->increasePlaybackCount();
				setOrder(playbackInfo().order + 1);
				orderChanged = true;
			}
		}
		m_breakRow = m_breakOrder = -1;
	}
	setPatternIndex(mapOrder(playbackInfo().order)->index());
	if(orderChanged) {
		m_patLoopRow = 0;
		m_patLoopCount = -1;
		try {
			if(doStore) {
				multiSongAt(currentSongIndex()).newState()->archive(this).finishSave();
			}
			else {
				bool wasLocked = tryLock();
				multiSongAt(currentSongIndex()).nextState()->archive(this).finishLoad();
				if(wasLocked) {
					unlock();
				}
			}
		}
		catch( boost::exception& e) {
			BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
		}
		catch(...) {
			BOOST_THROW_EXCEPTION( std::runtime_error("Unknown exception") );
		}
	}
	return true;
}

void ModModule::simulateTick(size_t& bufLen)
{
	try {
		if(tick() == 0)
			checkGlobalFx();
		bufLen = 0;
		if(!adjustPosition(false, true))
			return;
		BOOST_ASSERT( mapOrder(playbackInfo().order).use_count()>0 );
		if(orderAt(playbackInfo().order)->playbackCount() >= maxRepeat())
			return;
		// update channels...
		setPatternIndex(mapOrder(playbackInfo().order)->index());
		ModPattern::Ptr currPat = getPattern(playbackInfo().pattern);
		if(!currPat)
			return;
		bufLen = tickBufferLength(); // in frames
		for(unsigned short currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModChannel::Ptr chan = m_channels.at(currTrack);
			BOOST_ASSERT(chan.use_count()>0);
			ModCell::Ptr cell = currPat->cellAt(currTrack, playbackInfo().row);
			chan->update(cell, false);// m_patDelayCount != -1);
			chan->simTick(bufLen);
		}
		adjustPosition(true, true);
		setPosition(position() + bufLen);
	}
	catch( boost::exception& e) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
	catch(...) {
		BOOST_THROW_EXCEPTION( std::runtime_error("Unknown exception") );
	}
}

std::string ModModule::channelCellString(int16_t idx)
{
	BOOST_ASSERT( idx>=0 && idx<m_channels.size() );
	return m_channels.at(idx)->cellString();
}

std::string ModModule::channelStatus(int16_t idx)
{
	BOOST_ASSERT( idx>=0 && idx<m_channels.size() );
	return m_channels.at(idx)->statusString();
}

bool ModModule::initialize(uint32_t frq)
{
	if(initialized()) {
		return true;
	}
	IAudioSource::initialize(frq);
	addMultiSong( StateIterator() );
	multiSongAt(0).newState()->archive(this).finishSave();
	// calculate total length...
	LOG_MESSAGE("Calculating track lengths and preparing seek operations...");
	size_t currTickLen = 0;
	do {
		simulateTick(currTickLen);
		multiSongLengthAt(0) += currTickLen;
	}
	while(currTickLen != 0);
	LOG_MESSAGE("Preprocessed. %u", multiSongLengthAt(0));
	{
		IAudioSource::LockGuard guard(this);
		multiSongAt(0).currentState()->archive(this).finishLoad();
	}
	removeEmptySongs();
	return true;
}

bool ModModule::jumpNextOrder()
{
	return false; // TODO
}

bool ModModule::jumpNextSong()
{
	return false;
}

bool ModModule::jumpPrevOrder()
{
	return false; // TODO
}

bool ModModule::jumpPrevSong()
{
	return false;
}

GenOrder::Ptr ModModule::mapOrder(int16_t order)
{
	if(order<0 || order>63) {
		return GenOrder::Ptr();
	}
	return orderAt(order);
}

IArchive& ModModule::serialize(IArchive* data)
{
    GenModule::serialize(data);
	for(const ModChannel::Ptr& chan : m_channels) {
		data->archive(chan.get());
	}
	return *data;
}

uint8_t ModModule::channelCount() const
{
	return m_channels.size();
}

bool ModModule::existsSample(size_t idx) const
{
	if(idx==0 || idx>30) {
		return false;
	}
	return m_samples.at(idx-1)->length()>0;
}

void ModModule::checkGlobalFx()
{
	try {
		setPatternIndex(mapOrder(playbackInfo().order)->index());
		ModPattern::Ptr currPat = getPattern(playbackInfo().pattern);
		if(!currPat)
			return;
		std::string data;
		// check for pattern loops
		int patLoopCounter = 0;
		for(uint8_t currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModCell::Ptr cell = currPat->cellAt(currTrack, playbackInfo().row);
			if(!cell) continue;
			if(cell->effect() == 0x0f) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if(fx != 0x0e) continue;
			if(highNibble(fxVal) != 0x06) continue;
			if(lowNibble(fxVal) == 0x00) {      // loop start
				m_patLoopRow = playbackInfo().row;
			}
			else { // loop return
				patLoopCounter++;
				if(m_patLoopCount == -1) {    // first loop return -> set loop count
					m_patLoopCount = lowNibble(fxVal);
					m_breakRow = m_patLoopRow;
				}
				else if(m_patLoopCount > 1) {    // non-initial return -> decrease loop counter
					m_patLoopCount--;
					m_breakRow = m_patLoopRow;
				}
				else { // loops done...
					if(patLoopCounter == 1) {    // one loop, all ok
						m_patLoopCount = -1;
						m_breakRow = -1;
						m_patLoopRow = playbackInfo().row + 1;
					}
					else { // we got an "infinite" loop...
						m_patLoopCount = 127;
						m_breakRow = m_patLoopRow;
						LOG_WARNING("Infinite pattern loop detected");
					}
				}
			}
		}
		// check for pattern delays
		uint8_t patDelayCounter = 0;
		for(uint8_t currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModCell::Ptr cell = currPat->cellAt(currTrack, playbackInfo().row);
			if(!cell) continue;
			if(cell->effect() == 0x0f) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if(fx != 0x0e) continue;
			if(highNibble(fxVal) != 0x0e) continue;
			if(lowNibble(fxVal) == 0) continue;
			if(++patDelayCounter != 1) continue;
			if(m_patDelayCount != -1) continue;
			m_patDelayCount = lowNibble(fxVal);
		}
		if(m_patDelayCount > 1)
			m_patDelayCount--;
		else
			m_patDelayCount = -1;
		// now check for breaking effects
		for(uint8_t currTrack = 0; currTrack < channelCount(); currTrack++) {
			if(m_patLoopCount != -1) break;
			ModCell::Ptr cell = currPat->cellAt(currTrack, playbackInfo().row);
			if(!cell) continue;
			if(cell->effect() == 0x0f) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if(fx == 0x0b) {
				m_breakOrder = fxVal;
			}
			else if(fx == 0x0d) {
				m_breakRow = highNibble(fxVal) * 10 + lowNibble(fxVal);
				LOG_MESSAGE("Row %d: Break pattern to row %d", playbackInfo().row, m_breakRow);
			}
		}
	}
	catch( boost::exception& e) {
		BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	}
	catch(...) {
		BOOST_THROW_EXCEPTION( std::runtime_error("Unknown exception") );
	}
}

ModPattern::Ptr ModModule::getPattern(size_t idx) const
{
	if(idx >= m_patterns.size()) return ModPattern::Ptr();
	return m_patterns.at(idx);
}


}
}
