/*
    PeePeePlayer - an old-fashioned module player
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


#ifndef XMENVELOPEPROCESSOR_H
#define XMENVELOPEPROCESSOR_H

/**
 * @ingroup XmModule
 * @{
 */

#include "stream/iserializable.h"

#include <array>
#include <cstdint>

namespace ppp
{
namespace xm
{

/**
 * @class XmEnvelopeProcessor
 * @brief XM Envelope helper class
 */
class XmEnvelopeProcessor : public ISerializable
{
public:
    //! @brief Envelope flags
    enum class EnvelopeFlags : uint8_t
    {
        Enabled = 0x01, //!< @brief Envelope is enabled
        Sustain = 0x02, //!< @brief Envelope has a sustain point
        Loop = 0x04 //!< @brief Envelope is looped
    };
    //! @brief An envelope point
    struct EnvelopePoint {
        int16_t position; //!< @brief Position of the point
        int16_t value; //!< @brief Value of the point
    };
private:
    //! @brief Envelope flags
    EnvelopeFlags m_flags;
    //! @brief The envelope points
    std::array<EnvelopePoint, 12> m_points;
    //! @brief Number of used envelope points
    uint8_t m_numPoints;
    //! @brief Current position in the envelope
    int16_t m_position;
    //! @brief Index of the next point to process
    uint8_t m_nextIndex;
    //! @brief Sustain point index
    uint8_t m_sustainPoint;
    //! @brief Sustain loop start point index
    uint8_t m_loopStart;
    //! @brief Sustain loop end point index
    uint8_t m_loopEnd;
    //! @brief Current changing rate between two points
    int16_t m_currentRate;
    //! @brief Current envelope value, scaled by 0x100
    uint16_t m_currentValue;
    /**
     * @brief Check if we are on the sustain point
     * @param[in] idx Point index
     * @return @c true if on the sustain point
     */
    bool onSustain( uint8_t idx ) const;
    /**
     * @brief Check if we are at the loop end
     * @param[in] idx Point idx
     * @return @c true if at the loop end
     */
    bool atLoopEnd( uint8_t idx ) const;
public:
    /**
     * @brief Constructor, default is to disable this envelope
     */
    XmEnvelopeProcessor();
    /**
     * @overload
     * @brief Constructor
     * @param[in] flags Envelope flags
     * @param[in] points Envelope points
     * @param[in] numPoints Number of used envelope points
     * @param[in] sustainPt Sustain point index
     * @param[in] loopStart Loop start point index
     * @param[in] loopEnd Loop end point index
     */
    XmEnvelopeProcessor( EnvelopeFlags flags, const std::array<EnvelopePoint, 12>& points, uint8_t numPoints, uint8_t sustainPt, uint8_t loopStart, uint8_t loopEnd );
    /**
     * @brief Advance the envelope position if needed
     * @param[in] keyOn If the keyboard key is currently pressed
     */
    void increasePosition( bool keyOn );
    /**
     * @brief If the envelope is enabled
     * @return @c true if m_flags contains EnvelopFlags::Enabled
     */
    bool enabled() const;
    /**
     * @brief Calculate the real output volume
     * @param[in] volume Channel volume
     * @param[in] globalVolume Global module volume
     * @param[in] scale Volume scale value
     * @return The real volume used for mixing (0..64)
     */
    uint8_t realVolume( uint8_t volume, uint8_t globalVolume, uint16_t scale );
    /**
     * @brief Calculate the real panning value
     * @param[in] panning The channel panning
     * @return The real panning
     */
    uint8_t realPanning( uint8_t panning );
    /**
     * @brief Set the envelope position
     * @param[in] pos The requested position (not the point index)
     */
    void setPosition( uint8_t pos );
    virtual AbstractArchive& serialize( AbstractArchive* data );
    void doKeyOff();
    void retrigger();
};

/**
 * @brief Convenienve operator for checking the flags
 * @param[in] a Flags value 1
 * @param[in] b Flags value 2
 * @return @c true if @c a&b is not 0
 */
inline bool operator&( const XmEnvelopeProcessor::EnvelopeFlags& a, const XmEnvelopeProcessor::EnvelopeFlags& b )
{
    return ( static_cast<uint8_t>( a ) & static_cast<uint8_t>( b ) ) != 0;
}

}
}

/**
 * @}
 */

#endif // XMENVELOPEPROCESSOR_H
