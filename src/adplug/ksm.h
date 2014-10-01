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
 * ksm.h - KSM Player for AdPlug by Simon Peter <dn.tlp@gmx.net>
 */

#include "player.h"

class CksmPlayer : public CPlayer {
  DISABLE_COPY(CksmPlayer)
public:
  static CPlayer *factory();

  CksmPlayer() = default;

  bool load(const std::string &filename, const CFileProvider &fp);
  bool update();
  void rewind(int);
  size_t framesUntilUpdate() override { return SampleRate / 240; }

  std::string gettype() { return "Ken Silverman's Music Format"; }
  unsigned int getinstruments() { return 16; }
  std::string getinstrument(unsigned int n);

private:
  static const unsigned int adlibfreq[63];

  std::vector<uint32_t> note{};
  unsigned long count = 0, countstop = 0, chanage[18] = {0};
  unsigned int nownote = 0, numchans = 0, drumstat = 0;
  uint8_t trinst[16] = {0}, trquant[16] = {0}, trchan[16] = {0}, trvol[16] = {0}, inst[256][11] = {{0}},
      databuf[2048] = {0}, chanfreq[18] = {0}, chantrack[18] = {0};
  char instname[256][20] =  {""};

  bool songend = false;

  void loadinsts(binistream *f);
  void setinst(int chan, unsigned char v0, unsigned char v1, unsigned char v2,
               unsigned char v3, unsigned char v4, unsigned char v5,
               unsigned char v6, unsigned char v7, unsigned char v8,
               unsigned char v9, unsigned char v10);
};
