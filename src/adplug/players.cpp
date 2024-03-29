/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * players.h - Players enumeration, by Simon Peter <dn.tlp@gmx.net>
 */

#include <boost/algorithm/string.hpp>

#include "players.h"

/***** CPlayerDesc *****/

PlayerDesc::PlayerDesc(Factory f, const std::string& type, const std::vector<std::string>& ext)
  : factory( f ), filetype( type ), extensions( ext )
{
}

std::string PlayerDesc::get_extension(size_t n) const
{
  if( n >= extensions.size() )
  {
    return std::string();
  }
  return extensions[n];
}

