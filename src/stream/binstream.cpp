/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#include "binstream.h"

FBinStream::FBinStream( const std::string& filename ) :
	BinStream( SpIoStream( new std::fstream( filename.c_str(), std::ios::in | std::ios::binary ) ) ),
	m_filename( filename ) {
}

FBinStream::~FBinStream() {
	std::static_pointer_cast<std::fstream>( stream() )->close();
}

bool FBinStream::isOpen() const {
	return std::static_pointer_cast<std::fstream>( stream() )->is_open();
}

template<typename TR>
BinStream& BinStream::read( TR* data, const std::size_t count ) {
	m_stream->read( reinterpret_cast<char*>( data ), sizeof( TR )*count );
	return *this;
}
template<typename TW>
BinStream& BinStream::write( const TW* data, const std::size_t count ) {
	m_stream->write( reinterpret_cast<const char*>( data ), sizeof( TW )*count );
	return *this;
}

#define BINSTREAM_RW_IMPL(tn)\
	template BinStream &BinStream::read<tn>(tn *, std::size_t); \
	template BinStream &BinStream::write<tn>(const tn*, std::size_t);

BINSTREAM_RW_IMPL( int8_t )
BINSTREAM_RW_IMPL( uint8_t )
BINSTREAM_RW_IMPL( int16_t )
BINSTREAM_RW_IMPL( uint16_t )
BINSTREAM_RW_IMPL( int32_t )
BINSTREAM_RW_IMPL( uint32_t )
BINSTREAM_RW_IMPL( int64_t )
BINSTREAM_RW_IMPL( uint64_t )
BINSTREAM_RW_IMPL( char )
BINSTREAM_RW_IMPL( bool )
BINSTREAM_RW_IMPL( float )

ISerializable::~ISerializable()
{ }

IArchive::IArchive( const BinStream::Ptr& stream ) : m_loading( false ), m_stream( stream )
{ }

IArchive::~IArchive()
{ }

MemArchive::MemArchive() : IArchive( BinStream::Ptr( new SBinStream() ) )
{ }

MemArchive::~MemArchive()
{ }

bool BinStream::fail() const
{
    return m_stream->fail();
}

bool BinStream::good() const
{
    return m_stream->good();
}

void BinStream::clear()
{
    m_stream->clear();
}

void BinStream::seek(uint32_t pos)
{
    m_stream->seekg(pos);
    m_stream->seekp(pos);
}

void BinStream::seekrel(int32_t delta)
{
    uint32_t p = pos();
    m_stream->seekg(p + delta);
    m_stream->seekp(p + delta);
}

uint32_t BinStream::pos() const
{
    return m_stream->tellg();
}

const BinStream::SpIoStream &BinStream::stream() const
{
    return m_stream;
}

BinStream::SpIoStream &BinStream::stream()
{
    return m_stream;
}
