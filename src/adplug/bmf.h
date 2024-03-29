#pragma once

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
 * [xad] BMF player, by Riven the Mage <riven@ok.ru>
 */

#include "xad.h"

class BmfPlayer
  : public XadPlayer
{
public:
  DISABLE_COPY( BmfPlayer )

  static Player* factory();

  BmfPlayer() = default;

  ~BmfPlayer() override = default;

protected:
  enum BmfVersion
  {
    BMF0_9B,
    BMF1_1,
    BMF1_2
  };
  //
  struct bmf_event
  {
    uint8_t note;
    uint8_t delay;
    uint8_t volume;
    uint8_t instrument;
    uint8_t cmd;
    uint8_t cmd_data;
  };

  BmfVersion m_bmfVersion = BMF0_9B;
  std::string m_bmfTitle{};
  std::string m_bmfAuthor{};
  float m_bmfTimer = 0;

  struct
  {
    char name[11] = "";
    std::array<uint8_t, 13> data;
  } m_bmfInstruments[32];

  bmf_event m_bmfStreams[9][1024]{};

  int m_bmfActiveStreams = 0;

  struct Channel
  {
    uint16_t stream_position;
    uint8_t delay;
    uint16_t loop_position;
    uint8_t loop_counter;
  };

  Channel m_bmfChannels[9]{};

  bool xadplayer_load() override;

  void xadplayer_rewind(const boost::optional<size_t>& subsong) override;

  void xadplayer_update() override;

  size_t framesUntilUpdate() const override;

  std::string type() const override;

  std::string title() const override;

  std::string author() const override;

  std::string instrumentTitle(size_t i) const override;

  size_t instrumentCount() const override;

private:
  static const uint8_t bmf_adlib_registers[117];
  static const uint16_t bmf_notes[12];
  static const uint16_t bmf_notes_2[12];
  static const std::array<uint8_t, 13> bmf_default_instrument;

  int __bmf_convert_stream(const uint8_t* stream, int channel);
};
