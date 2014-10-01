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
 * [xad] RAT player, by Riven the Mage <riven@ok.ru>
 */

#include "xad.h"

class CxadratPlayer : public CxadPlayer {
  DISABLE_COPY(CxadratPlayer)
public:
  static CPlayer *factory();

  CxadratPlayer() = default;

protected:
  struct rat_header {
    char id[3] = "";
    unsigned char version = 0;
    char title[32] = "";
    unsigned char numchan = 0;
    unsigned char reserved_25 = 0;
    unsigned char order_end = 0;
    unsigned char reserved_27 = 0;
    unsigned char numinst = 0; // ?: Number of Instruments
    unsigned char reserved_29 = 0;
    unsigned char numpat = 0; // ?: Number of Patterns
    unsigned char reserved_2B = 0;
    unsigned char order_start = 0;
    unsigned char reserved_2D = 0;
    unsigned char order_loop = 0;
    unsigned char reserved_2F = 0;
    unsigned char volume = 0;
    unsigned char speed = 0;
    unsigned char reserved_32[12] = "";
    unsigned char patseg[2] = {0,0};
  };

  struct rat_event {
    unsigned char note;
    unsigned char instrument;
    unsigned char volume;
    unsigned char fx;
    unsigned char fxp;
  };

  struct rat_instrument {
    unsigned char freq[2];
    unsigned char reserved_2[2];
    unsigned char mod_ctrl;
    unsigned char car_ctrl;
    unsigned char mod_volume;
    unsigned char car_volume;
    unsigned char mod_AD;
    unsigned char car_AD;
    unsigned char mod_SR;
    unsigned char car_SR;
    unsigned char mod_wave;
    unsigned char car_wave;
    unsigned char connect;
    unsigned char reserved_F;
    unsigned char volume;
    unsigned char reserved_11[3];
  };

  struct {
    rat_header hdr{};

    unsigned char volume = 0;
    unsigned char order_pos = 0;
    unsigned char pattern_pos = 0;

    unsigned char *order = nullptr;

    rat_instrument *inst = nullptr;

    rat_event tracks[256][64][9]{};

    struct {
      unsigned char instrument = 0;
      unsigned char volume = 0;
      unsigned char fx = 0;
      unsigned char fxp = 0;
    } channel[9]{};
  } rat{};
  //
  bool xadplayer_load();
  void xadplayer_rewind(int);
  void xadplayer_update();
  float xadplayer_getrefresh();
  std::string xadplayer_gettype();
  std::string xadplayer_gettitle();
  unsigned int xadplayer_getinstruments();
  //
private:
  static const unsigned char rat_adlib_bases[18];
  static const unsigned short rat_notes[16];

  unsigned char __rat_calc_volume(unsigned char ivol, unsigned char cvol,
                                  unsigned char gvol);
};
