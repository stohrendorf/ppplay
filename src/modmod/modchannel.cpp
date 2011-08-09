/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "modchannel.h"

namespace ppp {
namespace mod {

ModChannel::ModChannel() : m_currentCell()
{

}

ModChannel::~ModChannel() = default;

std::string ModChannel::cellString()
{
	return m_currentCell.trackerString();
}

std::string ModChannel::effectDescription() const
{
	// TODO
}

std::string ModChannel::effectName() const
{
	// TODO
}

void ModChannel::mixTick(MixerFrameBuffer& mixBuffer)
{
	// TODO
}

std::string ModChannel::noteName()
{
	return "???"; // TODO
}

IArchive& ModChannel::serialize(IArchive* data)
{
    // TODO return ppp::GenChannel::serialize(data);
}

void ModChannel::simTick(size_t bufsize)
{
	// TODO
}

}
}
