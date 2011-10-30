#include "modmodule.h"

#include "stream/iarchive.h"
#include "stream/fbinstream.h"

#include <boost/exception/all.hpp>
#include <boost/format.hpp>

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
	m_patLoopCount(-1), m_breakRow(-1), m_patDelayCount(-1), m_breakOrder(~0)
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
	logger()->info(L4CXX_LOCATION, boost::format("Opening '%s'")%filename);
	FBinStream stream(filename);
	if(!stream.isOpen()) {
		logger()->warn(L4CXX_LOCATION, "Could not open file");
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
	logger()->info(L4CXX_LOCATION, "Probing meta-info for 31-sample mod...");
	stream.seek(1080);
	IdMetaInfo meta = findMeta(&stream);
	if(meta.channels == 0) {
		logger()->warn(L4CXX_LOCATION, "Could not find a valid module ID");
		return false;
	}
	logger()->debug(L4CXX_LOCATION, boost::format("%d-channel, ID '%s', Tracker '%s'")%(meta.channels+0)%meta.id%meta.tracker);
	setTrackerInfo(meta.tracker);
	for(int i=0; i<meta.channels; i++) {
		m_channels.push_back( ModChannel::Ptr(new ModChannel(this)) );
	}
	stream.seek(20);
	for(uint8_t i=0; i<31; i++) {
		ModSample::Ptr smp(new ModSample());
		if(!smp->loadHeader(stream)) {
			logger()->warn(L4CXX_LOCATION, "Sample header could not be loaded");
			return false;
		}
		m_samples.push_back(smp);
	}
	uint8_t maxPatNum = 0;
	{
		// load orders
		uint8_t songLen;
// 		LOG_DEBUG("Song len @ 0x%x", stream.pos());
		stream.read(&songLen);
		if(songLen>128) {
			songLen = 128;
		}
		logger()->debug(L4CXX_LOCATION, boost::format("Song length: %d")%(songLen+0));
		uint8_t tmp;
		stream.read(&tmp); // skip the restart pos
		for(uint8_t i=0; i<128; i++) {
			stream.read(&tmp);
			if(tmp>=64) {
				continue;
			}
			else if(tmp>maxPatNum) {
				maxPatNum = tmp;
			}
			//if(maxPatNum > 63) {
				//LOG_WARNING("Pattern number out of range: %u", maxPatNum);
				//return false;
			//}
			//LOG_DEBUG("Order: %u", tmp);
			if(i<songLen) {
				addOrder( GenOrder::Ptr(new GenOrder(tmp)) );
			}
		}
	}
	stream.seekrel(4); // skip the ID
	logger()->debug(L4CXX_LOCATION, boost::format("%d patterns @ %#x")%(maxPatNum+0)%stream.pos());
	for(uint8_t i=0; i<=maxPatNum; i++) {
		ModPattern::Ptr pat(new ModPattern());
		if(!pat->load(stream, meta.channels)) {
			logger()->warn(L4CXX_LOCATION, "Could not load pattern");
			return false;
		}
		m_patterns.push_back(pat);
	}
	logger()->debug(L4CXX_LOCATION, boost::format("Sample start @ %#x")%stream.pos());
	for(const ModSample::Ptr& smp : m_samples) {
		if(!smp->loadData(stream)) {
			logger()->warn(L4CXX_LOCATION, "Could not load sample data");
		}
	}
	logger()->debug(L4CXX_LOCATION, boost::format("pos=%#x size=%#x delta=%#x")%stream.pos()%stream.size()%(stream.size()-stream.pos()));
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
			logger()->info(L4CXX_LOCATION, "Song end reached: adjustPosition() failed");
			buf.reset();
			return;
		}
		if(orderAt(order())->playbackCount() >= maxRepeat()) {
			logger()->info(L4CXX_LOCATION, "Song end reached: Maximum repeat count reached");
			buf.reset();
			return;
		}
		// update channels...
		setPatternIndex(mapOrder(order())->index());
		ModPattern::Ptr currPat = getPattern(patternIndex());
		if(!currPat)
			return;
		MixerFrameBuffer mixerBuffer(new MixerFrameBuffer::element_type(tickBufferLength(), {0, 0}));
		for(unsigned short currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModChannel::Ptr chan = m_channels.at(currTrack);
			BOOST_ASSERT(chan.use_count()>0);
			ModCell::Ptr cell = currPat->cellAt(currTrack, row());
			chan->update(cell, false);// m_patDelayCount != -1);
			chan->mixTick(mixerBuffer);
		}
		buf->resize(mixerBuffer->size());
		MixerSample* mixerBufferPtr = &mixerBuffer->front().left;
		BasicSample* bufPtr = &buf->front().left;
		for(size_t i = 0; i < mixerBuffer->size(); i++) {    // postprocess...
			*(bufPtr++) = clipSample(*(mixerBufferPtr++) >> 2);
			*(bufPtr++) = clipSample(*(mixerBufferPtr++) >> 2);
		}
		if(!adjustPosition(true, false)) {
			buf.reset();
			return;
		}
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
		if(m_breakOrder != 0xffff) {
			orderAt(order())->increasePlaybackCount();
			if(m_breakOrder < orderCount()) {
				setOrder(m_breakOrder);
				if(order() >= orderCount()) {
					return false;
				}
				orderChanged = true;
			}
			setRow(0);
		}
		if(m_breakRow != -1) {
			if(m_breakRow <= 63) {
				setRow(m_breakRow);
			}
			if(m_breakOrder == 0xffff) {
				if(m_patLoopCount == -1) {
					orderAt(order())->increasePlaybackCount();
					setOrder(order() + 1);
					if(order() >= orderCount()) {
						return false;
					}
					orderChanged = true;
				}
				//else {
				//	LOG_MESSAGE(stringf("oO... aPatLoopCount=%d",aPatLoopCount));
				//}
			}
		}
		if((m_breakRow == -1) && (m_breakOrder == 0xffff) && (m_patDelayCount == -1)) {
			setRow((row() + 1) & 0x3f);
			if(row() == 0) {
				orderAt(order())->increasePlaybackCount();
				setOrder(order() + 1);
				if(order() >= orderCount()) {
					return false;
				}
				orderChanged = true;
			}
		}
		m_breakRow = -1;
		m_breakOrder = ~0;
	}
	if(order() >= orderCount()) {
		return false;
	}
	setPatternIndex(mapOrder(order())->index());
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
	//try {
		if(tick() == 0)
			checkGlobalFx();
		bufLen = 0;
		if(!adjustPosition(false, true))
			return;
		BOOST_ASSERT( mapOrder(order()).use_count()>0 );
		if(orderAt(order())->playbackCount() >= maxRepeat())
			return;
		// update channels...
		setPatternIndex(mapOrder(order())->index());
		ModPattern::Ptr currPat = getPattern(patternIndex());
		if(!currPat)
			return;
		bufLen = tickBufferLength(); // in frames
		for(unsigned short currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModChannel::Ptr chan = m_channels.at(currTrack);
			BOOST_ASSERT(chan.use_count()>0);
			ModCell::Ptr cell = currPat->cellAt(currTrack, row());
			chan->update(cell, false);// m_patDelayCount != -1);
			chan->simTick(bufLen);
		}
		if(!adjustPosition(true, true)) {
			bufLen = 0;
			return;
		}
		setPosition(position() + bufLen);
	//}
	//catch( boost::exception& e) {
		//BOOST_THROW_EXCEPTION( std::runtime_error( boost::current_exception_diagnostic_information() ) );
	//}
	//catch(...) {
		//BOOST_THROW_EXCEPTION( std::runtime_error("Unknown exception") );
	//}
}

