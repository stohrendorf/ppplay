/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2005 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * imf.h - IMF Player by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_IMFPLAYER
#define H_ADPLUG_IMFPLAYER

#include "player.h"

class CimfPlayer: public CPlayer
{
public:
  static CPlayer *factory();

	bool load(const std::string &filename, const CFileProvider &fp);
	bool update();
	void rewind(int subsong);
    size_t framesUntilUpdate() override {
        return SampleRate/m_timer;
    }

    std::string gettype() {
        return "IMF File Format";
    }
	std::string gettitle();
    std::string getauthor() {
        return m_authorName;
    }
	std::string getdesc();

protected:
    unsigned long m_pos=0;
    unsigned short m_del=0;
    bool m_songend=false;
    float m_rate=0;
    float m_timer=0;
    std::string m_footer{};
    std::string	m_trackName{}, m_gameName{}, m_authorName{}, m_remarks{};

	struct Sdata {
        unsigned char reg=0, val=0;
        unsigned short time=0;
    };
    std::vector<Sdata> m_data{};

private:
	float getrate(const std::string &filename, const CFileProvider &fp, binistream *f);
};

#endif
