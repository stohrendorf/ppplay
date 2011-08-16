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

#include "sbinstream.h"

#include <sstream>

SBinStream::SBinStream() : BinStream(BinStream::SpIoStream(new std::stringstream(std::ios::in | std::ios::out | std::ios::binary))) {
}

SBinStream::~SBinStream() = default;

size_t SBinStream::size() const
{
	std::shared_ptr<std::stringstream> p = std::static_pointer_cast<std::stringstream>(stream());
	return p->str().size();
}
