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

#ifndef PPPLAY_ISERIALIZABLE_H
#define PPPLAY_ISERIALIZABLE_H

#include <stream/ppplay_stream_export.h>

class AbstractArchive;

/**
 * @interface ISerializable
 * @ingroup Common
 * @brief Interface for serialisable classes
 */
class PPPLAY_STREAM_EXPORT ISerializable
{
public:
	/**
	 * @brief Serialise this object
	 * @param[in,out] archive AbstractArchive to serialize this object to
	 * @return Reference to @a archive for pipelining
	 */
	virtual AbstractArchive& serialize( AbstractArchive* archive ) = 0;
	/**
	 * @brief Destructor
	 */
	virtual ~ISerializable() { }
};

#endif
