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

/**
 * @ingroup S3mMod
 * @{
 */

#include "genmod/gensample.h"
#include "stream/binstream.h"

namespace ppp {
namespace s3m {

/**
 * @class S3mSample
 * @brief Sample class for S3M Samples
 */
class S3mSample : public GenSample {
		DISABLE_COPY(S3mSample)
	public:
		typedef std::shared_ptr<S3mSample> Ptr; //!< @brief Class pointer
		typedef std::vector<Ptr> Vector; //!< @brief Vector of class pointers
	private:
		//! @brief Whether this is a 16-bit sample
		bool m_highQuality;
	public:
		//! @brief Constructor
		S3mSample();
		/**
		 * @brief Load from a stream
		 * @param[in] str The stream to load this sample from
		 * @param[in] pos Position of the sample within @a str
		 * @param[in] imagoLoopEnd If @c true, the loop end is decreased by 1
		 * @return @c true on success
		 */
		bool load(BinStream& str, std::size_t pos, bool imagoLoopEnd);
		/**
		 * @brief Whether this is a 16-bit sample
		 * @return m_highQuality
		 */
		bool isHighQuality() const;
};

} // namespace ppp
} // namespace s3m

/**
 * @}
 */

#endif
