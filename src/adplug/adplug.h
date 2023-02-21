#pragma once

/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * adplug.h - AdPlug main header file, by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"
#include "players.h"

#include "ymf262/opl3.h"

class AdPlug
{
public:
  DISABLE_COPY( AdPlug )

  static const Players s_players;

  static std::shared_ptr<Player> factory(const std::string& fn, const Players& pl = s_players);

private:
  static const PlayerDesc allplayers[];

  static const Players& init_players(const PlayerDesc pd[]);
};
