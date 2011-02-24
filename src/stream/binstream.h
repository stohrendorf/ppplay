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
#include "stuff/pppexcept.h"

/**
 * @class BinStream
 * @ingroup Common
 * @brief A binary stream helper
 */
class BinStream {
		DISABLE_COPY( BinStream )
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
		explicit BinStream( SpIoStream stream ) : m_stream( stream ) { }
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
		explicit BinStream( SpIoStream stream, D d ) : m_stream( stream, d ) { }
		/**
		 * @brief Read data from the stream
		 * @tparam TR Data type
		 * @param[out] data Pointer to the data array
		 * @param[in] count Count of data elements (NOT the byte size)
		 * @return Reference to *this for pipelining
		 */
		template<typename TR> BinStream& read( TR* data, std::size_t count = 1 ) __attribute__( ( nonnull( 1 ) ) );
		/**
		 * @brief Write data to the stream
		 * @tparam TW Data type
		 * @param[in] data Pointer to the data array
		 * @param[in] count Count of data elements (NOT the byte size)
		 * @return Reference to *this for pipelining
		 */
		template<typename TW> BinStream& write( const TW* data, std::size_t count = 1 ) __attribute__( ( nonnull( 1 ) ) );
		/**
		 * @brief Get the failbit of the IO Stream
		 * @return @c true on error
		 */
		bool fail() const {
			return m_stream->fail();
		}
		/**
		 * @brief Get the goodbit of the IO Stream
		 * @return @c false on error
		 */
		bool good() const {
			return m_stream->good();
		}
		/**
		 * @brief Clear the failbits of the IO Stream
		 */
		void clear() {
			m_stream->clear();
		}
		/**
		 * @brief Seek to a stream position
		 * @param[in] pos Position to seek to
		 */
		void seek( uint32_t pos ) {
			m_stream->seekg( pos );
			m_stream->seekp( pos );
		}
		/**
		 * @brief Seek to a relative stream position
		 * @param[in] delta Relative seek position
		 */
		void seekrel( int32_t delta ) {
			uint32_t p = pos();
			m_stream->seekg( p + delta );
			m_stream->seekp( p + delta );
		}
		/**
		 * @brief Get the stream position
		 * @return The IO Stream position
		 */
		uint32_t pos() const {
			return m_stream->tellg();
		}
		/**
		 * @brief Const access to the internal stream
		 * @return BinStream::m_stream
		 */
		const SpIoStream& stream() const {
			return m_stream;
		}
		/**
		 * @brief Access to the internal stream
		 * @return BinStream::m_stream
		 */
		SpIoStream& stream() {
			return m_stream;
		}
};

// NOTE Templates are (due to explicit instantiation) outsourced ;-)

/**
 * @class FBinStream
 * @ingroup Common
 * @brief Class derived from BinStream for files
 * @note This is a read-only stream
 */
