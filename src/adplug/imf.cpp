/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * imf.cpp - IMF Player by Simon Peter <dn.tlp@gmx.net>
 *
 * FILE FORMAT:
 * There seem to be 2 different flavors of IMF formats out there. One version
 * contains just the raw IMF music data. In this case, the first word of the
 * file is always 0 (because the music data starts this way). This is already
 * the music data! So read in the entire file and play it.
 *
 * If this word is greater than 0, it specifies the size of the following
 * song data in bytes. In this case, the file has a footer that contains
 * arbitrary infos about it. Mostly, this is plain ASCII text with some words
 * of the author. Read and play the specified amount of song data and display
 * the remaining data as ASCII text.
 *
 * NOTES:
 * This player handles the two above mentioned formats, as well as a third
 * type, invented by Martin Fernandez <mfernan@cnba.uba.ar>, that's got a
 * proper header to add title/game name information. After the header starts
 * the normal IMF file in one of the two above mentioned formats.
 *
 * This player also handles a special footer format by Adam Nielsen,
 * which has defined fields of information about the song, the author
 * and more.
 */

#include <string.h>

#include "imf.h"
#include "database.h"

/*** public methods *************************************/

CPlayer *CimfPlayer::factory()
{
  return new CimfPlayer();
}

bool CimfPlayer::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;
  unsigned long fsize, flsize, mfsize = 0;
  unsigned int i;

  // file validation section
  {
    char	header[5];
    int		version;

    f->readString(header, 5);
    version = f->readInt(1);

    if(strncmp(header, "ADLIB", 5) || version != 1) {
      if(!fp.extension(filename, ".imf") && !fp.extension(filename, ".wlf")) {
	// It's no IMF file at all
	fp.close(f);
	return false;
      } else
	f->seek(0);	// It's a normal IMF file
    } else {
      // It's a IMF file with header
      m_trackName = f->readString('\0');
      m_gameName = f->readString('\0');
      f->ignore(1);
      mfsize = f->pos() + 2;
    }
  }

  // load section
  if(mfsize)
    fsize = f->readInt(4);
  else
    fsize = f->readInt(2);
  flsize = fp.filesize(f);
  size_t size;
  if(!fsize) {		// footerless file (raw music data)
    if(mfsize)
      f->seek(-4, binio::Add);
    else
      f->seek(-2, binio::Add);
    size = (flsize - mfsize) / 4;
  } else		// file has got a footer
    size = fsize / 4;

  m_data.resize(size);
  for(i = 0; i < size; i++) {
    m_data[i].reg = f->readInt(1); m_data[i].val = f->readInt(1);
    m_data[i].time = f->readInt(2);
  }

  // read footer, if any
  if(fsize && (fsize < flsize - 2 - mfsize)) {
    if(f->readInt(1) == 0x1a) {
      // Adam Nielsen's footer format
      m_trackName = f->readString();
      m_authorName = f->readString();
      m_remarks = f->readString();
    } else {
      // Generic footer
      unsigned long footerlen = flsize - fsize - 2 - mfsize;

      m_footer.resize(footerlen);

      for(size_t i=0; i<footerlen; ++i)
          m_footer += char(f->readInt(1));
    }
  }

  m_rate = getrate(filename, fp, f);
  fp.close(f);
  rewind(0);
  return true;
}

bool CimfPlayer::update()
{
	do {
		getOpl()->writeReg(m_data[m_pos].reg,m_data[m_pos].val);
		m_del = m_data[m_pos].time;
		m_pos++;
    } while(!m_del && m_pos < m_data.size());

    if(m_pos >= m_data.size()) {
		m_pos = 0;
		m_songend = true;
	}
    else m_timer = m_rate / (float)m_del;

	return !m_songend;
}

void CimfPlayer::rewind(int subsong)
{
	m_pos = 0; m_del = 0; m_timer = m_rate; m_songend = false;
	getOpl()->writeReg(1,32);	// go to OPL2 mode
}

std::string CimfPlayer::gettitle()
{
  std::string	title;

  title = m_trackName;

  if(!m_trackName.empty() && !m_gameName.empty())
    title += " - ";

  title += m_gameName;

  return title;
}

std::string CimfPlayer::getdesc()
{
  std::string desc = m_footer;

  if(!m_remarks.empty() && !m_footer.empty())
    desc += "\n\n";

  desc += m_remarks;

  return desc;
}

/*** private methods *************************************/

float CimfPlayer::getrate(const std::string &filename, const CFileProvider &fp, binistream *f)
{
  if(m_db) {	// Database available
    f->seek(0, binio::Set);
    CClockRecord *record = (CClockRecord *)m_db->search(CAdPlugDatabase::CKey(*f));
    if (record && record->m_type == CAdPlugDatabase::CRecord::ClockSpeed)
      return record->m_clock;
  }

  // Otherwise the database is either unavailable, or there's no entry for this file
  if (fp.extension(filename, ".imf")) return 560.0f;
  if (fp.extension(filename, ".wlf")) return 700.0f;
  return 700.0f; // default speed for unknown files that aren't .IMF or .WLF
}
