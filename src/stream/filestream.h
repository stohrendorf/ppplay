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

#ifndef PPPLAY_FILESTREAM_H
#define PPPLAY_FILESTREAM_H

#include "stream.h"

#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/case_conv.hpp>

/**
 * @class FileStream
 * @ingroup Common
 * @brief Class derived from Stream for files
 * @note This is a read-only stream
 */
class PPPLAY_STREAM_EXPORT FileStream : public Stream
{
    DISABLE_COPY(FileStream)
        FileStream() = delete;
private:
    size_t m_size; //!< @brief Cached size of the file
public:
    enum class Mode
    {
        Read, Write
    };
    /**
     * @brief Default contructor
     * @param[in] filename Filename of the file to open
     */
    explicit FileStream(const std::string& filename, Mode mode = Mode::Read);
    /**
     * @brief Check if the file is opened
     * @return @c true if the file is opened
     */
    bool isOpen() const;
    std::streamsize size() const override;

    std::string extension() const
    {
        return boost::algorithm::to_lower_copy(boost::filesystem::path(name()).extension().string());
    }
};

#endif
