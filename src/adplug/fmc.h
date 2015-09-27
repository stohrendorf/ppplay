/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.

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

  fmc.h - FMC loader by Riven the Mage <riven@ok.ru>
*/

#include "protrack.h"

class CfmcLoader : public CmodPlayer {
    DISABLE_COPY(CfmcLoader)
    public:
        static CPlayer *factory();

    CfmcLoader() = default;

    bool load(const std::string &filename);
    size_t framesUntilUpdate() const;

    std::string type() const;
    std::string title() const;
    std::string instrumentTitle(size_t n) const;
    uint32_t instrumentCount() const;

private:

#pragma pack(push,1)
    struct fmc_event {
        uint8_t byte0;
        uint8_t byte1;
        uint8_t byte2;
    };
#pragma pack(pop)

    struct fmc_header {
        char id[4] = "";
        char title[21] = "";
        uint8_t numchan = 0;
    };
    fmc_header header{};

#pragma pack(push,1)
    struct fmc_instrument {
        uint8_t synthesis;
        uint8_t feedback;

        uint8_t mod_attack;
        uint8_t mod_decay;
        uint8_t mod_sustain;
        uint8_t mod_release;
        uint8_t mod_volume;
        uint8_t mod_ksl;
        uint8_t mod_freq_multi;
        uint8_t mod_waveform;
        uint8_t mod_sustain_sound;
        uint8_t mod_ksr;
        uint8_t mod_vibrato;
        uint8_t mod_tremolo;
        uint8_t car_attack;
        uint8_t car_decay;
        uint8_t car_sustain;
        uint8_t car_release;
        uint8_t car_volume;
        uint8_t car_ksl;
        uint8_t car_freq_multi;
        uint8_t car_waveform;
        uint8_t car_sustain_sound;
        uint8_t car_ksr;
        uint8_t car_vibrato;
        uint8_t car_tremolo;

        int8_t pitch_shift;

        char name[21];
    };
#pragma pack(pop)

    fmc_instrument instruments[32];

    using CmodPlayer::addInstrument;
    void addInstrument(const fmc_instrument &instrument);
};
