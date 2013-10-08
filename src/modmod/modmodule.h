/*
    PPPlay - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef MODMODULE_H
#define MODMODULE_H

/**
 * @ingroup ModMod
 * @{
 */

#include "genmod/abstractmodule.h"

namespace ppp
{
namespace mod
{

class ModPattern;
class ModSample;
class ModChannel;

class ModModule : public AbstractModule
{
    DISABLE_COPY( ModModule )
    ModModule() = delete;
    friend class ModChannel;
public:
    /**
     * @brief Factory method
     * @param[in] filename Module filename
     * @param[in] frequency Rendering frequency
     * @param[in] maxRpt Maximum repeat count
     * @return Module pointer or nullptr
     */
    static AbstractModule* factory( Stream* stream, uint32_t frequency, int maxRpt, Sample::Interpolation inter );
private:
    std::vector<ModSample*> m_samples; //!< @brief Samples
    std::vector<ModPattern*> m_patterns; //!< @brief Patterns
    std::vector<ModChannel*> m_channels; //!< @brief Channels
    int8_t m_patLoopRow;
    int m_patLoopCount;
    int8_t m_breakRow;
    int m_patDelayCount;
    uint16_t m_breakOrder;
    bool adjustPosition( bool estimateOnly );
    void checkGlobalFx();
    ModPattern* getPattern( size_t idx ) const;
protected:
    virtual AbstractArchive& serialize( AbstractArchive* data );
public:
    ModModule( int maxRpt, Sample::Interpolation inter );
    virtual ~ModModule();
    bool load( Stream* stream, int loadMode );
private:
    virtual size_t internal_buildTick( AudioFrameBuffer* buf );
    virtual ChannelState internal_channelStatus( size_t idx ) const;
    virtual int internal_channelCount() const;
    ModSample* sampleAt( size_t idx ) const;
    bool existsSample( size_t idx ) const;
    /**
     * @brief Get the logger
     * @return Child logger with attached ".mod"
     */
    static light4cxx::Logger* logger();
};

}
}

/**
 * @}
 */

#endif
