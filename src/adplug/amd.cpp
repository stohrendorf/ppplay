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
 * amd.cpp - AMD Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include <string.h>

#include "amd.h"
#include "debug.h"

CPlayer *CamdLoader::factory()
{
  return new CamdLoader();
}

bool CamdLoader::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;
  struct {
    char id[9];
    unsigned char version;
  } header;
  int i, j, k, t, numtrax, maxi = 0;
  unsigned char buf, buf2, buf3;
  const unsigned char convfx[10] = {0,1,2,9,17,11,13,18,3,14};
  const unsigned char convvol[64] = {
    0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 0xa, 0xa, 0xb,
    0xc, 0xc, 0xd, 0xe, 0xe, 0xf, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x21,
    0x22, 0x23, 0x25, 0x26, 0x28, 0x29, 0x2b, 0x2d, 0x2e, 0x30, 0x32, 0x35,
    0x37, 0x3a, 0x3c, 0x3f
  };

  // file validation section
  if(fp.filesize(f) < 1072) { fp.close(f); return false; }
  f->seek(1062); f->readString(header.id, 9);
  header.version = f->readInt(1);
  if(strncmp(header.id, "<o\xefQU\xeeRoR", 9) &&
     strncmp(header.id, "MaDoKaN96", 9)) { fp.close(f); return false; }

  // load section
  memset(inst, 0, sizeof(inst));
  f->seek(0);
  f->readString(songname, sizeof(songname));
  f->readString(author, sizeof(author));
  for(i = 0; i < 26; i++) {
    f->readString(instname[i], 23);
    for(j = 0; j < 11; j++) inst[i].data[j] = f->readInt(1);
  }
  m_length = f->readInt(1); nop = f->readInt(1) + 1;
  for(i=0;i<128;i++) m_order[i] = f->readInt(1);
  f->seek(10, binio::Add);
  if(header.version == 0x10) {	// unpacked module
    maxi = nop * 9;
    for(i=0;i<64*9;i++)
      trackord[i/9][i%9] = i+1;
    t = 0;
    while(!f->ateof()) {
      for(j=0;j<64;j++)
	for(i=t;i<t+9;i++) {
	  buf = f->readInt(1);
	  m_tracks[i][j].param2 = (buf&127) % 10;
	  m_tracks[i][j].param1 = (buf&127) / 10;
	  buf = f->readInt(1);
	  m_tracks[i][j].inst = buf >> 4;
	  m_tracks[i][j].command = buf & 0x0f;
	  buf = f->readInt(1);
	  if(buf >> 4)	// fix bug in AMD save routine
	    m_tracks[i][j].note = ((buf & 14) >> 1) * 12 + (buf >> 4);
	  else
	    m_tracks[i][j].note = 0;
	  m_tracks[i][j].inst += (buf & 1) << 4;
	}
      t += 9;
    }
  } else {			// packed module
    for(i=0;i<nop;i++)
      for(j=0;j<9;j++)
	trackord[i][j] = f->readInt(2) + 1;
    numtrax = f->readInt(2);
    for(k=0;k<numtrax;k++) {
      i = f->readInt(2);
      if(i > 575) i = 575;	// fix corrupted modules
      maxi = (i + 1 > maxi ? i + 1 : maxi);
      j = 0;
      do {
	buf = f->readInt(1);
	if(buf & 128) {
	  for(t = j; t < j + (buf & 127) && t < 64; t++) {
	    m_tracks[i][t].command = 0;
	    m_tracks[i][t].inst = 0;
	    m_tracks[i][t].note = 0;
	    m_tracks[i][t].param1 = 0;
	    m_tracks[i][t].param2 = 0;
	  }
	  j += buf & 127;
	  continue;
	}
	m_tracks[i][j].param2 = buf % 10;
	m_tracks[i][j].param1 = buf / 10;
	buf = f->readInt(1);
	m_tracks[i][j].inst = buf >> 4;
	m_tracks[i][j].command = buf & 0x0f;
	buf = f->readInt(1);
	if(buf >> 4)	// fix bug in AMD save routine
	  m_tracks[i][j].note = ((buf & 14) >> 1) * 12 + (buf >> 4);
	else
	  m_tracks[i][j].note = 0;
	m_tracks[i][j].inst += (buf & 1) << 4;
	j++;
      } while(j<64);
    }
  }
  fp.close(f);

  // convert to protracker replay data
  m_bpm = 50; m_restartpos = 0; m_flags = Decimal;
  for(i=0;i<26;i++) {	// convert instruments
    buf = inst[i].data[0];
    buf2 = inst[i].data[1];
    inst[i].data[0] = inst[i].data[10];
    inst[i].data[1] = buf;
    buf = inst[i].data[2];
    inst[i].data[2] = inst[i].data[5];
    buf3 = inst[i].data[3];
    inst[i].data[3] = buf;
    buf = inst[i].data[4];
    inst[i].data[4] = inst[i].data[7];
    inst[i].data[5] = buf3;
    buf3 = inst[i].data[6];
    inst[i].data[6] = inst[i].data[8];
    inst[i].data[7] = buf;
    inst[i].data[8] = inst[i].data[9];
    inst[i].data[9] = buf2;
    inst[i].data[10] = buf3;
    for(j=0;j<23;j++)	// convert names
      if(instname[i][j] == '\xff')
	instname[i][j] = '\x20';
  }
  for(i=0;i<maxi;i++)	// convert patterns
    for(j=0;j<64;j++) {
      m_tracks[i][j].command = convfx[m_tracks[i][j].command];
      // extended command
      if(m_tracks[i][j].command == 14) {
	if(m_tracks[i][j].param1 == 2) {
	  m_tracks[i][j].command = 10;
	  m_tracks[i][j].param1 = m_tracks[i][j].param2;
	  m_tracks[i][j].param2 = 0;
	}

	if(m_tracks[i][j].param1 == 3) {
	  m_tracks[i][j].command = 10;
	  m_tracks[i][j].param1 = 0;
	}
      }

      // fix volume
      if(m_tracks[i][j].command == 17) {
	int vol = convvol[m_tracks[i][j].param1 * 10 + m_tracks[i][j].param2];

	if(vol > 63) vol = 63;
	m_tracks[i][j].param1 = vol / 10;
	m_tracks[i][j].param2 = vol % 10;
      }
    }

  rewind(0);
  return true;
}

size_t CamdLoader::framesUntilUpdate()
{
  if(m_tempo)
    return SampleRate/m_tempo;
  else
    return SampleRate/18.2;
}
