/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2003 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "sdl.h"

#include "light4cxx/logger.h"

namespace
{
light4cxx::Logger* logger = light4cxx::Logger::get( "badplay.output.sdl" );
}

SDLPlayer::SDLPlayer(int freq, size_t bufsize)
  : m_interp( opl::Opl3::SampleRate, freq )
{
  memset( &m_spec, 0x00, sizeof(SDL_AudioSpec) );

  if( SDL_Init( SDL_INIT_AUDIO ) < 0 )
  {
    logger->fatal( L4CXX_LOCATION, "unable to initialize SDL -- %s", SDL_GetError() );
    exit( EXIT_FAILURE );
  }

  m_spec.freq = freq;
  m_spec.format = AUDIO_S16SYS;
  m_spec.channels = 2;
  BOOST_ASSERT( bufsize <= std::numeric_limits<Uint16>::max() );
  m_spec.samples = static_cast<Uint16>(bufsize);
  m_spec.callback = &SDLPlayer::callback;
  m_spec.userdata = this;

  if( SDL_OpenAudio( &m_spec, &m_spec ) < 0 )
  {
    logger->fatal( L4CXX_LOCATION, "unable to open audio -- %s", SDL_GetError() );
    exit( EXIT_FAILURE );
  }

  logger->debug( L4CXX_LOCATION, "got audio buffer size -- %d", m_spec.size );
}

SDLPlayer::~SDLPlayer()
{
  if( !SDL_WasInit( SDL_INIT_AUDIO ) )
  {
    return;
  }

  SDL_CloseAudio();
  SDL_Quit();
}

void SDLPlayer::frame()
{
  SDL_PauseAudio( 0 );
  SDL_Delay( m_spec.freq / (m_spec.size / 4) );
}

void SDLPlayer::callback(void* userdata, Uint8* audiobuf, int byteLen)
{
  auto self = reinterpret_cast<SDLPlayer*>(userdata);
  static long framesUntilUpdate = 0;
  size_t framesToWrite = byteLen / 4u;
  auto* bufPtr = reinterpret_cast<int16_t*>(audiobuf);
  // Prepare audiobuf with emulator output
  while( framesToWrite > 0 )
  {
    while( framesUntilUpdate <= 0 )
    {
      self->setIsPlaying( self->getPlayer()->update() );
      framesUntilUpdate += self->getPlayer()->framesUntilUpdate();
    }

    std::array<int16_t, 4> samples;
    self->getPlayer()->read( &samples );
    bufPtr[0] = ppp::clip( samples[0] + samples[2], -32768, 32767 );
    bufPtr[1] = ppp::clip( samples[1] + samples[3], -32768, 32767 );
    bufPtr += 2;

    if( self->m_interp.next() == 2 )
    {
      self->getPlayer()->read( nullptr ); // skip a sample
      --framesUntilUpdate;
    }
    self->m_interp = 0;
    --framesUntilUpdate;
    --framesToWrite;
  }
}