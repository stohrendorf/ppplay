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

#ifndef H_OSS
#define H_OSS

#include "output.h"

class OSSPlayer: public EmuPlayer
{
public:
  OSSPlayer(Copl *nopl, const char *device, unsigned char bits, int channels,
	    int freq, unsigned long bufsize);
  virtual ~OSSPlayer();

protected:
  virtual void output(const void *buf, unsigned long size);

private:
  int		audio_fd;	// audio device file
  unsigned long	size;		// audio buffer size in bytes
};

#endif
