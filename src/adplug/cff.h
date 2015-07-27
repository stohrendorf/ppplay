/*
  AdPlug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2006 Simon Peter <dn.tlp@gmx.net>, et al.

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

  cff.h - BoomTracker loader by Riven the Mage <riven@ok.ru>
*/

#include "protrack.h"

class CcffLoader : public CmodPlayer {
    DISABLE_COPY(CcffLoader)
    public:
        static CPlayer *factory();

    CcffLoader() = default;

    bool load(const std::string &filename);
    void rewind(int subsong);

    std::string type() const;
    std::string title() const;
    std::string author() const;
    std::string instrumentTitle(size_t n) const;
    uint32_t instrumentCount() const;

private:

    class cff_unpacker {
    public:

        long unpack(unsigned char *ibuf, unsigned char *obuf);

    private:

        uint32_t get_code();
        void translate_code(unsigned long code, unsigned char *string, const std::vector<std::vector<uint8_t> > &dictionary);

        void cleanup();
        bool startup(const std::vector<std::vector<uint8_t> > &dictionary);

        void expand_dictionary(unsigned char *string, std::vector<std::vector<uint8_t> > &dictionary);

        unsigned char *m_input;
        unsigned char *m_output;

        size_t m_outputLength;

        unsigned char m_codeLength;

        unsigned long m_bitsBuffer;
        unsigned int m_bitsLeft;

        unsigned char m_theString[256];
    };

#pragma pack(push,1)
    struct cff_header {
        char id[16];
        unsigned char version;
        unsigned short size;
        unsigned char packed;
        unsigned char reserved[12];
    };
    cff_header header;
#pragma pack(pop)

    struct cff_instrument {
        unsigned char data[12];
        char name[21];
    } instruments[47];

    char song_title[20];
    char song_author[20];

    struct cff_event {
        unsigned char byte0;
        unsigned char byte1;
        unsigned char byte2;
    };
};
