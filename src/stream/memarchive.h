/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Syron <mr.syron@googlemail.com>

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

#ifndef MEMARCHIVE_H
#define MEMARCHIVE_H

#include "iarchive.h"

/**
 * @class MemArchive
 * @ingroup Common
 * @brief Specialization of IArchive for memory storage
 */
class MemArchive : public IArchive {
		DISABLE_COPY(MemArchive)
	public:
		/**
		 * @brief Constructs this archive using a SBinStream
		 */
		MemArchive();
		virtual ~MemArchive();
};

#endif