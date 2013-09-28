#include "fftobserver.h"

#include <boost/assert.hpp>
#include "audiofifo.h"

#include "fft.h"

FftObserver::FftObserver( AudioFifo* fifo )
    : m_fifo( fifo ), m_buffer( new AudioFrameBuffer::element_type( ppp::FFT::InputLength ) ), m_filled( 0 )
    , m_left( ppp::FFT::InputLength, 0 ), m_right( ppp::FFT::InputLength, 0 )
{
    BOOST_ASSERT( fifo != nullptr );
    m_fifo->dataPushed.connect( boost::bind( &FftObserver::dataPushed, this, _1 ) );
}

FftObserver::~FftObserver()
{
    m_fifo->dataPushed.disconnect( boost::bind( &FftObserver::dataPushed, this, _1 ) );
}

void FftObserver::dataPushed( const AudioFrameBuffer& buffer )
{
    if( !buffer ) {
        return;
    }
    size_t size = std::min( buffer->size(), ppp::FFT::InputLength - m_filled );
    std::copy( buffer->begin(), buffer->begin() + size, m_buffer->begin() + m_filled );
    m_filled += size;
    if( m_filled == ppp::FFT::InputLength ) {
        m_filled = 0;
        ppp::FFT::doFFT( m_buffer, &m_left, &m_right );
    }
}
