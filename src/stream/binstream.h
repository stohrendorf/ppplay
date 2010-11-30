#ifndef binstreamH
#define binstreamH

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "stuff/utils.h"

#ifndef WITHIN_DOXYGEN
class BinStream;
#endif
/**
 * @class IStreamable
 * @ingroup Common
 * @brief Interface class for complex objects
 */
class ISerializable {
	public:
		/**
		 * @brief Serialise this object
		 * @param[in,out] stream BinStream to serialize this object to
		 * @return Reference to @a stream for pipelining
		 */
		virtual BinStream &serialize(BinStream &stream) const = 0;
		/**
		 * @brief Unserialise this object
		 * @param[in,out] stream BinStream to serialize this object from
		 * @return Reference to @a stream for pipelining
		 */
		virtual BinStream &unserialize(BinStream &stream) = 0;
		/**
		 * @brief Destructor
		 */
		virtual inline ~ISerializable() {}
};

/**
 * @class BinStream
 * @ingroup Common
 * @brief A binary stream helper
 */
class BinStream {
	public:
		typedef std::shared_ptr<std::iostream> SpIoStream; //!< @brief Shared IO Stream Pointer
		typedef std::shared_ptr<BinStream> SpBinStream; //!< @brief Shared BinStream Pointer
	private:
		SpIoStream m_stream; //!< @brief The IO Stream associated with this BinStream
		/**
		 * @brief Deny default constructor
		 */
		BinStream() = delete;
		/**
		 * @brief Deny copy constructor
		 */
		BinStream(const BinStream &) = delete;
		/**
		 * @brief Deny copy operator
		 */
		BinStream &operator=(const BinStream &) = delete;
	public:
		/**
		 * @brief Default constructor
		 * @param[in] stream Shared pointer to the IO Stream to associate with this BinStream
		 */
		explicit BinStream(SpIoStream stream) : m_stream(stream) { }
		/**
		 * @brief Pure virtual destructor
		 */
		virtual ~BinStream() {}
		/**
		 * @brief Constructor with stream destructor
		 * @tparam D Stream destructor typedef
		 * @param[in] stream Shared pointer to the std::iostream to associate with this BinStream
		 * @param[in] d Destructor
		 */
		template<class D>
		explicit BinStream(SpIoStream stream, D d) : m_stream(stream, d) { }
		/**
		 * @brief Read array data from the stream
		 * @tparam TR Data type
		 * @param[out] data Pointer to the data array
		 * @param[in] count Count of data elements (NOT the byte size)
		 * @return Reference to *this for pipelining
		 */
		template<typename TR> BinStream &read(TR *data, const std::size_t count = 1) __attribute__((nonnull(1)));
		/**
		 * @brief Write array data to the stream
		 * @tparam TW Data type
		 * @param[in] data Pointer to the data array
		 * @param[in] count Count of data elements (NOT the byte size)
		 * @return Reference to *this for pipelining
		 */
		template<typename TW> BinStream &write(const TW *data, const std::size_t count = 1) __attribute__((nonnull(1)));
		/**
		 * @brief Serialise an ISerialisable object
		 * @param[in] str Reference to the object
		 * @return Reference to *this for pipelining
		 */
		BinStream &writeSerialisable(const ISerializable *str) { return str->serialize(*this); }
		/**
		 * @brief Unserialise an ISerialisable object
		 * @param[in,out] str Reference to the object
		 * @return Reference to *this for pipelining
		 */
		BinStream &readSerialisable(ISerializable *str) { return str->unserialize(*this); }
		/**
		 * @brief Get the failbit of the IO Stream
		 * @return @c true on error
		 */
		bool fail() const { return m_stream->fail(); }
		/**
		 * @brief Get the goodbit of the IO Stream
		 * @return @c false on error
		 */
		bool good() const { return m_stream->good(); }
		/**
		 * @brief Clear the failbits of the IO Stream
		 */
		void clear() { m_stream->clear(); }
		/**
		 * @brief Seek to a stream position
		 * @param[in] pos Position to seek to
		 */
		void seek(std::size_t pos) { m_stream->seekg(pos); m_stream->seekp(pos); }
		/**
		 * @brief Seek to a relative stream position
		 * @param[in] delta Relative seek position
		 */
		void seekrel(int delta) { uint32_t p = pos(); m_stream->seekg(p+delta); m_stream->seekp(p+delta); }
		/**
		 * @brief Get the stream position
		 * @return The IO Stream position
		 */
		uint32_t pos() const { return m_stream->tellg(); }
		/**
		 * @brief Const access to the internal stream
		 * @return BinStream::m_stream
		 */
		const SpIoStream &stream() const { return m_stream; }
		/**
		 * @brief Access to the internal stream
		 * @return BinStream::m_stream
		 */
		SpIoStream &stream() { return m_stream; }
};

// NOTE Templates are (due to explicit instantiation) outsourced ;-)

/**
 * @class FBinStream
 * @ingroup Common
 * @brief Class derived from ::BinStream for files
 * @note This is a read-only stream
 */
class FBinStream : public BinStream {
	private:
		std::string m_filename; //!< @brief Filename of the file
		/**
		 * @brief Deny default constructor
		 */
		FBinStream() = delete;
		/**
		 * @brief Deny copy constructor
		 */
		FBinStream(const FBinStream &) = delete;
		/**
		 * @brief Deny copy operator
		 */
		FBinStream &operator=(const FBinStream &) = delete;
	public:
		/**
		 * @brief Default contructor
		 * @param[in] filename Filename of the file to open
		 */
		explicit FBinStream(const std::string &filename);
		/**
		 * @brief Destructor
		 */
		virtual ~FBinStream();
		/**
		 * @brief Check if the file is opened
		 * @return @c true if the file is opened
		 */
		bool isOpen() const;
};

/**
 * @class SBinStream
 * @ingroup Common
 * @brief Class derived from ::BinStream for a ::stringstream
 */
class SBinStream : public BinStream {
	private:
		/**
		 * @brief Deny copy constructor
		 */
		SBinStream(const SBinStream &) = delete;
		/**
		 * @brief Deny copy operator
		 */
		SBinStream &operator=(const SBinStream &) = delete;
	public:
		/**
		 * @brief Default constructor
		 */
		explicit SBinStream() : BinStream(BinStream::SpIoStream(new std::stringstream(std::ios::in|std::ios::out|std::ios::binary))) {};
		/**
		 * @brief Destructor
		 */
		virtual ~SBinStream() {}
};

#ifndef WITHIN_DOXYGEN

#define BINSTREAM_RW_DECL(tn)\
extern template BinStream &BinStream::read<tn>(tn *, const std::size_t); \
extern template BinStream &BinStream::write<tn>(const tn*, const std::size_t);

BINSTREAM_RW_DECL(int8_t)
BINSTREAM_RW_DECL(uint8_t)
BINSTREAM_RW_DECL(int16_t)
BINSTREAM_RW_DECL(uint16_t)
BINSTREAM_RW_DECL(int32_t)
BINSTREAM_RW_DECL(uint32_t)
BINSTREAM_RW_DECL(int64_t)
BINSTREAM_RW_DECL(uint64_t)
BINSTREAM_RW_DECL(char)
BINSTREAM_RW_DECL(bool)
BINSTREAM_RW_DECL(float)

#undef BINSTREAM_RW_DECL

#endif // WITHIN_DOXYGEN

#endif // binstreamH
