/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2003 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
 */

#ifndef H_OUTPUT
#define H_OUTPUT

#include <adplug/player.h>

class Player
{
public:
  CPlayer	*p;
  bool		playing;

  Player();
  virtual ~Player();

  virtual void frame() = 0;
  virtual Copl *get_opl() = 0;
};

class EmuPlayer: public Player
{
private:
  Copl		*opl;
  char		*audiobuf;
  unsigned long	buf_size, freq;
  unsigned char	bits, channels;

public:
  EmuPlayer(Copl *nopl, unsigned char nbits, unsigned char nchannels,
	    unsigned long nfreq, unsigned long nbufsize);
  virtual ~EmuPlayer();

  virtual void setbufsize(unsigned long nbufsize);
  virtual void frame();
  virtual Copl *get_opl() { return opl; }

protected:
  virtual void output(const void *buf, unsigned long size) = 0;
  // The output buffer is always of the size requested through the constructor.
  // This time, size is measured in bytes, not samples!

  unsigned char getsampsize() { return (channels * (bits / 8)); }
};

#endif
