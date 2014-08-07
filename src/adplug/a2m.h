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
 * a2m.h - A2M Loader by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_A2MLOADER
#define H_ADPLUG_A2MLOADER

#include "protrack.h"

class Ca2mLoader: public CmodPlayer
{
public:
    static CPlayer *factory(opl::Opl3 *newopl);

    Ca2mLoader(opl::Opl3 *newopl): CmodPlayer(newopl)
    { }

    bool load(const std::string &filename, const CFileProvider &fp);
    float getrefresh();

    std::string gettype()
    { return std::string("AdLib Tracker 2"); }
    std::string gettitle()
    { if(*m_songname) return std::string(m_songname,1,*m_songname); else return std::string(); }
    std::string getauthor()
    { if(*m_author) return std::string(m_author,1,*m_author); else return std::string(); }
    unsigned int getinstruments()
    { return 250; }
    std::string getinstrument(unsigned int n)
    { return std::string(m_instname[n],1,*m_instname[n]); }

private:

#define ADPLUG_A2M_COPYRANGES		6
#define ADPLUG_A2M_FIRSTCODE		257
#define ADPLUG_A2M_MINCOPY		3
#define ADPLUG_A2M_MAXCOPY		255
#define ADPLUG_A2M_CODESPERRANGE	(ADPLUG_A2M_MAXCOPY - ADPLUG_A2M_MINCOPY + 1)
#define ADPLUG_A2M_MAXCHAR		(ADPLUG_A2M_FIRSTCODE + ADPLUG_A2M_COPYRANGES * ADPLUG_A2M_CODESPERRANGE - 1)
#define ADPLUG_A2M_TWICEMAX		(2 * ADPLUG_A2M_MAXCHAR + 1)

    static const unsigned int MAXFREQ=2000,
    MINCOPY=ADPLUG_A2M_MINCOPY,
    MAXCOPY=ADPLUG_A2M_MAXCOPY,
    COPYRANGES=ADPLUG_A2M_COPYRANGES,
    CODESPERRANGE=ADPLUG_A2M_CODESPERRANGE,
    TERMINATE=256,
    FIRSTCODE=ADPLUG_A2M_FIRSTCODE,
    MAXCHAR=FIRSTCODE + COPYRANGES * CODESPERRANGE - 1,
    SUCCMAX=MAXCHAR + 1,
    TWICEMAX=ADPLUG_A2M_TWICEMAX,
    ROOT=1,
    MAXBUF=42 * 1024,
    MAXDISTANCE=21389,
    MAXSIZE=21389 + MAXCOPY;

    static const unsigned short m_bitvalue[14];
    static const signed short m_copybits[ADPLUG_A2M_COPYRANGES],
    m_copymin[ADPLUG_A2M_COPYRANGES];

    void inittree();
    void updatefreq(unsigned short a,unsigned short b);
    void updatemodel(unsigned short code);
    unsigned short inputcode(unsigned short bits);
    unsigned short uncompress();
    void decode();
    unsigned short sixdepak(unsigned short *source,unsigned char *dest,unsigned short size);

    char m_songname[43], m_author[43], m_instname[250][33];

    unsigned short m_bitcount=0, m_bitbuffer=0, m_bufcount=0, m_obufcount=0, m_inputSize=0,
    m_outputSize=0, m_leftc[ADPLUG_A2M_MAXCHAR+1], m_rightc[ADPLUG_A2M_MAXCHAR+1],
    m_dad[ADPLUG_A2M_TWICEMAX+1], m_freq[ADPLUG_A2M_TWICEMAX+1], *m_wdbuf=nullptr;
    unsigned char *m_obuf=nullptr, *m_buf=nullptr;
};

#endif
