/*
    PPPlay - an old-fashioned module player
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

#ifndef SONGINFO_H
#define SONGINFO_H

#include "ppplay_core_export.h"

#include <stuff/trackingcontainer.h>
#include <stream/abstractarchive.h>
#include <stream/memarchive.h>

namespace ppp
{

/**
 * @ingroup GenMod
 * @{
 */

/**
 * @struct SongInfo
 * @brief Information about a sub-song within a module
 */
struct PPPLAY_CORE_EXPORT SongInfo {
    explicit SongInfo() = default;

    explicit inline SongInfo( SongInfo&& rhs )
        : states( std::move( rhs.states ) )
        , length( rhs.length ) {
        rhs.length = 0;
    }

    ~SongInfo() = default;

    inline SongInfo& operator=( SongInfo rhs ) {
        rhs.swap(*this);
        return *this;
    }
    
    void swap(SongInfo& rhs) {
        std::swap(states, rhs.states);
        std::swap(length, rhs.length);
    }

    //! @brief States for seeking
    TrackingContainer < std::unique_ptr<AbstractArchive> > states{};
    //! @brief Length in sample frames
    size_t length = 0;

    size_t storedSeconds() const noexcept
    {
        return m_storedSeconds;
    }

    bool storeIfNecessary(uint32_t secs, ISerializable* data)
    {
        if(m_storedSeconds + 15 > secs)
        {
            return false;
        }

        m_storedSeconds = secs;
        states.emplace_back(std::make_unique<MemArchive>())->archive(data).finishSave();
        states.next();
        return true;
    }

private:
    size_t m_storedSeconds = 0;
};

/**
 * @}
 */

}

#endif
