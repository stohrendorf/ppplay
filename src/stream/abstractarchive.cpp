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

#include "abstractarchive.h"
#include "iserializable.h"

AbstractArchive::AbstractArchive( const Stream::Ptr& stream ) : m_loading( false ), m_stream( stream )
{ }

bool AbstractArchive::isLoading() const
{
    return m_loading;
}

bool AbstractArchive::isSaving() const
{
    return !m_loading;
}

AbstractArchive& AbstractArchive::archive( ISerializable* data )
{
    BOOST_ASSERT( data != nullptr );
    return data->serialize( this );
}

void AbstractArchive::finishSave()
{
    BOOST_ASSERT( !m_loading );
    m_stream->seek( 0 );
    m_loading = true;
}

void AbstractArchive::finishLoad()
{
    BOOST_ASSERT( m_loading );
    m_stream->seek( 0 );
}

