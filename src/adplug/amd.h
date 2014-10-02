/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
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
 * amd.h - AMD Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "protrack.h"

class CamdLoader : public CmodPlayer {
  DISABLE_COPY(CamdLoader)
public:
  static CPlayer *factory();

  CamdLoader() : CmodPlayer() {}
  ;

  bool load(const std::string &filename, const CFileProvider &fp);
  size_t framesUntilUpdate();

  std::string gettype() { return std::string("AMUSIC Adlib Tracker"); }
  ;
  std::string gettitle() { return std::string(songname, 0, 24); }
  ;
  std::string getauthor() { return std::string(author, 0, 24); }
  ;
  unsigned int getinstruments() { return 26; }
  ;
  std::string getinstrument(unsigned int n) {
    return std::string(m_instrumentNames[n], 0, 23);
  }
  ;

private:
  char songname[24], author[24], m_instrumentNames[26][23];
};