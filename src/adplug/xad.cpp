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

    m_tune.resize(f.size() - 80);
    f.read(m_tune.data(), m_tune.size());

    ret = xadplayer_load();

    if (ret)
        rewind(0);

    return ret;
}

void CxadPlayer::rewind(int subsong) {
    setCurrentSpeed(m_xadHeader.speed);
    m_xadSpeedCounter = 1;
    m_xadPlaying = true;
    m_xadLooping = false;

    // rewind()
    xadplayer_rewind(subsong);

#ifdef DEBUG
    AdPlug_LogWrite("-----------\n");
#endif
}

bool CxadPlayer::update() {
    if (--m_xadSpeedCounter == 0) {
        m_xadSpeedCounter = currentSpeed();

        // update()
        xadplayer_update();
    }

    return m_xadPlaying && !m_xadLooping;
}