class FBinStream : public BinStream {
		DISABLE_COPY( FBinStream )
		FBinStream() = delete;
	private:
		std::string m_filename; //!< @brief Filename of the file
	public:
		/**
		 * @brief Default contructor
		 * @param[in] filename Filename of the file to open
		 */
		explicit FBinStream( const std::string& filename );
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
 * @brief Class derived from BinStream for a std::stringstream
 */
class SBinStream : public BinStream {
		DISABLE_COPY( SBinStream )
	public:
		explicit SBinStream() : BinStream( BinStream::SpIoStream( new std::stringstream( std::ios::in | std::ios::out | std::ios::binary ) ) ) {};
		virtual ~SBinStream() {}
};

#define BINSTREAM_RW_DECL(tn)\
	extern template BinStream &BinStream::read<tn>(tn *, std::size_t); \
	extern template BinStream &BinStream::write<tn>(const tn*, std::size_t);

BINSTREAM_RW_DECL( int8_t )
BINSTREAM_RW_DECL( uint8_t )
BINSTREAM_RW_DECL( int16_t )
BINSTREAM_RW_DECL( uint16_t )
BINSTREAM_RW_DECL( int32_t )
BINSTREAM_RW_DECL( uint32_t )
BINSTREAM_RW_DECL( int64_t )
BINSTREAM_RW_DECL( uint64_t )
BINSTREAM_RW_DECL( char )
BINSTREAM_RW_DECL( bool )
BINSTREAM_RW_DECL( float )

#undef BINSTREAM_RW_DECL

/**
 * @interface ISerializable
 * @ingroup Common
 * @brief Interface for serialisable classes
 */
class ISerializable {
	public:
		/**
		 * @brief Serialise this object
		 * @param[in,out] archive IArchive to serialize this object to
		 * @return Reference to @a archive for pipelining
		 */
		virtual class IArchive& serialize( class IArchive* archive ) = 0;
		/**
		 * @brief Destructor
		 */
		virtual ~ISerializable() = 0;
};

/**
 * @interface IArchive
 * @ingroup Common
 * @brief Interface for archives used by ISerializable inherited classes
 */
class IArchive {
		DISABLE_COPY( IArchive )
		IArchive() = delete;
	public:
		typedef std::shared_ptr<IArchive> Ptr; //!< @brief Class pointer
		typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
	private:
		bool m_loading; //!< @brief @c true for read-only access, @c false for write-only access
		BinStream::Ptr m_stream; //!< @brief The associated BinStream for storage
	public:
		/**
		 * @brief Constructor
		 * @param[in] stream The storage stream
		 */
		IArchive( const BinStream::Ptr& stream );
		virtual ~IArchive() = 0;
		/**
		 * @brief Whether this archive is read-only
		 * @return m_loading
		 * @see isSaving()
		 */
		bool isLoading() const {
			return m_loading;
		}
		/**
		 * @brief Whether this archive is write-only
		 * @return !m_loading
		 * @see isLoading()
		 */
		bool isSaving() const {
			return !m_loading;
		}
		/**
		 * @brief Serialization operator
		 * @tparam T Data type
		 * @param[in,out] data Data to save or load
		 * @return Reference to *this
		 * @note Operation depends on m_loading
		 */
		template<class T> IArchive& operator&( T& data ) {
			if( m_loading )
				m_stream->read( &data, 1 );
			else
				m_stream->write( &data, 1 );
			return *this;
		}
		/**
		 * @brief Serialization operator for arrays
		 * @tparam T Data type
		 * @param[in,out] data Data array to save or load
		 * @param[in] count Number of elements in @a data
		 * @return Reference to *this
		 * @note Operation depends on m_loading
		 */
		template<class T> IArchive& array( T* data, std::size_t count ) {
			PPP_TEST( data == NULL );
			if( m_loading )
				m_stream->read( data, count );
			else
				m_stream->write( data, count );
			return *this;
		}
		/**
		 * @brief Serialization operator for other archives
		 * @param[in,out] data Archive to save or load
		 * @return Reference to *this
		 * @note Operation depends on m_loading
		 */
		IArchive& archive( ISerializable* data ) {
			PPP_TEST( data == NULL );
			return data->serialize( this );
		}
		/**
		 * @brief Finish saving operation
		 * @details
		 * Resets the stream pointer to 0 and sets m_loading to @c true.
		 * @see finishLoad()
		 */
		void finishSave() {
			PPP_TEST( m_loading );
			m_stream->seek( 0 );
			m_loading = true;
		}
		/**
		 * @brief Finish loading operation
		 * @details
		 * Resets the stream pointer to 0. The archive remains read-only.
		 * @see finishSave()
		 */
		void finishLoad() {
			PPP_TEST( !m_loading );
			m_stream->seek( 0 );
		}
};

/**
 * @class MemArchive
 * @ingroup Common
 * @brief Specialization of IArchive for memory storage
 */
class MemArchive : public IArchive {
		DISABLE_COPY( MemArchive )
	public:
		/**
		 * @brief Constructs this archive using a SBinStream
		 */
		MemArchive();
		virtual ~MemArchive();
};

#endif // binstreamH
