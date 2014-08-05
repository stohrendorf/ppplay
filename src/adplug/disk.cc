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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libbinio/binwrap.h>
#include <libbinio/binfile.h>

#include "defines.h"
#include "disk.h"

#define BUFSIZE		512

DiskWriter::DiskWriter(Copl *nopl, const char *filename, unsigned char nbits,
		       unsigned char nchannels, unsigned long nfreq)
  : EmuPlayer(nopl, nbits, nchannels, nfreq, BUFSIZE), f(0), samplesize(0)
{
  if(!filename) {
    message(MSG_ERROR, "no output filename specified");
    exit(EXIT_FAILURE);
  }

  // If filename is '-', output to stdout
  if(strcmp(filename, "-"))
    f = new binofstream(filename);
  else
    f = new binowstream(&std::cout);	// not very good to mix cout with stdout

  if(!f || f->error()) {
    message(MSG_ERROR, "cannot open file for output -- %s", filename);
    if(f) delete f;
    exit(EXIT_FAILURE);
  }

  f->setFlag(binio::BigEndian, false);

  // Write Microsoft RIFF WAVE header
  f->writeString("RIFF", 4); f->writeInt(36, 4); f->writeString("WAVEfmt ", 8);
  f->writeInt(16, 4); f->writeInt(1, 2); f->writeInt(nchannels, 2);
  f->writeInt(nfreq, 4); f->writeInt(nfreq * getsampsize(), 4);
  f->writeInt(getsampsize(), 2); f->writeInt(nbits, 2);
  f->writeString("data", 4); f->writeInt(0, 4);
}

DiskWriter::~DiskWriter()
{
  if(!f) return;

  if(samplesize % 2) { // Wave data must end on an even byte boundary
    f->writeInt(0, 1);
    samplesize++;
  }

  // Write file sizes
  f->seek(40); f->writeInt(samplesize, 4);
  samplesize += 36; // make absolute filesize (add header size)
  f->seek(4); f->writeInt(samplesize, 4);

  // end disk writing
  delete f;
}

void DiskWriter::output(const void *buf, unsigned long size)
{
  char		*b = (char *)buf;
  unsigned long	i, ssize = getsampsize();

  for(i = 0; i < size; i += ssize)
    switch(ssize) {
    case 1: f->writeInt(*(b + i), 1); break;
    case 2: f->writeInt(*(short *)(b + i), 2); break;
    case 4: f->writeInt(*(long *)(b + i), 4); break;
    }

  samplesize += size;
}
