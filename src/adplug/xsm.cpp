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
 * xsm.cpp - eXtra Simple Music Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include <string.h>

#include "xsm.h"

CxsmPlayer::CxsmPlayer()
  : CPlayer(), music(0)
{
}

CxsmPlayer::~CxsmPlayer()
{
  if(music) delete [] music;
}

bool CxsmPlayer::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;
  char			id[6];
  int			i, j;

  // check if header matches
  f->readString(id, 6); songlen = f->readInt(2);
  if(strncmp(id, "ofTAZ!", 6) || songlen > 3200) { fp.close(f); return false; }

  // read and set instruments
  for(i = 0; i < 9; i++) {
    getOpl()->writeReg(0x20 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0x23 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0x40 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0x43 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0x60 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0x63 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0x80 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0x83 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0xe0 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0xe3 + m_opTable[i], f->readInt(1));
    getOpl()->writeReg(0xc0 + m_opTable[i], f->readInt(1));
    f->ignore(5);
  }

  // read song data
  music = new char [songlen * 9];
  for(i = 0; i < 9; i++)
    for(j = 0; j < songlen; j++)
      music[j * 9 + i] = f->readInt(1);

  // success
  fp.close(f);
  rewind(0);
  return true;
}

bool CxsmPlayer::update()
{
  int c;

  if(notenum >= songlen) {
    songend = true;
    notenum = last = 0;
  }

  for(c = 0; c < 9; c++)
    if(music[notenum * 9 + c] != music[last * 9 + c])
      getOpl()->writeReg(0xb0 + c, 0);

  for(c = 0; c < 9; c++) {
    if(music[notenum * 9 + c])
      play_note(c, music[notenum * 9 + c] % 12, music[notenum * 9 + c] / 12);
    else
      play_note(c, 0, 0);
  }

  last = notenum;
  notenum++;
  return !songend;
}

void CxsmPlayer::rewind(int)
{
  notenum = last = 0;
  songend = false;
}

size_t CxsmPlayer::framesUntilUpdate()
{
  return SampleRate/5;
}

void CxsmPlayer::play_note(int c, int note, int octv)
{
  int freq = m_noteTable[note];

  if(!note && !octv) freq = 0;
  getOpl()->writeReg(0xa0 + c, freq & 0xff);
  getOpl()->writeReg(0xb0 + c, (freq / 0xff) | 32 | (octv * 4));
}
