#include "fftobserver.h"

#include "audiofifo.h"
#include "fft.h"

#include <boost/assert.hpp>

FftObserver::FftObserver(AudioFifo* fifo)
  : m_fifo( fifo ), m_buffer( std::make_shared<AudioFrameBuffer>( ppp::FFT::InputLength ) ), m_filled( 0 )
  , m_left( ppp::FFT::InputLength, 0 ), m_right( ppp::FFT::InputLength, 0 ), m_dataPushedConnection()
{
  BOOST_ASSERT( fifo != nullptr );
  m_dataPushedConnection = m_fifo->dataPushed.connect( [this](const auto& x) { dataPushed( x ); } );
}

void FftObserver::dataPushed(const AudioFrameBufferPtr& buffer)
{
  if( !buffer )
  {
    return;
  }
  size_t size = std::min( buffer->size(), ppp::FFT::InputLength - m_filled );
  std::copy( buffer->begin(), buffer->begin() + size, m_buffer->begin() + m_filled );
  m_filled += size;
  if( m_filled == ppp::FFT::InputLength )
  {
    m_filled = 0;
    ppp::FFT::doFFT( m_buffer, &m_left, &m_right );
  }
}