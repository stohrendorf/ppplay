/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2004 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * mkj.cpp - MKJamz Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include <cstring>
#include <assert.h>

#include "mkj.h"
#include "debug.h"

CPlayer *CmkjPlayer::factory()
{
  return new CmkjPlayer();
}

bool CmkjPlayer::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;
  char	id[6];
  float	ver;
  int	i, j;
  short	inst[8];

  // file validation
  f->readString(id, 6);
  if(strncmp(id,"MKJamz",6)) { fp.close(f); return false; }
  ver = f->readFloat(binio::Single);
  if(ver > 1.12) { fp.close(f); return false; }

  // load
  maxchannel = f->readInt(2);
  getOpl()->writeReg(1, 32);
  for(i = 0; i < maxchannel; i++) {
    for(j = 0; j < 8; j++) inst[j] = f->readInt(2);
    getOpl()->writeReg(0x20+m_opTable[i],inst[4]);
    getOpl()->writeReg(0x23+m_opTable[i],inst[0]);
    getOpl()->writeReg(0x40+m_opTable[i],inst[5]);
    getOpl()->writeReg(0x43+m_opTable[i],inst[1]);
    getOpl()->writeReg(0x60+m_opTable[i],inst[6]);
    getOpl()->writeReg(0x63+m_opTable[i],inst[2]);
    getOpl()->writeReg(0x80+m_opTable[i],inst[7]);
    getOpl()->writeReg(0x83+m_opTable[i],inst[3]);
  }
  maxnotes = f->readInt(2);
  songbuf = new short [(maxchannel+1)*maxnotes];
  for(i = 0; i < maxchannel; i++) channel[i].defined = f->readInt(2);
  for(i = 0; i < (maxchannel + 1) * maxnotes; i++)
    songbuf[i] = f->readInt(2);

  AdPlug_LogWrite("CmkjPlayer::load(\"%s\"): loaded file ver %.2f, %d channels,"
		  " %d notes/channel.\n", filename.c_str(), ver, maxchannel,
		  maxnotes);
  fp.close(f);
  rewind(0);
  return true;
}

bool CmkjPlayer::update()
{
  int c, i;
  short note;

  for(c = 0; c < maxchannel; c++) {
    if(!channel[c].defined)	// skip if channel is disabled
      continue;

    if(channel[c].pstat) {
      channel[c].pstat--;
      continue;
    }

    getOpl()->writeReg(0xb0 + c, 0);	// key off
    do {
      assert(channel[c].songptr < (maxchannel + 1) * maxnotes);
      note = songbuf[channel[c].songptr];
      if(channel[c].songptr - c > maxchannel)
	if(note && note < 250)
	  channel[c].pstat = channel[c].speed;
      switch(note) {
	// normal notes
      case 68: getOpl()->writeReg(0xa0 + c,0x81); getOpl()->writeReg(0xb0 + c,0x21 + 4 * channel[c].octave); break;
      case 69: getOpl()->writeReg(0xa0 + c,0xb0); getOpl()->writeReg(0xb0 + c,0x21 + 4 * channel[c].octave); break;
      case 70: getOpl()->writeReg(0xa0 + c,0xca); getOpl()->writeReg(0xb0 + c,0x21 + 4 * channel[c].octave); break;
      case 71: getOpl()->writeReg(0xa0 + c,0x2); getOpl()->writeReg(0xb0 + c,0x22 + 4 * channel[c].octave); break;
      case 65: getOpl()->writeReg(0xa0 + c,0x41); getOpl()->writeReg(0xb0 + c,0x22 + 4 * channel[c].octave); break;
      case 66: getOpl()->writeReg(0xa0 + c,0x87); getOpl()->writeReg(0xb0 + c,0x22 + 4 * channel[c].octave); break;
      case 67: getOpl()->writeReg(0xa0 + c,0xae); getOpl()->writeReg(0xb0 + c,0x22 + 4 * channel[c].octave); break;
      case 17: getOpl()->writeReg(0xa0 + c,0x6b); getOpl()->writeReg(0xb0 + c,0x21 + 4 * channel[c].octave); break;
      case 18: getOpl()->writeReg(0xa0 + c,0x98); getOpl()->writeReg(0xb0 + c,0x21 + 4 * channel[c].octave); break;
      case 20: getOpl()->writeReg(0xa0 + c,0xe5); getOpl()->writeReg(0xb0 + c,0x21 + 4 * channel[c].octave); break;
      case 21: getOpl()->writeReg(0xa0 + c,0x20); getOpl()->writeReg(0xb0 + c,0x22 + 4 * channel[c].octave); break;
      case 15: getOpl()->writeReg(0xa0 + c,0x63); getOpl()->writeReg(0xb0 + c,0x22 + 4 * channel[c].octave); break;
      case 255:	// delay
	channel[c].songptr += maxchannel;
	channel[c].pstat = songbuf[channel[c].songptr];
	break;
      case 254:	// set octave
	channel[c].songptr += maxchannel;
	channel[c].octave = songbuf[channel[c].songptr];
	break;
      case 253:	// set speed
	channel[c].songptr += maxchannel;
	channel[c].speed = songbuf[channel[c].songptr];
	break;
      case 252:	// set waveform
	channel[c].songptr += maxchannel;
	channel[c].waveform = songbuf[channel[c].songptr] - 300;
	if(c > 2)
	  getOpl()->writeReg(0xe0 + c + (c+6),channel[c].waveform);
	else
	  getOpl()->writeReg(0xe0 + c,channel[c].waveform);
	break;
      case 251:	// song end
	for(i = 0; i < maxchannel; i++) channel[i].songptr = i;
	songend = true;
	return false;
      }

      if(channel[c].songptr - c < maxnotes)
	channel[c].songptr += maxchannel;
      else
	channel[c].songptr = c;
    } while(!channel[c].pstat);
  }

  return !songend;
}

void CmkjPlayer::rewind(int)
{
  int i;

  for(i = 0; i < maxchannel; i++) {
    channel[i].pstat = 0;
    channel[i].speed = 0;
    channel[i].waveform = 0;
    channel[i].songptr = i;
    channel[i].octave = 4;
  }

  songend = false;
}

size_t CmkjPlayer::framesUntilUpdate()
{
  return SampleRate/100;
}
