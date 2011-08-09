#include "modmodule.h"

#include "stream/iarchive.h"
#include "stream/fbinstream.h"
#include "logger/logger.h"

namespace ppp {
namespace mod {

ModModule::ModModule(uint8_t maxRpt): GenModule(maxRpt),
	m_samples(), m_patterns(), m_channels()
{
}

ModModule::~ModModule() = default;

/**
 * @brief Maps module IDs to their respective channel counts
 */
typedef std::pair<std::string, uint8_t> IdChanPair;
 
static const std::array<IdChanPair, 1> idChanMap = {{
	{"M.K.", 4}
}};

uint8_t findChanId(BinStream* stream) {
	char id[4];
	stream->read(id, 4);
	for(const IdChanPair& p : idChanMap) {
		if(id == p.first) {
			return p.second;
		}
	}
	// revert...
	stream->seekrel(-4);
	return 0;
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
	char modName[22];
	stream.read(modName, 22);
	setTitle( stringncpy(modName, 22) );
	setTrackerInfo( "ProTracker" );
	// check 31-sample mod
	stream.seek(1080);
	uint8_t numChannels = findChanId(&stream);
	if(numChannels == 0) {
		LOG_WARNING("Could not find a valid module ID");
		return false;
	}
	m_channels.resize(numChannels);
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
		stream.read(&songLen);
		if(songLen>128) {
			songLen = 128;
		}
		uint8_t tmp;
		stream.read(&tmp);
		for(uint8_t i=0; i<128; i++) {
			stream.read(&tmp);
			if(i>=songLen) {
				tmp = 0xff;
			}
			else if(tmp>maxPatNum) {
				maxPatNum = tmp;
			}
			if(maxPatNum > 63) {
				LOG_WARNING("Pattern number out of range: %u", maxPatNum);
				return false;
			}
			orderAt(i)->setIndex(tmp);
		}
	}
	for(uint8_t i=0; i<maxPatNum; i++) {
		ModPattern::Ptr pat(new ModPattern());
		if(!pat->load(stream, numChannels)) {
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
	return true;
}

void ModModule::buildTick(AudioFrameBuffer& buf)
{
// TODO
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
	LOG_MESSAGE("Preprocessed.");
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

}
}