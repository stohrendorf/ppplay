#include "xmmodule.h"
#include "stream/binstream.h"
#include "logger/logger.h"

using namespace ppp;
using namespace ppp::xm;

#pragma pack(push,1)
struct XmHeader {
	char id[17]; // "Extended Module: "
	char title[20];
	uint8_t endOfFile; // 0x1a
	char trackerName[20];
	uint16_t version;
	uint32_t headerSize;
	uint16_t songLength;
	uint16_t restartPos;
	uint16_t numChannels;
	uint16_t numPatterns;
	uint16_t numInstruments;
	uint16_t flags;
	uint16_t defaultSpeed;
	uint16_t defaultTempo;
	uint8_t orders[256];
};
#pragma pack(pop)

XmModule::XmModule(const uint32_t frq, const uint8_t maxRpt) throw(PppException) : GenModule(frq, maxRpt), m_amiga(false) {
}

bool XmModule::load(const std::string& filename) throw(PppException) {
	XmHeader hdr;
	FBinStream file(filename);
	setFilename(filename);
	file.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
	LOG_DEBUG("Header end @ 0x%.8x", file.pos());
	if(!std::equal(hdr.id, hdr.id+17, "Extended Module: ") || hdr.endOfFile!=0x1a || hdr.numChannels>32) {
		LOG_WARNING_("XM Header invalid");
		return false;
	}
	if(hdr.version!=0x0104) {
		LOG_WARNING("Unsupported XM Version 0x%.4x", hdr.version);
		return false;
	}
	{
		std::string title = stringncpy(hdr.title, 20);
		while(title.length()>0 && title[title.length()-1]==' ')
			title.erase(title.length()-1, 1);
		setTitle(title);
	}
	setTrackerInfo(stringncpy(hdr.trackerName, 20));
	setTempo(hdr.defaultTempo & 0xff);
	setSpeed(hdr.defaultSpeed & 0xff);
	setGlobalVolume(0x40);
	m_amiga = (hdr.flags&1)==0;
	m_channelCount = hdr.numChannels;
	for(uint16_t i=0; i<hdr.numPatterns; i++) {
		XmPattern::Ptr pat(new XmPattern(hdr.numChannels));
		if(!pat->load(file)) {
			LOG_ERROR_("Pattern loading error");
			return false;
		}
		m_patterns.push_back(pat);
	}
	for(uint16_t i=0; i<hdr.numInstruments; i++) {
		XmInstrument::Ptr ins(new XmInstrument());
		if(!ins->load(file)) {
			LOG_ERROR_("Instrument loading error");
			return false;
		}
		m_instruments.push_back(ins);
	}
	// WARNING Remove this when player code exists!
	return false;
}

uint16_t XmModule::getTickBufLen() const throw(PppException) {
	PPP_TEST(getPlaybackInfo().tempo < 0x20);
	return getPlaybackFrq()*5 / (getPlaybackInfo().tempo<<1);
}

void XmModule::getTick(AudioFrameBuffer& buffer) throw(PppException) {
#warning Stub - implement me
	if(!buffer)
		buffer.reset(new AudioFrameBuffer::element_type);
}

void XmModule::getTickNoMixing(std::size_t& len) throw(PppException) {
#warning Stub - implement me
	len = getTickBufLen();
}

GenOrder::Ptr XmModule::mapOrder(int16_t order) throw(PppException) {
	static GenOrder::Ptr xxx( new GenOrder( 0xff ) );
	if ( !inRange<int16_t>( order, 0, getOrderCount() - 1 ) )
		return xxx;
	return getOrder(order);
}

std::string XmModule::getChanStatus(int16_t) throw() {
	return std::string();
}

bool XmModule::jumpNextTrack() throw(PppException) {
	return false;
}

bool XmModule::jumpPrevTrack() throw(PppException) {
	return false;
}

bool XmModule::jumpNextOrder() throw() {
	return false;
}

bool XmModule::jumpPrevOrder() throw() {
	return false;
}

std::string XmModule::getChanCellString(int16_t) throw() {
	return std::string();
}

uint8_t XmModule::channelCount() const {
	return m_channelCount;
}
