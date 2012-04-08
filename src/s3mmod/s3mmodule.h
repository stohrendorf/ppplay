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

#ifndef S3MMODULE_H
#define S3MMODULE_H

/**
 * @ingroup S3mMod
 * @{
 */

#include "genmod/genmodule.h"

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
class S3mModule : public GenModule
{
	DISABLE_COPY( S3mModule )
	S3mModule() = delete;
	friend class S3mChannel;
public:
	typedef std::shared_ptr<S3mModule> Ptr; //!< @brief Class pointer
	/**
	 * @brief Factory method
	 * @param[in] filename Module filename
	 * @param[in] frequency Rendering frequency
	 * @param[in] maxRpt Maximum repeat count
	 * @return Module pointer or nullptr
	 */
	static GenModule::Ptr factory( const std::string& filename, uint32_t frequency, int maxRpt = 2 );
private:
	uint16_t m_breakRow;      //!< @brief Row to break to, ~0 if unused
	uint16_t m_breakOrder;    //!< @brief Order to break to, ~0 if unused
	int16_t m_patLoopRow;    //!< @brief Row to loop back, -1 if unused
	int16_t m_patLoopCount;  //!< @brief Loop counter for pattern loop, -1 if unused
	int16_t m_patDelayCount; //!< @brief Delay counter for Pattern Delay, -1 if unused
	bool m_customData;     //!< @brief @c true if module contains special custom data
	std::vector<S3mSample*> m_samples; //!< @brief Samples
	std::vector<S3mPattern*> m_patterns; //!< @brief Patterns
	std::array<S3mChannel*, 32> m_channels; //!< @brief Channels
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
	S3mPattern* getPattern( size_t idx ) const;
protected:
	virtual IArchive& serialize( IArchive* data );
public:
	virtual ~S3mModule();
private:
	virtual uint8_t internal_channelCount() const;
	virtual size_t internal_buildTick( AudioFrameBuffer* buf );
	virtual std::string internal_channelStatus( size_t idx ) const;
	virtual std::string internal_channelCellString( size_t idx ) const;
	/**
	 * @copydoc ppp::GenModule::GenModule(uint8_t)
	 */
	S3mModule( int maxRpt = 2 );
	/**
	 * @brief Apply global effects
	 */
	void checkGlobalFx(  );
	/**
	 * @brief Adjust the playback position
	 * @param[in] estimateOnly Used when estimating track length
	 * @retval false if the end of the current song is reached
	 * @retval true otherwise
	 */
	bool adjustPosition( bool estimateOnly );
	/**
	 * @brief Load the module
	 * @param[in] fn Filename of the module to load
	 * @return @c true on success
	 */
	bool load( const std::string& fn );
	/**
	 * @brief Check if a sample exists
	 * @param[in] idx Sample index to check
	 * @retval true if the sample exists
	 * @retval false otherwise
	 */
	bool existsSample( int16_t idx );
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
	const S3mSample* sampleAt( size_t idx ) const;
	/**
	 * @brief Check if zero volume optimizations are present
	 * @return m_zeroVolOpt
	 */
	bool hasZeroVolOpt() const;
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".s3m"
	 */
	static light4cxx::Logger::Ptr logger();
};
} // namespace s3m
} // namespace ppp

/**
 * @}
 */

#endif
