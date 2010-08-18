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

#ifndef stmsampleH
#define stmsampleH

#include "gensample.h"

/**
* @file
* @brief STM Sample class
* @ingroup StmMod
*/

namespace ppp {
	namespace stm {

		/**
		* @class StmSample
		* @ingroup StmMod
		* @brief Sample class for STM Samples
		*/
		class StmSample : public GenSample {
			public:
				typedef std::shared_ptr<StmSample> CP;
			public:
				StmSample() throw();
				virtual ~StmSample() throw();
				virtual bool load(BinStream& str, std::size_t pos) throw(PppException);
		};
	} // namespace ppp
} // namespace stm

#endif
