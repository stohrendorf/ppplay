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
 * player.h - Replayer base class, by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_PLAYER
#define H_ADPLUG_PLAYER

#include <string>

#include "fprovide.h"
#include "database.h"
#include "ymf262/opl3.h"
#include "stuff/utils.h"

class CPlayer {
  DISABLE_COPY(CPlayer)
public:
  static constexpr auto SampleRate = opl::Opl3::SampleRate;
  CPlayer();
  virtual ~CPlayer() = default;

  /***** Operational methods *****/
  virtual bool load(const std::string &filename, // loads file
                    const CFileProvider &fp = CProvider_Filesystem()) = 0;
  virtual bool update() = 0;                 // executes replay code for 1 tick
  virtual void rewind(int subsong = -1) = 0; // rewinds to specified subsong
  virtual size_t framesUntilUpdate() = 0;

  /***** Informational methods *****/
  virtual std::string gettype() = 0; // returns file type
  virtual std::string gettitle()     // returns song title
      {
    return std::string();
  }
  virtual std::string getauthor() // returns song author name
      {
    return std::string();
  }
  virtual std::string getdesc() // returns song description
      {
    return std::string();
  }
  virtual unsigned int getpatterns() // returns number of patterns
      {
    return 0;
  }
  virtual unsigned int getpattern() // returns currently playing pattern
      {
    return 0;
  }
  virtual unsigned int getorders() // returns size of orderlist
      {
    return 0;
  }
  virtual unsigned int getorder() // returns currently playing song position
      {
    return 0;
  }
  virtual unsigned int getrow() // returns currently playing row
      {
    return 0;
  }
  virtual unsigned int getspeed() // returns current song speed
      {
    return 0;
  }
  virtual unsigned int getsubsongs() // returns number of subsongs
      {
    return 1;
  }
  virtual unsigned int getsubsong() // returns current subsong
      {
    return 0;
  }
  virtual unsigned int getinstruments() // returns number of instruments
      {
    return 0;
  }
  virtual std::string
  getinstrument(unsigned int) // returns n-th instrument name
      {
    return std::string();
  }

  opl::Opl3 *getOpl() { return &m_oplChip; }
  virtual void read(std::array<int16_t, 4> *data) { m_oplChip.read(data); }

private:
  opl::Opl3 m_oplChip;

protected:
  CAdPlugDatabase *m_db; // AdPlug Database

  static const unsigned short m_noteTable[12]; // standard adlib note table
  static const unsigned char
      m_opTable[9]; // the 9 operators as expected by the OPL
};

#endif
