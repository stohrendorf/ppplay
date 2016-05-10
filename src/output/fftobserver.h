/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2013  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PPPLAY_FFTOBSERVER_H
#define PPPLAY_FFTOBSERVER_H

#include "ppplay_core_export.h"
#include "audiotypes.h"
#include <stuff/utils.h>
#include <boost/signals2/connection.hpp>

class AudioFifo;

/**
 * @ingroup Output
 * @{
 */

 /**
  * @class FftObserver
  * @brief Observer for calculating the DFFT
  */
class PPPLAY_CORE_EXPORT FftObserver
{
    DISABLE_COPY(FftObserver)
private:
    //! @brief Observed FIFO
    AudioFifo* m_fifo;
    AudioFrameBuffer m_buffer;
    size_t m_filled;
    std::vector<uint16_t> m_left;
    std::vector<uint16_t> m_right;
    boost::signals2::scoped_connection m_dataPushedConnection;
public:
    explicit FftObserver(AudioFifo* fifo);
    ~FftObserver() = default;

    const std::vector<uint16_t>& left() const
    {
        return m_left;
    }

    const std::vector<uint16_t>& right() const
    {
        return m_right;
    }
private:
    void dataPushed(const AudioFrameBuffer& buffer);
};

/**
 * @}
 */

#endif
