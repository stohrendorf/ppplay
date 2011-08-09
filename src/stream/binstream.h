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

#ifndef BINSTREAM_H
#define BINSTREAM_H

#include "stuff/utils.h"

#include <iostream>
#include <memory>

/**
 * @class BinStream
 * @ingroup Common
 * @brief A binary stream helper
 */
class BinStream {
		DISABLE_COPY(BinStream)
		BinStream() = delete;
	public:
		typedef std::shared_ptr<std::iostream> SpIoStream; //!< @brief Shared IO Stream Pointer
		typedef std::shared_ptr<BinStream> Ptr; //!< @brief Class pointer
	private:
		SpIoStream m_stream; //!< @brief The IO Stream associated with this BinStream
	public:
		/**
		 * @brief Default constructor
		 * @param[in] stream Shared pointer to the IO Stream to associate with this BinStream
		 */
		explicit BinStream(SpIoStream stream) ;
		/**
		 * @brief Destructor
		 */
		virtual ~BinStream() {}
		/**
		 * @brief Constructor with stream destructor
		 * @tparam D Stream destructor type
		 * @param[in] stream Shared pointer to the std::iostream to associate with this BinStream
		 * @param[in] d Destructor
		 */
		template<class D>
		explicit BinStream(SpIoStream stream, D d) : m_stream(stream, d) { }
		/**
		 * @brief Read data from the stream
		 * @tparam TR Data type
		 * @param[out] data Pointer to the data array
		 * @param[in] count Count of data elements (NOT the byte size)
		 * @return Reference to *this for pipelining
		 */
		template<typename TR> BinStream& read(TR* data, size_t count = 1) __attribute__((nonnull(1)));
		/**
		 * @brief Write data to the stream
		 * @tparam TW Data type
		 * @param[in] data Pointer to the data array
		 * @param[in] count Count of data elements (NOT the byte size)
		 * @return Reference to *this for pipelining
		 */
		template<typename TW> BinStream& write(const TW* data, size_t count = 1) __attribute__((nonnull(1)));
		/**
		 * @brief Get the failbit of the IO Stream
		 * @return @c true on error
		 */
		bool fail() const;
		/**
		 * @brief Get the goodbit of the IO Stream
		 * @return @c false on error
		 */
		bool good() const;
		/**
		 * @brief Clear the failbits of the IO Stream
		 */
		void clear();
		/**
		 * @brief Seek to a stream position
		 * @param[in] pos Position to seek to
		 */
		void seek(uint32_t pos);
		/**
		 * @brief Seek to a relative stream position
		 * @param[in] delta Relative seek position
		 */
		void seekrel(int32_t delta);
		/**
		 * @brief Get the stream position
		 * @return The IO Stream position
		 */
		uint32_t pos() const;
		/**
		 * @brief Const access to the internal stream
		 * @return BinStream::m_stream
		 */
		const SpIoStream& stream() const;
		/**
		 * @brief Access to the internal stream
		 * @return BinStream::m_stream
		 */
		SpIoStream& stream();
};

// NOTE Templates are (due to explicit instantiation) outsourced ;-)

#define BINSTREAM_RW_DECL(tn)\
	extern template BinStream &BinStream::read<tn>(tn *, size_t); \
	extern template BinStream &BinStream::write<tn>(const tn*, size_t);

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

#endif // binstreamH
