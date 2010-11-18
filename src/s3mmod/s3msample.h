/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef s3msampleH
#define s3msampleH

#include "gensample.h"

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
				typedef PVector<S3mSample> List;
			public:
				S3mSample() throw();
				virtual ~S3mSample() throw();
				virtual bool load(BinStream& str, std::size_t pos) throw(PppException);
		};
	} // namespace ppp
} // namespace s3m

#endif
