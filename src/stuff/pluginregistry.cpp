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

#include "pluginregistry.h"
#include "stream/archivefilestream.h"
#include "stream/filestream.h"
#include "stuff/system.h"

#include "xmmod/xmmodule.h"
#include "itmod/itmodule.h"
#include "s3mmod/s3mmodule.h"
#include "modmod/modmodule.h"
#include "hscmod/hscmodule.h"

namespace ppp
{
AbstractModule::Ptr tryLoad(const std::string& filename, uint32_t frq, int maxRpt, Sample::Interpolation inter)
{
    static const auto plugins = {
            &xm::XmModule::factory,
            &it::ItModule::factory,
            &s3m::S3mModule::factory,
            &mod::ModModule::factory,
            &hsc::Module::factory
    };

    {
        FileStream file( filename );
        if( file.isOpen() )
        {
            for( auto plugin : plugins )
            {
                file.clear();
                file.seek( 0 );
                if( auto result = (*plugin)( &file, frq, maxRpt, inter ) )
                {
                    return result;
                }
            }
        }
    }

    {
        ArchiveFileStream file( filename );
        if( file.isOpen() )
        {
            for( auto plugin : plugins )
            {
                file.clear();
                file.seek( 0 );
                if( auto result = (*plugin)( &file, frq, maxRpt, inter ) )
                {
                    return result;
                }
            }
        }
    }
    return nullptr;
}
}
