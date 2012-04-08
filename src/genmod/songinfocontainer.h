/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef SONGINFOCONTAINER_H
#define SONGINFOCONTAINER_H

#include "songinfo.h"

/**
 * @ingroup GenMod
 * @{
 */

namespace ppp
{

/**
 * @class SongInfoContainer
 * @brief Container for song states
 */
class SongInfoContainer
{
	DISABLE_COPY(SongInfoContainer)
private:
	//! @brief Per-song infos
	std::vector<SongInfo> m_songs;
	//! @brief The current song index in m_songs
	int16_t m_currentSongIndex;
	//! @brief Initial module state
	IArchive::Ptr m_initialState;
	
public:
	explicit SongInfoContainer();
	
	/**
	 * @brief Set the current song index
	 * @param[in] idx New index
	 * @throw std::out_of_range if idx is invalid
	 */
	void setIndex(uint16_t idx);
	
	/**
	 * @brief Get the current song index
	 * @return m_currentSongIndex
	 */
	int16_t index() const;
	
	/**
	 * @brief Getter for song information
	 * @param[in] idx Song index
	 * @return SongInfo for song with index idx
	 * @throw std::out_of_range if idx is invalid
	 */
	const SongInfo& at(size_t idx) const;
	
	/**
	 * @brief Getter for song information
	 * @param[in] idx Song index
	 * @return SongInfo for song with index idx
	 * @throw std::out_of_range if idx is invalid
	 */
	SongInfo& at(size_t idx);

	/**
	 * @brief Get current song information
	 * @return SongInfo for song index m_currentSongIndex
	 * @throw std::out_of_range if the container is empty
	 */
	const SongInfo& current() const;
	
	/**
	 * @brief Get current song information
	 * @return SongInfo for song index m_currentSongIndex
	 * @throw std::out_of_range if the container is empty
	 */
	SongInfo& current();
	
	/**
	 * @brief Append a SongInfo to the container
	 * @param[in] info SongInfo to append
	 */
	void append(const SongInfo& info);
	
	/**
	 * @brief Get the number of contained songs
	 * @return m_songs.size()
	 */
	size_t size() const;
	
	/**
	 * @brief Get the initial state archive
	 * @return m_initialState
	 */
	IArchive::Ptr initialState() const;
	
	/**
	 * @brief Removes songs where SongInfo::length == 0
	 */
	void removeEmptySongs();
};

}

/**
 * @}
 */

#endif
