/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PPPLAY_ABSTRACTARCHIVE_H
#define PPPLAY_ABSTRACTARCHIVE_H

#include "stream.h"

#include <boost/assert.hpp>

#include <vector>
#include <type_traits>

class ISerializable;

/**
 * @interface AbstractArchive
 * @ingroup Common
 * @brief Interface for archives used by ISerializable inherited classes
 */
class PPPLAY_STREAM_EXPORT AbstractArchive
{
    DISABLE_COPY( AbstractArchive )
    AbstractArchive() = delete;
public:
    typedef std::shared_ptr<AbstractArchive> Ptr; //!< @brief Class pointer
    typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
private:
    bool m_loading; //!< @brief @c true for read-only access, @c false for write-only access
    Stream::Ptr m_stream; //!< @brief The associated BinStream for storage
public:
    /**
     * @brief Constructor
     * @param[in] stream The storage stream
     */
    AbstractArchive( const Stream::Ptr& stream );
    virtual ~AbstractArchive() { }
    /**
     * @brief Whether this archive is read-only
     * @return m_loading
     * @see isSaving()
     */
    bool isLoading() const ;
    /**
     * @brief Whether this archive is write-only
     * @return !m_loading
     * @see isLoading()
     */
    bool isSaving() const ;
// the pragma is used to get rid of the following annoying GCC message:
// warning: ‘AbstractArchive& AbstractArchive::operator%(T&)’ should return by value [-Weffc++]
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
    inline AbstractArchive& operator%( ISerializable* data ) {
        return archive( data );
    }
    inline AbstractArchive& operator%( ISerializable& data ) {
        return archive( &data );
    }
    /**
     * @brief Serialization operator
     * @tparam T Data type
     * @param[in,out] data Data to save or load
     * @return Reference to *this
     * @note Operation depends on m_loading
     */
    template<class T>
    inline
    // This neat piece of template meta-programming is necessary so that this operator
    // does not overwrite the above overloads for ISerializable.
    typename std::enable_if < !std::is_base_of<ISerializable, typename std::remove_pointer<T>::type >::value, AbstractArchive& >::type
    operator%( T& data ) {
        static_assert( std::has_trivial_copy_assign<T>::value, "Data to serialize must be trivially copyable" );
        static_assert( !std::is_pointer<T>::value, "Data to serialize must not be a pointer" );
        if( m_loading ) {
            *m_stream >> data;
        }
        else {
            *m_stream << data;
        }
        return *this;
    }
#pragma GCC diagnostic pop
    /**
     * @brief Serialization operator for arrays
     * @tparam T Data type
     * @param[in,out] data Data array to save or load
     * @param[in] count Number of elements in @a data
     * @return Reference to *this
     * @note Operation depends on m_loading
     */
    template<class T>
    inline AbstractArchive& array( T* data, size_t count ) {
        static_assert( std::has_trivial_copy_assign<T>::value, "Array to serialize must be trivially copyable" );
        static_assert( !std::is_pointer<T>::value, "Array to serialize must not be a pointer" );
        BOOST_ASSERT( data != nullptr );
        if( m_loading ) {
            m_stream->read( data, count );
        }
        else {
            m_stream->write( data, count );
        }
        return *this;
    }
    /**
     * @brief Serialization operator for other archives
     * @param[in,out] data Archive to save or load
     * @return Reference to *this
     * @note Operation depends on m_loading
     */
    AbstractArchive& archive( ISerializable* data ) ;
    /**
     * @brief Finish saving operation
     * @details
     * Resets the stream pointer to 0 and sets m_loading to @c true.
     * @see finishLoad()
     */
    void finishSave();
    /**
     * @brief Finish loading operation
     * @details
     * Resets the stream pointer to 0. The archive remains read-only.
     * @see finishSave()
     */
    void finishLoad();
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
template<>
inline AbstractArchive& AbstractArchive::operator%( std::string& str )
{
    if( !m_loading ) {
        size_t len = str.length();
        m_stream->write( &len );
        m_stream->write( str.data(), len );
    }
    else {
        size_t len = 0;
        m_stream->read( &len );
        str.resize( len );
        m_stream->read( &str.front(), len );
    }
    return *this;
}
#pragma GCC diagnostic pop

#endif
