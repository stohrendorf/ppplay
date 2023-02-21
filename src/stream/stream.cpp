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

#include "stream.h"

Stream::Stream(std::iostream* stream, const std::string& name)
  : m_stream( stream ), m_name( name )
{
}

Stream::~Stream()
{
  delete m_stream;
}

void Stream::clear()
{
  m_stream->clear();
}

void Stream::seek(std::streamoff pos)
{
  m_stream->seekg( pos );
  m_stream->seekp( pos );
}

void Stream::seekrel(std::streamoff delta)
{
  auto p = pos();
  m_stream->seekg( p + delta );
  m_stream->seekp( p + delta );
}

std::streamoff Stream::pos() const
{
  return m_stream->tellg();
}

const std::iostream* Stream::stream() const
{
  return m_stream;
}

std::iostream* Stream::stream()
{
  return m_stream;
}

std::string Stream::name() const
{
  return m_name;
}

void Stream::setName(const std::string& name)
{
  m_name = name;
}