std::string ModModule::channelCellString(size_t idx)
{
	BOOST_ASSERT( idx<m_channels.size() );
	return m_channels.at(idx)->cellString();
}

std::string ModModule::channelStatus(size_t idx)
{
	BOOST_ASSERT( idx<m_channels.size() );
	return m_channels.at(idx)->statusString();
}

bool ModModule::initialize(uint32_t frq)
{
	if(initialized()) {
		return true;
	}
	IAudioSource::initialize(frq);
	addMultiSong( StateIterator() );
	multiSongAt(currentSongIndex()).newState()->archive(this).finishSave();
	// calculate total length...
	logger()->info(L4CXX_LOCATION, "Calculating track lengths and preparing seek operations...");
	do {
		logger()->info(L4CXX_LOCATION, boost::format("Pre-processing Track %d") % currentSongIndex());
		size_t currTickLen = 0;
		do {
			simulateTick(currTickLen);
			multiSongLengthAt(currentSongIndex()) += currTickLen;
		} while(currTickLen != 0);
		logger()->info(L4CXX_LOCATION, "Preprocessed.");
		for(size_t i=0; i<orderCount(); i++) {
			BOOST_ASSERT(orderAt(i).use_count()>0);
			if(orderAt(i)->playbackCount()==0) {
				addMultiSong( StateIterator() );
				setCurrentSongIndex( currentSongIndex()+1 );
				break;
			}
		}
		logger()->info(L4CXX_LOCATION, "Trying to jump to the next song");
	} while(jumpNextSong());
	if(songCount() > 0) {
		logger()->info(L4CXX_LOCATION, "Preprocessed.");
		IAudioSource::LockGuard guard(this);
		multiSongAt(0).currentState()->archive(this).finishLoad();
	}
	removeEmptySongs();
	return true;
}

bool ModModule::jumpNextOrder()
{
	IArchive::Ptr next = multiSongAt(currentSongIndex()).nextState();
	if(!next) {
		return false;
	}
	IAudioSource::LockGuard guard(this);
	next->archive(this).finishLoad();
	return true;
}

