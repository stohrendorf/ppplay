/*
  AdPlug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  xad.h - XAD shell player by Riven the Mage <riven@ok.ru>
*/

#ifndef H_ADPLUG_XAD
#define H_ADPLUG_XAD

#include "player.h"

class CxadPlayer : public CPlayer {
    DISABLE_COPY(CxadPlayer)
    public:
        static CPlayer *factory();

    CxadPlayer() = default;
    ~CxadPlayer() = default;

    bool load(const std::string &filename);
    bool update();
    void rewind(int subsong);
    size_t framesUntilUpdate();

    std::string gettype();
    std::string gettitle();
    std::string getauthor();
    std::string getinstrument(unsigned int i);
    unsigned int getinstruments();

protected:
    virtual void xadplayer_rewind(int subsong) = 0;
    virtual bool xadplayer_load() = 0;
    virtual void xadplayer_update() = 0;
    virtual float xadplayer_getrefresh() = 0;
    virtual std::string xadplayer_gettype() = 0;
    virtual std::string xadplayer_gettitle() { return m_xadHeader.title; }
    virtual std::string xadplayer_getauthor() { return m_xadHeader.author; }
    virtual std::string xadplayer_getinstrument(unsigned int) {
        return std::string();
    }
    virtual unsigned int xadplayer_getinstruments() { return 0; }

    enum Format : uint16_t {
        None,
        HYP = 1,
        PSI,
        FLASH,
        BMF,
        RAT,
        HYBRID
    };

#pragma pack(push,1)
    struct xad_header {
        uint32_t id = 0;
        char title[36] = "";
        char author[36] = "";
        Format fmt = None;
        uint8_t speed = 0;
        uint8_t reserved_a = 0;
    };
#pragma pack(pop)

    xad_header m_xadHeader;

    std::vector<uint8_t> tune{};

    struct {
        int playing = 0;
        int looping = 0;
        unsigned char speed = 0;
        unsigned char speed_counter = 0;
    } plr{};
};

#endif
