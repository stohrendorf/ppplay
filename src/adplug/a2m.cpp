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
 * a2m.cpp - A2M Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * This loader detects and loads version 1, 4, 5 & 8 files.
 *
 * version 1-4 files:
 * Following commands are ignored: FF1 - FF9, FAx - FEx
 *
 * version 5-8 files:
 * Instrument panning is ignored. Flags byte is ignored.
 * Following commands are ignored: Gxy, Hxy, Kxy - &xy
 */

#include <cstring>
#include "a2m.h"

const unsigned short Ca2mLoader::m_bitvalue[14] =
  {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};

const signed short Ca2mLoader::m_copybits[COPYRANGES] =
  {4, 6, 8, 10, 12, 14};

const signed short Ca2mLoader::m_copymin[COPYRANGES] =
  {0, 16, 80, 336, 1360, 5456};

CPlayer *Ca2mLoader::factory(opl::Opl3 *newopl)
{
  return new Ca2mLoader(newopl);
}

bool Ca2mLoader::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;
  char id[10];
  int i,j,k,t;
  unsigned int l;
  unsigned char *org, *orgptr, flags = 0, numpats, version;
  unsigned long crc, alength;
  unsigned short len[9], *secdata, *secptr;
  const unsigned char convfx[16] = {0,1,2,23,24,3,5,4,6,9,17,13,11,19,7,14};
  const unsigned char convinf1[16] = {0,1,2,6,7,8,9,4,5,3,10,11,12,13,14,15};
  const unsigned char newconvfx[] = {0,1,2,3,4,5,6,23,24,21,10,11,17,13,7,19,
				     255,255,22,25,255,15,255,255,255,255,255,
				     255,255,255,255,255,255,255,255,14,255};

  // read header
  f->readString(id, 10); crc = f->readInt(4);
  version = f->readInt(1); numpats = f->readInt(1);

  // file validation section
  if(strncmp(id,"_A2module_",10) || (version != 1 && version != 5 &&
				     version != 4 && version != 8)) {
    fp.close(f);
    return false;
  }

  // load, depack & convert section
  nop = numpats; length = 128; restartpos = 0;
  if(version < 5) {
    for(i=0;i<5;i++) len[i] = f->readInt(2);
    t = 9;
  } else {	// version >= 5
    for(i=0;i<9;i++) len[i] = f->readInt(2);
    t = 18;
  }

  // block 0
  secdata = new unsigned short [len[0] / 2];
  if(version == 1 || version == 5) {
    for(i=0;i<len[0]/2;i++) secdata[i] = f->readInt(2);
    org = new unsigned char [MAXBUF]; orgptr = org;
    sixdepak(secdata,org,len[0]);
  } else {
    orgptr = (unsigned char *)secdata;
    for(i=0;i<len[0];i++) orgptr[i] = f->readInt(1);
  }
  memcpy(m_songname,orgptr,43); orgptr += 43;
  memcpy(m_author,orgptr,43); orgptr += 43;
  memcpy(m_instname,orgptr,250*33); orgptr += 250*33;

  for(i=0;i<250;i++) {	// instruments
    inst[i].data[0] = *(orgptr+i*13+10);
    inst[i].data[1] = *(orgptr+i*13);
    inst[i].data[2] = *(orgptr+i*13+1);
    inst[i].data[3] = *(orgptr+i*13+4);
    inst[i].data[4] = *(orgptr+i*13+5);
    inst[i].data[5] = *(orgptr+i*13+6);
    inst[i].data[6] = *(orgptr+i*13+7);
    inst[i].data[7] = *(orgptr+i*13+8);
    inst[i].data[8] = *(orgptr+i*13+9);
    inst[i].data[9] = *(orgptr+i*13+2);
    inst[i].data[10] = *(orgptr+i*13+3);

    if(version < 5)
      inst[i].misc = *(orgptr+i*13+11);
    else {	// version >= 5 -> OPL3 format
      int pan = *(orgptr+i*13+11);

      if(pan)
	inst[i].data[0] |= (pan & 3) << 4;	// set pan
      else
	inst[i].data[0] |= 48;			// enable both speakers
    }

    inst[i].slide = *(orgptr+i*13+12);
  }

  orgptr += 250*13;
  memcpy(order,orgptr,128); orgptr += 128;
  bpm = *orgptr; orgptr++;
  initspeed = *orgptr; orgptr++;
  if(version >= 5) flags = *orgptr;
  if(version == 1 || version == 5) delete [] org;
  delete [] secdata;

  // blocks 1-4 or 1-8
  alength = len[1];
  for(i = 0; i < (version < 5 ? numpats / 16 : numpats / 8); i++)
    alength += len[i+2];

  secdata = new unsigned short [alength / 2];
  if(version == 1 || version == 5) {
    for(l=0;l<alength/2;l++) secdata[l] = f->readInt(2);
    org = new unsigned char [MAXBUF * (numpats / (version == 1 ? 16 : 8) + 1)];
    orgptr = org; secptr = secdata;
    orgptr += sixdepak(secptr,orgptr,len[1]); secptr += len[1] / 2;
    if(version == 1) {
      if(numpats > 16)
	orgptr += sixdepak(secptr,orgptr,len[2]); secptr += len[2] / 2;
      if(numpats > 32)
	orgptr += sixdepak(secptr,orgptr,len[3]); secptr += len[3] / 2;
      if(numpats > 48)
	sixdepak(secptr,orgptr,len[4]);
    } else {
      if(numpats > 8)
	orgptr += sixdepak(secptr,orgptr,len[2]); secptr += len[2] / 2;
      if(numpats > 16)
	orgptr += sixdepak(secptr,orgptr,len[3]); secptr += len[3] / 2;
      if(numpats > 24)
	orgptr += sixdepak(secptr,orgptr,len[4]); secptr += len[4] / 2;
      if(numpats > 32)
	orgptr += sixdepak(secptr,orgptr,len[5]); secptr += len[5] / 2;
      if(numpats > 40)
	orgptr += sixdepak(secptr,orgptr,len[6]); secptr += len[6] / 2;
      if(numpats > 48)
	orgptr += sixdepak(secptr,orgptr,len[7]); secptr += len[7] / 2;
      if(numpats > 56)
	sixdepak(secptr,orgptr,len[8]);
    }
    delete [] secdata;
  } else {
    org = (unsigned char *)secdata;
    for(l=0;l<alength;l++) org[l] = f->readInt(1);
  }

  if(version < 5) {
    for(i=0;i<numpats;i++)
      for(j=0;j<64;j++)
	for(k=0;k<9;k++) {
	  struct Tracks	*track = &tracks[i * 9 + k][j];
	  unsigned char	*o = &org[i*64*t*4+j*t*4+k*4];

	  track->note = o[0] == 255 ? 127 : o[0];
	  track->inst = o[1];
	  track->command = convfx[o[2]];
	  track->param2 = o[3] & 0x0f;
	  if(track->command != 14)
	    track->param1 = o[3] >> 4;
	  else {
	    track->param1 = convinf1[o[3] >> 4];
	    if(track->param1 == 15 && !track->param2) {	// convert key-off
	      track->command = 8;
	      track->param1 = 0;
	      track->param2 = 0;
	    }
	  }
	  if(track->command == 14) {
	    switch(track->param1) {
	    case 2: // convert define waveform
	      track->command = 25;
	      track->param1 = track->param2;
	      track->param2 = 0xf;
	      break;
	    case 8: // convert volume slide up
	      track->command = 26;
	      track->param1 = track->param2;
	      track->param2 = 0;
	      break;
	    case 9: // convert volume slide down
	      track->command = 26;
	      track->param1 = 0;
	      break;
	    }
	  }
	}
  } else {	// version >= 5
    realloc_patterns(64, 64, 18);

    for(i=0;i<numpats;i++)
      for(j=0;j<18;j++)
	for(k=0;k<64;k++) {
	  struct Tracks	*track = &tracks[i * 18 + j][k];
	  unsigned char	*o = &org[i*64*t*4+j*64*4+k*4];

	  track->note = o[0] == 255 ? 127 : o[0];
	  track->inst = o[1];
	  track->command = newconvfx[o[2]];
	  track->param1 = o[3] >> 4;
	  track->param2 = o[3] & 0x0f;

	  // Convert '&' command
	  if(o[2] == 36)
	    switch(track->param1) {
	    case 0:	// pattern delay (frames)
	      track->command = 29;
	      track->param1 = 0;
	      // param2 already set correctly
	      break;

	    case 1:	// pattern delay (rows)
	      track->command = 14;
	      track->param1 = 8;
	      // param2 already set correctly
	      break;
	    }
	}
  }

  init_trackord();

  if(version == 1 || version == 5)
    delete [] org;
  else
    delete [] secdata;

  // Process flags
  if(version >= 5) {
    CmodPlayer::flags |= Opl3;				// All versions >= 5 are OPL3
    if(flags & 8) CmodPlayer::flags |= Tremolo;		// Tremolo depth
    if(flags & 16) CmodPlayer::flags |= Vibrato;	// Vibrato depth
  }

  fp.close(f);
  rewind(0);
  return true;
}

