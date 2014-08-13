/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * mid.h - LAA, SCI, MID & CMF Player by Philip Hassey <philhassey@hotmail.com>
 */

#include "player.h"
#include "mid/almidi.h"
#include "stuff/numberutils.h"

class CmidPlayer: public CPlayer
{
public:
  static CPlayer *factory();

  CmidPlayer();

  bool load(const std::string &filename, const CFileProvider &fp);
  bool update();
  void rewind(int subsong);
  size_t framesUntilUpdate();

  std::string gettype();
  std::string gettitle()
    { return std::string(m_title); }
  std::string getauthor()
    { return std::string(m_author); }
  std::string getdesc()
    { return std::string(m_remarks); }
  unsigned int getinstruments()
    { return m_tins; }
  unsigned int getsubsongs()
    { return m_subsongs; }

 protected:
  static const unsigned char adlib_opadd[];
  static const int percussion_map[];

  struct midi_channel {
    int inum;
    unsigned char ins[11];
    int vol;
    int nshift;
    int on;
  };

  struct midi_track {
    size_t tend;
    size_t spos;
    size_t pos;
    unsigned long iwait;
    int on;
    unsigned char pv;
  };

  char *m_author,*m_title,*m_remarks,m_emptystr;
  size_t m_dataPos;
  unsigned long m_sierraPos; //sierras gotta be special.. :>
  int m_subsongs;
  std::vector<uint8_t> m_data;

  int m_adlibStyle;
  bool m_melodicMode;
  unsigned char m_myInsBank[128][14], m_sMyInsBank[128][14];
  midi_channel m_ch[16];
  int m_chp[18][3];

  long m_deltas;
  long m_msqtr;

  midi_track m_tracks[16];
  unsigned int m_currentTrack;

  float m_fwait;
  unsigned long m_iwait;
  bool m_doing;

  enum class FileType {
      Unknown,
      Lucas,
      Midi,
      Cmf,
      Sierra,
      AdvSierra,
      OldLucas
  };

  FileType m_type = FileType::Unknown;
  int m_tins,m_stins;

 private:
  bool load_sierra_ins(const std::string &fname, const CFileProvider &fp);
  void midiprintf(const char *format, ...);
  unsigned char datalook(long m_dataPos);
  unsigned long getnexti(size_t num);
  unsigned long getnext(size_t num);
  unsigned long getval();
  void sierra_next_section();
  void midi_fm_instrument(int voice, unsigned char *inst);
  void midi_fm_percussion(int m_ch, unsigned char *inst);
  void midi_fm_volume(int voice, int volume);
  void midi_fm_playnote(int voice, int note, int volume);
  void midi_fm_endnote(int voice);
  void midi_fm_reset();
};

class CDukePlayer : CPlayer {
private:
    std::unique_ptr<ppp::EMidi> m_emidi;
public:
    static CPlayer* factory() {
        return new CDukePlayer();
    }

    bool load(const std::string &filename, const CFileProvider &fp);
    bool update() {
        return m_emidi->serviceRoutine();
    }

    void rewind(int){}
    size_t framesUntilUpdate() {
        return SampleRate / m_emidi->ticksPerSecond();
    }

    std::string gettype()
      { return "Duke MIDI"; }
    std::string gettitle()
      { return std::string(); }
    std::string getauthor()
      { return std::string(); }
    std::string getdesc()
      { return std::string(); }
    unsigned int getinstruments()
      { return 0; }
    unsigned int getsubsongs()
      { return 1; }

    virtual void read(std::array<int16_t, 4> *data) override {
        m_emidi->read(data);
    }
};
