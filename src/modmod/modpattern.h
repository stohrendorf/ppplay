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

#ifndef modpatternH
#define modpatternH

#include "genpattern.h"
#include "modbase.h"

using namespace std;

/**
 * @file
 * @brief MOD Pattern class
 * @ingroup ModMod
 */

namespace ppp {
	namespace mod {

		/**
		 * @class ModCell
		 * @ingroup ModMod
		 * @brief A ScreamTracker cell
		 */
		class ModCell : public GenCell {
			public:
				typedef std::tr1::shared_ptr<ModCell> CP;
			private:
				unsigned char aNote; //!< @brief Note value
				unsigned char aInstr; //!< @brief Instrument value
				unsigned char aVolume; //!< @brief Volume value
				unsigned char aEffect; //!< @brief Effect
				unsigned char aEffectValue; //!< @brief Effect value
			public:
				ModCell() throw();
				virtual ~ModCell() throw();
				virtual bool load(BinStream &str) throw(PppException);
				virtual void reset() throw();
				virtual string trackerString() const throw();
				/**
				 * @brief Assignment operator
				 * @param[in] src Source cell
				 * @return Reference to *this
				 */
				ModCell &operator=(const ModCell &src) throw();
				/**
				 * @brief Get the cell's note
				 * @return #aNote
				 */
				unsigned char getNote() const throw();
				/**
				 * @brief Get the cell's instrument
				 * @return #aInstr
				 */
				unsigned char getInstr() const throw();
				/**
				 * @brief Get the cell's volume
				 * @return #aVolume
				 */
				unsigned char getVolume() const throw();
				/**
				 * @brief Get the cell's effect
				 * @return #aEffect
				 */
				unsigned char getEffect() const throw();
				/**
				 * @brief Get the cell's effect value
				 * @return #aEffectValue
				 */
				unsigned char getEffectValue() const throw();
		};

		/**
		 * @class ModPattern
		 * @ingroup ModMod
		 * @brief Pattern class for MOD Patterns
		 */
		class ModPattern : public GenPattern {
			public:
				typedef std::tr1::shared_ptr<ModPattern> CP;
			protected:
				virtual GenCell::CP createCell(int trackIndex, int row) throw(PppException);
			public:
				ModPattern() throw(PppException);
				virtual ~ModPattern() throw();
				virtual bool load(BinStream& str, unsigned int pos) throw(PppException);
		};
	} // namespace ppp
} // namespace mod

#endif
