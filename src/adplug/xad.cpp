/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  xad.cpp - XAD shell player by Riven the Mage <riven@ok.ru>
*/

#include "xad.h"
#include "debug.h"

#include "stream/filestream.h"

/* -------- Public Methods -------------------------------- */

bool CxadPlayer::load(const std::string &filename) {
    FileStream f(filename);
    if (!f)
        return false;
    bool ret = false;

    // load header
    f >> m_xadHeader;

    // 'XAD!' - signed ?
    if (m_xadHeader.id != 0x21444158) {
        return false;
    }

    tune.resize(f.size() - 80);
    f.read(tune.data(), tune.size());

    ret = xadplayer_load();

    if (ret)
        rewind(0);

    return ret;
}

void CxadPlayer::rewind(int subsong) {
    plr.speed = m_xadHeader.speed;
    plr.speed_counter = 1;
    plr.playing = 1;
    plr.looping = 0;

    // rewind()
    xadplayer_rewind(subsong);

#ifdef DEBUG
    AdPlug_LogWrite("-----------\n");
#endif
}

bool CxadPlayer::update() {
    if (--plr.speed_counter)
        goto update_end;

    plr.speed_counter = plr.speed;

    // update()
    xadplayer_update();

update_end:
    return (plr.playing && (!plr.looping));
}

size_t CxadPlayer::framesUntilUpdate() {
    return SampleRate / xadplayer_getrefresh();
}

std::string CxadPlayer::gettype() { return xadplayer_gettype(); }

std::string CxadPlayer::gettitle() { return xadplayer_gettitle(); }

std::string CxadPlayer::getauthor() { return xadplayer_getauthor(); }

std::string CxadPlayer::getinstrument(unsigned int i) {
    return xadplayer_getinstrument(i);
}

unsigned int CxadPlayer::getinstruments() { return xadplayer_getinstruments(); }