bool ModModule::jumpNextSong()
{
	if(!isMultiSong()) {
		logger()->info(L4CXX_LOCATION, "This is not a multi-song");
		return false;
	}
	BOOST_ASSERT( mapOrder(order()).use_count()>0 );
	orderAt(order())->increasePlaybackCount();
	setCurrentSongIndex(currentSongIndex() + 1);
	if(currentSongIndex() >= songCount()) {
		for(uint16_t i = 0; i < orderCount(); i++) {
			BOOST_ASSERT(orderAt(i).use_count()>0);
			if(orderAt(i)->playbackCount() == 0) {
				BOOST_ASSERT(mapOrder(i).use_count()>0);
				setPatternIndex(mapOrder(i)->index());
				setOrder(i);
				addMultiSong(StateIterator());
				setPosition(0);
				IAudioSource::LockGuard guard(this);
				multiSongAt(currentSongIndex()).newState()->archive(this).finishSave();
				return true;
			}
		}
		addMultiSong(StateIterator());
		return false;
	}
	else {
		IAudioSource::LockGuard guard(this);
		multiSongAt(currentSongIndex()).gotoFront();
		multiSongAt(currentSongIndex()).currentState()->archive(this).finishLoad();
		BOOST_ASSERT(mapOrder(order()).use_count()>0);
		setPatternIndex(mapOrder(order())->index());
		return true;
	}
	logger()->fatal(L4CXX_LOCATION, "This should definitively NOT have happened...");
	return false;
}

bool ModModule::jumpPrevOrder()
{
	IArchive::Ptr next = multiSongAt(currentSongIndex()).prevState();
	if(!next) {
		return false;
	}
	IAudioSource::LockGuard guard(this);
	next->archive(this).finishLoad();
	return true;
}

bool ModModule::jumpPrevSong()
{
	if(!isMultiSong()) {
		logger()->info(L4CXX_LOCATION, "This is not a multi-song");
		return false;
	}
	if(currentSongIndex() == 0) {
		logger()->info(L4CXX_LOCATION, "Already on first song");
		return false;
	}
	setCurrentSongIndex(currentSongIndex() - 1);
	IAudioSource::LockGuard guard(this);
	multiSongAt(currentSongIndex()).gotoFront();
	multiSongAt(currentSongIndex()).currentState()->archive(this).finishLoad();
	BOOST_ASSERT( mapOrder(order()).use_count()>0 );
	setPatternIndex(mapOrder(order())->index());
	return true;
}

GenOrder::Ptr ModModule::mapOrder(int16_t order)
{
	if(order<0 || order>127) {
		return GenOrder::Ptr();
	}
	return orderAt(order);
}

IArchive& ModModule::serialize(IArchive* data)
{
	GenModule::serialize(data)
	% m_breakRow
	% m_breakOrder
	% m_patLoopRow
	% m_patLoopCount
	% m_patDelayCount;
	for(ModChannel::Ptr& chan : m_channels) {
		if(!chan) {
			continue;
		}
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
		setPatternIndex(mapOrder(order())->index());
		ModPattern::Ptr currPat = getPattern(patternIndex());
		if(!currPat)
			return;
		// check for pattern loops
		int patLoopCounter = 0;
		for(uint8_t currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModCell::Ptr cell = currPat->cellAt(currTrack, row());
			if(!cell) continue;
			if(cell->effect() == 0x0f) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if(fx != 0x0e) continue;
			if(highNibble(fxVal) != 0x06) continue;
			if(lowNibble(fxVal) == 0x00) {      // loop start
				m_patLoopRow = row();
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
						m_patLoopRow = row() + 1;
					}
					else { // we got an "infinite" loop...
						m_patLoopCount = 127;
						m_breakRow = m_patLoopRow;
						logger()->warn(L4CXX_LOCATION, "Infinite pattern loop detected");
					}
				}
			}
		}
		// check for pattern delays
		uint8_t patDelayCounter = 0;
		for(uint8_t currTrack = 0; currTrack < channelCount(); currTrack++) {
			ModCell::Ptr cell = currPat->cellAt(currTrack, row());
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
			ModCell::Ptr cell = currPat->cellAt(currTrack, row());
			if(!cell) continue;
			if(cell->effect() == 0x0f) continue;
			uint8_t fx = cell->effect();
			uint8_t fxVal = cell->effectValue();
			if(fx == 0x0b) {
				m_breakOrder = fxVal;
			}
			else if(fx == 0x0d) {
				m_breakRow = highNibble(fxVal) * 10 + lowNibble(fxVal);
				logger()->info(L4CXX_LOCATION, boost::format("Row %1%: Break pattern to row %2%")%row()%(m_breakRow+0));
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

light4cxx::Logger::Ptr ModModule::logger()
{
	return light4cxx::Logger::get( GenModule::logger()->name() + ".mod" );
}


}
}
