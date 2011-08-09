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

ModSample::Ptr ModModule::sampleAt(size_t idx) const
{
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

IdMetaInfo findMeta(BinStream* stream) {
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
	char modName[22];
	stream.read(modName, 22);
	setTitle( stringncpy(modName, 22) );
	// check 31-sample mod
	LOG_MESSAGE("Probing meta-info for 31-sample mod...");
	stream.seek(1080);
	IdMetaInfo meta = findMeta(&stream);
	if(meta.channels == 0) {
		LOG_WARNING("Could not find a valid module ID");
		return false;
	}
	setTrackerInfo(meta.tracker);
	m_channels.resize(meta.channels);
	stream.seek(22);
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
