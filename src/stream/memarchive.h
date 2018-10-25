/*
    PPPlay - an old-fashioned module player
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

#ifndef PPPLAY_MEMARCHIVE_H
#define PPPLAY_MEMARCHIVE_H

#include "abstractarchive.h"

/**
 * @class MemArchive
 * @ingroup Common
 * @brief Specialization of AbstractArchive for memory storage
 */
class MemArchive
    : public AbstractArchive
{
public:
    DISABLE_COPY(MemArchive)

    /**
     * @brief Constructs this archive using a SBinStream
     */
    MemArchive();

    ~MemArchive() override;
};

#endif
