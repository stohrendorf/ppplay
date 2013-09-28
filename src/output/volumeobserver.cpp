#include "volumeobserver.h"

#include <boost/assert.hpp>
#include "audiofifo.h"

namespace
{
/**
 * @brief Makes volume values logarithmic
 * @param[in] value Volume to make logarithmic
 * @return A more "natural" feeling value
 * @see AudioFifo::calcVolume()
 */
uint16_t logify( uint16_t value )
{
    uint32_t tmp = value << 1;
    if( tmp > 0x8000 )
        tmp = ( 0x8000 + tmp ) >> 1;
    if( tmp > 0xb000 )
        tmp = ( 0xb000 + tmp ) >> 1;
    if( tmp > 0xe000 )
        tmp = ( 0xe000 + tmp ) >> 1;
    return tmp > 0xffff ? 0xffff : tmp;
}

/**
 * @brief Sums up the absolute sample values of an AudioFrameBuffer
 * @param[in] buf The buffer with the values to sum up
 * @param[out] left Sum of absolute left sample values
 * @param[out] right Sum of absolute right sample values
 */
void sumAbsValues( const AudioFrameBuffer& buf, uint64_t& left, uint64_t& right )
{
    left = right = 0;
    for( const BasicSampleFrame & frame : *buf ) {
        left += std::abs( frame.left );
        right += std::abs( frame.right );
    }
}
}

VolumeObserver::VolumeObserver( AudioFifo* fifo )
    : m_volLeftSum( 0 ), m_volLeftLog( 0 ), m_volRightSum( 0 ), m_volRightLog( 0 ), m_fifo( fifo )
{
    BOOST_ASSERT( fifo != nullptr );
    m_fifo->dataPulled.connect( boost::bind( &VolumeObserver::dataPulled, this, _1 ) );
    m_fifo->dataPushed.connect( boost::bind( &VolumeObserver::dataPushed, this, _1 ) );
}

VolumeObserver::~VolumeObserver()
{
    m_fifo->dataPulled.disconnect( boost::bind( &VolumeObserver::dataPulled, this, _1 ) );
    m_fifo->dataPushed.disconnect( boost::bind( &VolumeObserver::dataPushed, this, _1 ) );
}

void VolumeObserver::dataPulled( const AudioFrameBuffer& buffer )
{
    uint64_t left = 0, right = 0;
    sumAbsValues( buffer, left, right );
    m_volLeftSum -= left;
    m_volRightSum -= right;
    if( m_fifo->queuedLength() < 4 ) {
        m_volLeftLog = m_volRightLog = 0;
    }
    else {
        m_volLeftLog = logify( m_volLeftSum / ( m_fifo->queuedLength() >> 2 ) );
        m_volRightLog = logify( m_volRightSum / ( m_fifo->queuedLength() >> 2 ) );
    }
}

void VolumeObserver::dataPushed( const AudioFrameBuffer& buffer )
{
    uint64_t left = 0, right = 0;
    sumAbsValues( buffer, left, right );
    m_volLeftSum += left;
    m_volRightSum += right;
    if( m_fifo->queuedLength() < 4 ) {
        m_volLeftLog = m_volRightLog = 0;
    }
    else {
        m_volLeftLog = logify( m_volLeftSum / ( m_fifo->queuedLength() >> 2 ) );
        m_volRightLog = logify( m_volRightSum / ( m_fifo->queuedLength() >> 2 ) );
    }
}
