/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#include "archivefilestream.h"

#include <archive.h>
#include <archive_entry.h>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

#include "light4cxx/logger.h"

namespace
{
light4cxx::Logger* logger()
{
    return light4cxx::Logger::get( "ArchiveFileStream" );
}
}

ArchiveFileStream::ArchiveFileStream( const std::string& filename ) : MemoryStream( filename ), m_isOpen( false )
{
    boost::filesystem::path zipPath( filename );
    boost::filesystem::path zipFilePath;
    while( zipPath.has_parent_path() && !boost::filesystem::is_regular_file( zipPath ) ) {
        zipFilePath = zipPath.filename() / zipFilePath;
        zipPath.remove_filename();
    }
    if( !boost::filesystem::is_regular_file( zipPath ) ) {
        logger()->warn( L4CXX_LOCATION, "Failed to open '%s'", filename );
        return;
    }
    logger()->trace( L4CXX_LOCATION, "Decomposed '%s': ZIP '%s', File '%s', using %s", filename, zipPath.string(), zipFilePath.string(), archive_version_string() );

    archive* arch = archive_read_new();
    archive_read_support_format_all( arch );
    archive_read_support_filter_all( arch );
#ifdef BOOST_WINDOWS_API
    char arcPath[260];
    wcstombs( arcPath, zipPath.c_str(), 260 );
    if( ARCHIVE_OK == archive_read_open_filename( arch, arcPath, 1 << 16 ) ) {
#else
    if( ARCHIVE_OK == archive_read_open_filename( arch, zipPath.c_str(), 1 << 16 ) ) {
#endif
        logger()->trace( L4CXX_LOCATION, "Opened '%s'", filename );
        archive_entry* entry;
        while( ARCHIVE_OK == archive_read_next_header( arch, &entry ) ) {
            boost::filesystem::path current( archive_entry_pathname( entry ) );
            if( zipFilePath == "." || current == zipFilePath ) {
                setName( current.string() );
                std::stringstream* str = static_cast<std::stringstream*>( stream() );
                auto size = archive_entry_size( entry );
                std::vector<char> data(size);
                archive_read_data( arch, data.data(), size );
                str->write( data.data(), size );
                break;
            }
        }
    }
    else {
        logger()->error( L4CXX_LOCATION, "Failed to open '%s'", filename );
    }
    archive_read_free( arch );

    m_isOpen = true;
}
