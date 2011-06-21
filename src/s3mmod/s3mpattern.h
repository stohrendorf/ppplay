/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef S3MPATTERN_H
#define S3MPATTERN_H

#include "s3mcell.h"

/**
 * @file
 * @brief S3M Pattern class
 * @ingroup S3mMod
 */

namespace ppp {
namespace s3m {

/**
 * @class S3mPattern
 * @ingroup S3mMod
 * @brief Pattern class for S3M Patterns
 */
class S3mPattern {
	public:
		typedef std::shared_ptr<S3mPattern> Ptr;
		typedef std::vector<Ptr> Vector;
	private:
		std::vector<S3mCell::Vector> m_tracks;
		S3mCell::Ptr createCell(uint16_t trackIndex, int16_t row);
	public:
		S3mPattern();
		~S3mPattern();
		bool load(BinStream& str, std::size_t pos);
		S3mCell::Ptr cellAt(uint16_t trackIndex, int16_t row);
};

} // namespace s3m
} // namespace ppp

#endif
