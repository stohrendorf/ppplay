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

#ifndef XMINSTRUMENT_H
#define XMINSTRUMENT_H

/**
 * @ingroup XmModule
 * @{
 */

#include "xmenvelopeprocessor.h"

#include "light4cxx/logger.h"

class BinStream;

namespace ppp
{
namespace xm
{

class XmSample;

/**
 * @class XmInstrument
 * @brief XM Instrument class
 */
class XmInstrument
{
	DISABLE_COPY( XmInstrument )
private:
	//! @brief Samples this instrument contains
	std::vector<XmSample*> m_samples;
	//! @brief Note map that maps notes to the samples
	uint8_t m_map[96];
	//! @brief This instrument's title
	std::string m_title;
	//! @brief Panning envelope flags
	XmEnvelopeProcessor::EnvelopeFlags m_panEnvFlags;
	//! @brief Volume envelope flags
	XmEnvelopeProcessor::EnvelopeFlags m_volEnvFlags;
	//! @brief Panning envelope points
	std::array<XmEnvelopeProcessor::EnvelopePoint, 12> m_panPoints;
	//! @brief Volume envelope points
	std::array<XmEnvelopeProcessor::EnvelopePoint, 12> m_volPoints;
	//! @brief Number of volume envelope points
	uint8_t m_numVolPoints;
	//! @brief Number of panning envelope points
	uint8_t m_numPanPoints;
	//! @brief Volume envelope loop start
	uint8_t m_volLoopStart;
	//! @brief Panning envelope loop start
	uint8_t m_panLoopStart;
	//! @brief Volume envelope loop end
	uint8_t m_volLoopEnd;
	//! @brief Panning envelope loop end
	uint8_t m_panLoopEnd;
	//! @brief Volume envelope sustain point index
	uint8_t m_volSustainPoint;
	//! @brief Panning envelope sustain point index
	uint8_t m_panSustainPoint;
	//! @brief Volume fadeout value
	uint16_t m_fadeout;
	//! @brief Auto vibrato rate
	uint8_t m_vibRate;
	//! @brief Auto vibrato depth
	uint8_t m_vibDepth;
	//! @brief Auto vibrato sweep
	uint8_t m_vibSweep;
	//! @brief Auto vibrato type
	uint8_t m_vibType;
public:
	//! @brief Constructor
	XmInstrument();
	~XmInstrument();
	/**
	 * @brief Load this instrument from a stream
	 * @param[in] str The stream to load from
	 * @return @c true on success
	 */
	bool load( BinStream& str );
	/**
	 * @brief Map a note index into its sample index
	 * @param[in] note Note to map
	 * @return Mapped sample index
	 * @note @a note is zero-based!
	 */
	uint8_t mapNoteIndex( uint8_t note ) const;
	/**
	 * @brief Map a note index into its sample
	 * @param[in] note Note to map
	 * @return Mapped sample
	 * @note @a note is zero-based!
	 */
	XmSample* mapNoteSample( uint8_t note ) const;
	/**
	 * @brief Get the instrument's title
	 * @return m_title
	 */
	std::string title() const;
	/**
	 * @brief Get the volume fadeout value
	 * @return m_fadeout
	 */
	uint16_t fadeout() const ;
	/**
	 * @brief Create the volume envelope processor
	 * @return Volume envelope processor
	 */
	XmEnvelopeProcessor volumeProcessor() const;
	/**
	 * @brief Create the panning envelope processor
	 * @return Volume envelope processor
	 */
	XmEnvelopeProcessor panningProcessor() const;
	/**
	 * @brief Get the auto vibrato rate
	 * @return m_vibRate
	 */
	uint8_t vibRate() const;
	/**
	 * @brief Get the auto vibrato depth
	 * @return m_vibDepth
	 */
	uint8_t vibDepth() const;
	/**
	 * @brief Get the auto vibrato sweep value
	 * @return m_vibSweep
	 */
	uint8_t vibSweep() const;
	/**
	 * @brief Get the auto vibrato type
	 * @return m_vibType
	 */
	uint8_t vibType() const;
protected:
	/**
	 * @brief Get the logger
	 * @return Child logger with attached ".xm"
	 */
	static light4cxx::Logger::Ptr logger();
};

}
}

/**
 * @}
 */

#endif // XMINSTRUMENT_H
