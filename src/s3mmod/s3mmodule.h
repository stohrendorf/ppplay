/*
    PPPlay - an old-fashioned module player
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

#ifndef S3MMODULE_H
#define S3MMODULE_H

/**
 * @ingroup S3mMod
 * @{
 */

#include "genmod/abstractmodule.h"

#include <array>

namespace ppp
{
namespace s3m
{
class S3mSample;

class S3mPattern;

class S3mChannel;

/**
 * @class S3mModule
 * @brief Module class for S3M Modules
 */
class S3mModule
    : public AbstractModule
{
    friend class S3mChannel;

public:
    DISABLE_COPY(S3mModule)

    S3mModule() = delete;

    /**
     * @copydoc ppp::GenModule::GenModule(uint8_t)
     */
    S3mModule(int maxRpt, Sample::Interpolation inter);

    /**
     * @brief Factory method
     * @param[in] filename Module filename
     * @param[in] frequency Rendering frequency
     * @param[in] maxRpt Maximum repeat count
     * @return Module pointer or nullptr
     */
    static std::shared_ptr<AbstractModule> factory(Stream* stream, uint32_t frequency, int maxRpt, Sample::Interpolation inter);

private:
    uint16_t m_breakRow;      //!< @brief Row to break to, ~0 if unused
    uint16_t m_breakOrder;    //!< @brief Order to break to, ~0 if unused
    int16_t m_patLoopRow;    //!< @brief Row to loop back, -1 if unused
    int16_t m_patLoopCount;  //!< @brief Loop counter for pattern loop, -1 if unused
    int16_t m_patDelayCount; //!< @brief Delay counter for Pattern Delay, -1 if unused
    bool m_customData;     //!< @brief @c true if module contains special custom data
    std::vector<std::unique_ptr<S3mSample>> m_samples; //!< @brief Samples
    std::vector<std::unique_ptr<S3mPattern>> m_patterns; //!< @brief Patterns
    std::array<std::unique_ptr<S3mChannel>, 32> m_channels; //!< @brief Channels
    uint8_t m_usedChannels; //!< @brief Number of used channels
    bool m_amigaLimits; //!< @brief @c true if amiga limits are present
    bool m_fastVolSlides; //!< @brief @c true if fast volume slides are present
    bool m_st2Vibrato; //!< @brief @c true if ScreamTracker v2 vibrato is present
    bool m_zeroVolOpt; //!< @brief @c true if zero volume optimization is present
    /**
     * @brief Get a pattern
     * @param[in] idx Pattern index of the requested pattern
     * @return Pattern pointer or nullptr
     */
    const std::unique_ptr<S3mPattern>& getPattern(size_t idx) const;

protected:
    AbstractArchive& serialize(AbstractArchive* data) override;

public:
    ~S3mModule() override;

private:
    int internal_channelCount() const override;

    size_t internal_buildTick(const AudioFrameBufferPtr& buffer) override;

    ChannelState internal_channelStatus(size_t idx) const override;

    /**
     * @brief Apply global effects
     */
    void checkGlobalFx();

    /**
     * @brief Adjust the playback position
     * @param[in] estimateOnly Used when estimating track length
     * @retval false if the end of the current song is reached
     * @retval true otherwise
     */
    bool adjustPosition();

    /**
     * @brief Load the module
     * @param[in] fn Filename of the module to load
     * @return @c true on success
     */
    bool load(Stream* stream);

    /**
     * @brief Check if a sample exists
     * @param[in] idx Sample index to check
     * @retval true if the sample exists
     * @retval false otherwise
     */
    bool existsSample(int16_t idx);

    /**
     * @brief Check if amiga limits are present
     * @return m_amigaLimits
     */
    bool hasAmigaLimits() const;

    /**
     * @brief Check if fast volume slides are present
     * @return m_fastVolSlides
     */
    bool hasFastVolSlides() const;

    /**
     * @brief Check if ScreamTracker v2 Vibrato is present
     * @return m_st2Vibrato
     */
    bool st2Vibrato() const;

    /**
     * @brief Get the maximum sample number
     * @return Maximum sample number
     */
    size_t numSamples() const;

    /**
     * @brief Get a sample
     * @param[in] idx Sample index
     * @return Sample pointer or nullptr
     */
    const std::unique_ptr<S3mSample>& sampleAt(size_t idx) const;

    /**
     * @brief Check if zero volume optimizations are present
     * @return m_zeroVolOpt
     */
    bool hasZeroVolOpt() const;

    /**
     * @brief Get the logger
     * @return Child logger with attached ".s3m"
     */
    static light4cxx::Logger* logger();
};
} // namespace s3m
} // namespace ppp

/**
 * @}
 */

#endif
