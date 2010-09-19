#include "binstream.h"

FBinStream::FBinStream(const std::string& filename) :
	BinStream(SpIoStream(new std::fstream(filename.c_str(),std::ios::in|std::ios::binary))),
	m_filename(filename)
{
}

FBinStream::~FBinStream() {
	std::static_pointer_cast<std::fstream>(stream())->close();
}

bool FBinStream::isOpen() const {
	return std::static_pointer_cast<std::fstream>(stream())->is_open();
}

template<typename TR>
BinStream &BinStream::read(TR *data, const std::size_t count) {
	m_stream->read(reinterpret_cast<char*>(data), sizeof(TR)*count);
	return *this;
}
template<typename TW>
BinStream &BinStream::write( const TW *data, const std::size_t count) {
	m_stream->write(reinterpret_cast<const char*>(data), sizeof(TW)*count);
	return *this;
}

#ifndef WITHIN_DOXYGEN
SHARED_PTR_IMPL(std::iostream)
SHARED_PTR_IMPL(BinStream)

#define BINSTREAM_RW_IMPL(tn)\
template BinStream &BinStream::read<tn>(tn *, const std::size_t); \
template BinStream &BinStream::write<tn>(const tn*, const std::size_t);

BINSTREAM_RW_IMPL(int8_t)
BINSTREAM_RW_IMPL(uint8_t)
BINSTREAM_RW_IMPL(int16_t)
BINSTREAM_RW_IMPL(uint16_t)
BINSTREAM_RW_IMPL(int32_t)
BINSTREAM_RW_IMPL(uint32_t)
BINSTREAM_RW_IMPL(int64_t)
BINSTREAM_RW_IMPL(uint64_t)
BINSTREAM_RW_IMPL(char)
BINSTREAM_RW_IMPL(bool)
BINSTREAM_RW_IMPL(float)
#endif // WITHIN_DOXYGEN
