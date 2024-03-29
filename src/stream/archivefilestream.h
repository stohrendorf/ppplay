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

#ifndef PPPLAY_ARCHIVEFILESTREAM_H
#define PPPLAY_ARCHIVEFILESTREAM_H

#include "memorystream.h"

/**
 * @class ArchiveFileStream
 * @ingroup Common
 * @brief Class derived from MemoryStream for an archive file
 */
class ArchiveFileStream
  : public MemoryStream
{
public:
  DISABLE_COPY( ArchiveFileStream )

  explicit ArchiveFileStream(const std::string& filename);

  bool isOpen() const
  {
    return m_isOpen;
  }

private:
  bool m_isOpen;
};

#endif
