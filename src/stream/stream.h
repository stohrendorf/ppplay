/*
    PPPlay - an old-fashioned module player
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

#ifndef PPPLAY_STREAM_H
#define PPPLAY_STREAM_H

#include <stuff/utils.h>
#include <stream/ppplay_stream_export.h>

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

/**
 * @class Stream
 * @ingroup Common
 * @brief A binary stream helper
 */
class PPPLAY_STREAM_EXPORT Stream
{
    DISABLE_COPY(Stream)
        Stream() = delete;
public:
    typedef std::shared_ptr<Stream> Ptr; //!< @brief Class pointer
private:
    std::iostream* m_stream; //!< @brief The IO Stream associated with this BinStream
    std::string m_name;
public:
    /**
     * @brief Default constructor
     * @param[in] stream Shared pointer to the IO Stream to associate with this BinStream
     */
    explicit Stream(std::iostream* stream, const std::string& name = "Stream");
    /**
     * @brief Destructor
     */
    virtual ~Stream();
    /**
     * @brief Read data from the stream
     * @tparam TR Data type
     * @param[out] data Pointer to the data array
     * @param[in] count Count of data elements (NOT the byte size)
     * @return Reference to *this for pipelining
     */
    template<typename T>
    inline Stream& read(T* data, size_t count = 1)
    {
        static_assert(std::is_trivially_copy_assignable<T>::value, "Data to read must be trivially copyable");
        static_assert(!std::is_pointer<T>::value, "Data to read must not be a pointer");
        m_stream->read(reinterpret_cast<char*>(data), count * sizeof(T));
        return *this;
    }
    /**
     * @brief Write data to the stream
     * @tparam T Data type
     * @param[in] data Pointer to the data array
     * @param[in] count Count of data elements (NOT the byte size)
     * @return Reference to *this for pipelining
     */
    template<typename T>
    inline Stream& write(const T* data, size_t count = 1)
    {
        static_assert(std::is_trivially_copy_assignable<T>::value, "Data to write must be trivially copyable");
        static_assert(!std::is_pointer<T>::value, "Data to write must not be a pointer");
        m_stream->write(reinterpret_cast<const char*>(data), count * sizeof(T));
        return *this;
    }
    /**
     * @brief Clear the failbits of the IO Stream
     */
    void clear();
    /**
     * @brief Seek to a stream position
     * @param[in] pos Position to seek to
     */
    void seek(std::streamoff pos);
    /**
     * @brief Seek to a relative stream position
     * @param[in] delta Relative seek position
     */
    void seekrel(std::streamoff delta);
    /**
     * @brief Get the stream position
     * @return The IO Stream position
     */
    std::streamoff pos() const;
    /**
     * @brief Const access to the internal stream
     * @return BinStream::m_stream
     */
    const std::iostream* stream() const;
    /**
     * @brief Access to the internal stream
     * @return BinStream::m_stream
     */
    std::iostream* stream();
    /**
     * @brief Returns the size of the underlying stream
     * @return The stream size
     */
    virtual std::streamsize size() const = 0;
    virtual std::string name() const;

    inline operator bool() const
    {
        return good();
    }
    inline bool good() const
    {
        return m_stream && m_stream->good();
    }

    template<class T>
    friend inline Stream& operator >> (Stream& str, T& data)
    {
        return str.read(&data);
    }

    template<class T>
    friend inline Stream& operator<<(Stream& str, const T& data)
    {
        return str.write(&data);
    }

protected:
    void setName(const std::string& name);
};

#endif // binstreamH
