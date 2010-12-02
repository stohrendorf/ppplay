#include "xmmodule.h"

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
	uint16_t defaultTempo;
	uint16_t defaultSpeed;
	uint8_t orders[256];
	
};
#pragma pack(pop)

XmModule::XmModule(const uint32_t frq = 44100, const uint8_t maxRpt = 2) throw(PppException) : GenModule() {
}

bool XmModule::load(const std::string& filename) throw(PppException) {
	return false;
}

uint16_t XmModule::getTickBufLen() const throw(PppException) {
	return 0;
}

void XmModule::getTick(ppp::AudioFrameBuffer& buffer) throw(PppException) {
}

void XmModule::getTickNoMixing(std::size_t&) throw(PppException) {
}

GenOrder::Ptr XmModule::mapOrder(int16_t) throw(PppException) {
	return GenOrder::Ptr();
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

std::string XmModule::getChanCellString(int16_t) throw() {
	return std::string();
}

uint8_t XmModule::channelCount() const {
	return 0;
}
