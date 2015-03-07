/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * adl.h - ADL player adaption by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_ADLPLAYER
#define H_ADPLUG_ADLPLAYER

#include "player.h"

class AdlibDriver;

class CadlPlayer : public CPlayer {
  DISABLE_COPY(CadlPlayer)
public:
  static CPlayer *factory();

  CadlPlayer();
  ~CadlPlayer();

  bool load(const std::string &filename);
  bool update();
  void rewind(int subsong = -1);

  // refresh rate is fixed at 72Hz
  size_t framesUntilUpdate() override { return SampleRate / 72; }

  unsigned int getsubsongs();
  unsigned int getsubsong() { return cursubsong; }
  std::string gettype() { return std::string("Westwood ADL"); }

private:
  int numsubsongs = 0;
  int cursubsong = 0;

  AdlibDriver* _driver = nullptr;

  std::array<uint8_t, 120> _trackEntries{};
  std::vector<uint8_t> _soundDataPtr{};
  int _sfxPlayingSound = -1;

  uint8_t _sfxPriority = 0;
  uint8_t _sfxFourthByteOfSong = 0;

  bool init();
  void process();
  void playTrack(uint8_t track);
  void playSoundEffect(uint8_t track);
  void play(uint8_t track);
  void unk1();
  void unk2();
};

#endif
