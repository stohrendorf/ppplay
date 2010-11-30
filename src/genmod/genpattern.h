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

#ifndef genpatternH
#define genpatternH

/**
 * @file
 * @ingroup GenMod
 * @brief General Pattern and Track definitions
 */

#include "genbase.h"

namespace ppp {

	/**
	 * @class GenCell
	 * @ingroup GenMod
	 * @brief A single note cell
	 */
	class GenCell : public ISerializable {
		public:
			typedef std::shared_ptr<GenCell> Ptr; //!< @brief Class pointer
			typedef std::vector<Ptr> Vector;
		private:
			bool m_active; //!< @brief Is this cell used/relevant?
		public:
			/**
			 * @brief Constructor, sets aActive = @c false
			 */
			GenCell() throw();
			/**
			 * @brief Destructor, does nothing
			 */
			virtual ~GenCell();
			/**
			 * @brief Loads a cell from an input file stream
			 * @param[in,out] str File stream to load from
			 * @return @c true if the cell was successfully loaded
			 */
			virtual bool load(BinStream &str) throw(PppException) = 0;
			/**
			 * @brief Reset the cell so that it is practically "unused"
			 */
			virtual void reset() throw();
			/**
			 * @brief Is this cell active/used?
			 * @return #aActive
			 */
			bool isActive() const throw();
			/**
			 * @brief Get the tracker-like string representation of this cell
			 * @return Tracker-like string
			 */
			virtual std::string trackerString() const throw();
			virtual BinStream &serialize(BinStream &str) const;
			virtual BinStream &unserialize(BinStream &str);
			void setActive(bool a) throw() { m_active = a; }
	};

#if 0
	/**
	 * @brief A list of tracks to represent a pattern
	 * @ingroup GenMod
	 */
	typedef std::vector<GenCell::Vector> GenTrackVector;

	/**
	 * @class GenPattern
	 * @ingroup GenMod
	 * @brief An abstract pattern class
	 */
	class GenPattern {
		public:
			typedef std::shared_ptr<GenPattern> Ptr; //!< @brief Class pointer
			typedef std::vector<Ptr> Vector;
		private:
			GenTrackVector m_tracks; //!< @brief contains the tracks
			/**
			 * @brief Create a cell within the pattern
			 * @param[in] trackIndex Index of the track
			 * @param[in] row Row within the track
			 * @return Pointer to the created (or already existing) cell
			 */
			virtual GenCell::Ptr createCell(int16_t trackIndex, int16_t row) throw(PppException) __attribute__((deprecated("Will be removed in future versions"))) = 0;
		public:
			/**
			 * @brief The constructor
			 */
			GenPattern() throw(PppException);
			/**
			 * @brief The destructor
			 */
			virtual ~GenPattern() throw();
			/**
			 * @brief Load a pattern from a file position
			 * @param[in,out] str The stream to load the pattern from
			 * @param[in] pos Position in the stream
			 * @return @c true on success
			 */
			virtual bool load(BinStream& str, const std::size_t pos) throw(PppException) = 0;
			/**
			 * @brief Get a track within the pattern
			 * @param[in] idx Index of the track
			 * @return Pointer to the track
			 */
			virtual GenCell::Vector* getTrack(int16_t idx) throw();
			/**
			 * @brief Get a cell within the pattern
			 * @param[in] trackIndex Index of the track
			 * @param[in] row Row within the track
			 * @return Pointer to the cell
			 */
			virtual GenCell::Ptr getCell(int16_t trackIndex, int16_t row) throw() __attribute__((deprecated("Will be removed in future versions")));
			void addTrack(const GenCell::Vector &t) __attribute__((deprecated("Will be removed in future versions"))) { m_tracks.push_back(t); }
	};
#endif
} // namespace ppp

#endif
