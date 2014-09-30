/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2003 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef H_OUTPUT
#define H_OUTPUT

#include "adplug/player.h"
#include "genmod/breseninter.h"

class Player
{
    DISABLE_COPY(Player)
    public:

    Player() = default;
    virtual ~Player() = default;

    virtual void frame() = 0;

    void setPlayer(std::shared_ptr<CPlayer>& player) {
        m_player = player;
    }

    bool isPlaying() const noexcept {
        return m_playing;
    }

protected:
    void setIsPlaying(bool value) noexcept {
        m_playing = value;
    }
    CPlayer* getPlayer() {
        return m_player.get();
    }

private:
    bool m_playing = false;
    std::shared_ptr<CPlayer> m_player{};
};

class EmuPlayer: public Player
{
    DISABLE_COPY(EmuPlayer)
    private:
        std::vector<int16_t> m_audioBuf;
    unsigned long	m_freq;
    ppp::BresenInterpolation m_oplInterp;

public:
    EmuPlayer(unsigned long nfreq, size_t nbufsize);

    virtual void setBufferSize(size_t nbufsize);
    virtual void frame();

protected:
    virtual void output(const std::vector<int16_t>& buf) = 0;
    // The output buffer is always of the size requested through the constructor.
    // This time, size is measured in bytes, not samples!
};

#endif
