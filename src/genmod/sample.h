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

#ifndef PPPLAY_SAMPLE_H
#define PPPLAY_SAMPLE_H

#include "output/audiotypes.h"
#include "light4cxx/logger.h"

namespace ppp
{

class BresenInterpolation;
/**
 * @ingroup GenMod
 * @{
 */

/**
 * @class GenSample
 * @brief An abstract sample class
 */
class Sample
{
	DISABLE_COPY( Sample )
public:
	//! @brief Sample position
	typedef uint_fast32_t PositionType;
	//! @brief Loop type definitions
	enum class LoopType
	{
		None, //!< @brief not looped
		Forward, //!< @brief Forward looped
		Pingpong //!< @brief Ping pong looped
	};
private:
	//! @brief Loop start sample
	PositionType m_loopStart;
	//! @brief Loop end sample (points to 1 frame @e after the loop end)
	PositionType m_loopEnd;
	//! @brief Default volume of the sample
	uint8_t m_volume;
	//! @brief Base frequency of the sample
	uint16_t m_frequency;
	//! @brief Sample data
	BasicSampleFrame::Vector m_data;
	//! @brief Sample filename
	std::string m_filename;
	//! @brief Sample title
	std::string m_title;
	//! @brief Loop type
	LoopType m_looptype;
	/**
	 * @brief Wraps a virtual position of ping-pong looped samples to the real position
	 * @param[in] pos Virtual position
	 * @return Real position
	 * @note Time-critical
	 */
	PositionType makeRealPos( PositionType pos ) const;
	/**
	 * @brief Adjust the playback position so it doesn't fall out of the sample data. Returns EndOfSample if it does
	 * @param[in,out] pos Reference to the variable that should be adjusted
	 * @return Adjusted position
	 * @note Time-critical
	 */
	PositionType adjustPosition( PositionType pos ) const;
	/**
	 * @brief Get a sample
	 * @param[in,out] pos Position of the requested sample
	 * @return Sample value, 0 if invalid value for @a pos
	 */
	inline BasicSampleFrame sampleAt( PositionType pos ) const;
public:
	/**
	 * @brief Constructor
	 */
	Sample();
	/**
	 * @brief Destructor
	 */
	virtual ~Sample();
	/**
	 * @brief Get the sample's Base Frequency
	 * @return Base frequency
	 */
	uint16_t frequency() const;
	/**
	 * @brief Get the sample's default volume
	 * @return Default volume
	 */
	uint8_t volume() const;
	/**
	 * @brief Get the sample's name
	 * @return Sample's name
	 */
	std::string title() const;
	/**
	 * @brief Is the sample looped?
	 * @return @c true if the sample is looped
	 */
	bool isLooped() const;
	/**
	 * @brief Get the sample's length
	 * @return The sample's length
	 */
	PositionType length() const;
	/**
	 * @brief Get the loop type
	 * @return The loop type
	 */
	LoopType loopType() const;
	
	bool mixNonInterpolated( BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const;
	bool mixLinearInterpolated( BresenInterpolation* bresen, MixerFrameBuffer* buffer, int factorLeft, int factorRight, int rightShift ) const;
protected:
	typedef BasicSampleFrame::Vector::iterator Iterator;
	typedef BasicSampleFrame::Vector::const_iterator ConstIterator;
	/**
	 * @brief Set m_frequency
	 * @param[in] f The new frequency value
	 */
	void setFrequency( uint16_t f );
	/**
	 * @brief Set m_looptype
	 * @param[in] l The new loop type value
	 */
	void setLoopType( LoopType l );
	/**
	 * @brief Get data start iterator
	 * @return Data start iterator
	 */
	inline Iterator beginIterator() {
		return m_data.begin();
	}
	/**
	 * @brief Get data end iterator
	 * @return Data end iterator
	 */
	inline Iterator endIterator() {
		return m_data.end();
	}
	/**
	 * @brief Get data start iterator
	 * @return Data start iterator
	 */
	inline ConstIterator beginIterator() const {
		return m_data.begin();
	}
	/**
	 * @brief Get data end iterator
	 * @return Data end iterator
	 */
	inline ConstIterator endIterator() const {
		return m_data.cend();
	}
	/**
	 * @brief Set the sample's name
	 * @param[in] t The new name
	 */
	void setTitle( const std::string& t );
	/**
	 * @brief Set the sample's filename
	 * @param[in] f The new filename
	 */
	void setFilename( const std::string& f );
	/**
	 * @brief Set the sample's loop start
	 * @param[in] s The new loop start
	 */
	void setLoopStart( PositionType s );
	/**
	 * @brief Set the sample's loop end
	 * @param[in] e The new loop end
	 */
	void setLoopEnd( PositionType e );
	/**
	 * @brief Set the sample's default volume
	 * @param[in] v The new volume
	 */
	void setVolume( uint8_t v );
	/**
	 * @brief Resize the data
	 * @param[in] size New size
	 */
	inline void resizeData( PositionType size ) {
		m_data.resize( size );
	}
	/**
	 * @brief Get the logger
	 * @return Logger with name "sample"
	 */
	static light4cxx::Logger* logger();
};

/**
 * @}
 */

} // namespace ppp

#endif
