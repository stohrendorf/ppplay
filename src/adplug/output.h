#pragma once

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

#include "adplug/player.h"
#include "genmod/stepper.h"

class PlayerHandler
{
public:
    DISABLE_COPY(PlayerHandler)

    PlayerHandler() = default;

    virtual ~PlayerHandler() = default;

    virtual void frame() = 0;

    void setPlayer(std::shared_ptr<Player>& player)
    {
        m_player = player;
    }

    bool isPlaying() const noexcept
    {
        return m_playing;
    }

protected:
    void setIsPlaying(bool value) noexcept
    {
        m_playing = value;
    }

    Player* getPlayer()
    {
        return m_player.get();
    }

private:
    bool m_playing = false;
    std::shared_ptr<Player> m_player{};
};

class EmuPlayer
    : public PlayerHandler
{
private:
    std::vector<int16_t> m_audioBuf;
    unsigned long m_freq;
    ppp::Stepper m_oplInterp;

public:
    DISABLE_COPY(EmuPlayer)

    EmuPlayer(unsigned long nfreq, size_t nbufsize);

    void frame() override;

protected:
    virtual void output(const std::vector<int16_t>& buf) = 0;
    // The output buffer is always of the size requested through the constructor.
    // This time, size is measured in bytes, not samples!
};