float Ca2mLoader::getrefresh()
{
	if(tempo != 18)
		return (float) (tempo);
	else
		return 18.2f;
}

/*** private methods *************************************/

void Ca2mLoader::inittree()
{
	unsigned short i;

	for(i=2;i<=TWICEMAX;i++) {
        m_dad[i] = i / 2;
        m_freq[i] = 1;
	}

	for(i=1;i<=MAXCHAR;i++) {
        m_leftc[i] = 2 * i;
        m_rightc[i] = 2 * i + 1;
	}
}

void Ca2mLoader::updatefreq(unsigned short a,unsigned short b)
{
	do {
        m_freq[m_dad[a]] = m_freq[a] + m_freq[b];
        a = m_dad[a];
		if(a != ROOT)
            if(m_leftc[m_dad[a]] == a)
                b = m_rightc[m_dad[a]];
			else
                b = m_leftc[m_dad[a]];
	} while(a != ROOT);

    if(m_freq[ROOT] == MAXFREQ)
		for(a=1;a<=TWICEMAX;a++)
            m_freq[a] >>= 1;
}

void Ca2mLoader::updatemodel(unsigned short code)
{
	unsigned short a=code+SUCCMAX,b,c,code1,code2;

    m_freq[a]++;
    if(m_dad[a] != ROOT) {
        code1 = m_dad[a];
        if(m_leftc[code1] == a)
            updatefreq(a,m_rightc[code1]);
		else
            updatefreq(a,m_leftc[code1]);

		do {
            code2 = m_dad[code1];
            if(m_leftc[code2] == code1)
                b = m_rightc[code2];
			else
                b = m_leftc[code2];

            if(m_freq[a] > m_freq[b]) {
                if(m_leftc[code2] == code1)
                    m_rightc[code2] = a;
				else
                    m_leftc[code2] = a;

                if(m_leftc[code1] == a) {
                    m_leftc[code1] = b;
                    c = m_rightc[code1];
				} else {
                    m_rightc[code1] = b;
                    c = m_leftc[code1];
				}

                m_dad[b] = code1;
                m_dad[a] = code2;
				updatefreq(b,c);
				a = b;
			}

            a = m_dad[a];
            code1 = m_dad[a];
		} while(code1 != ROOT);
	}
}

