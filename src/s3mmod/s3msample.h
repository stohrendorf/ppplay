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

#ifndef S3MSAMPLE_H
#define S3MSAMPLE_H

#include "genmod/gensample.h"
#include "stream/binstream.h"

/**
* @file
* @brief S3M Sample class
* @ingroup S3mMod
*/

namespace ppp {
namespace s3m {

/**
* @class S3mSample
* @ingroup S3mMod
* @brief Sample class for S3M Samples
*/
class S3mSample : public GenSample {
	public:
		typedef std::shared_ptr<S3mSample> Ptr;
		typedef std::vector<Ptr> Vector;
	private:
		bool m_highQuality;
	public:
		S3mSample();
		virtual ~S3mSample();
		bool load(BinStream& str, std::size_t pos, bool imagoLoopEnd);
		bool isHighQuality() const;
};
} // namespace ppp
} // namespace s3m

#endif