unsigned short Ca2mLoader::inputcode(unsigned short bits)
{
	unsigned short i,code=0;

	for(i=1;i<=bits;i++) {
        if(!m_bitcount) {
            if(m_bitcount == MAXBUF)
                m_bufcount = 0;
            m_bitbuffer = m_wdbuf[m_bufcount];
            m_bufcount++;
            m_bitcount = 15;
		} else
            m_bitcount--;

        if(m_bitbuffer > 0x7fff)
            code |= m_bitvalue[i-1];
        m_bitbuffer <<= 1;
	}

	return code;
}

unsigned short Ca2mLoader::uncompress()
{
	unsigned short a=1;

	do {
        if(!m_bitcount) {
            if(m_bufcount == MAXBUF)
                m_bufcount = 0;
            m_bitbuffer = m_wdbuf[m_bufcount];
            m_bufcount++;
            m_bitcount = 15;
		} else
            m_bitcount--;

        if(m_bitbuffer > 0x7fff)
            a = m_rightc[a];
		else
            a = m_leftc[a];
        m_bitbuffer <<= 1;
	} while(a <= MAXCHAR);

	a -= SUCCMAX;
	updatemodel(a);
	return a;
}

void Ca2mLoader::decode()
{
	unsigned short i,j,k,t,c,count=0,dist,len,index;

	inittree();
	c = uncompress();

	while(c != TERMINATE) {
		if(c < 256) {
            m_obuf[m_obufcount] = (unsigned char)c;
            m_obufcount++;
            if(m_obufcount == MAXBUF) {
                m_outputSize = MAXBUF;
                m_obufcount = 0;
			}

            m_buf[count] = (unsigned char)c;
			count++;
			if(count == MAXSIZE)
				count = 0;
		} else {
			t = c - FIRSTCODE;
			index = t / CODESPERRANGE;
			len = t + MINCOPY - index * CODESPERRANGE;
            dist = inputcode(m_copybits[index]) + len + m_copymin[index];

			j = count;
			k = count - dist;
			if(count < dist)
				k += MAXSIZE;

			for(i=0;i<=len-1;i++) {
                m_obuf[m_obufcount] = m_buf[k];
                m_obufcount++;
                if(m_obufcount == MAXBUF) {
                    m_outputSize = MAXBUF;
                    m_obufcount = 0;
				}

                m_buf[j] = m_buf[k];
				j++; k++;
				if(j == MAXSIZE) j = 0;
				if(k == MAXSIZE) k = 0;
			}

			count += len;
			if(count >= MAXSIZE)
				count -= MAXSIZE;
		}
		c = uncompress();
	}
    m_outputSize = m_obufcount;
}

unsigned short Ca2mLoader::sixdepak(unsigned short *source, unsigned char *dest,
				    unsigned short size)
{
	if((unsigned int)size + 4096 > MAXBUF)
		return 0;

    m_buf = new unsigned char [MAXSIZE];
    m_inputSize = size;
    m_bitcount = 0; m_bitbuffer = 0;
    m_obufcount = 0; m_bufcount = 0;
    m_wdbuf = source; m_obuf = dest;

	decode();
    delete [] m_buf;
    return m_outputSize;
}
