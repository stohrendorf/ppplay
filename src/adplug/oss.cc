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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/soundcard.h>

#include "oss.h"

#define DEFAULT_DEVICE	"/dev/dsp"	// Default output device file

OSSPlayer::OSSPlayer(Copl *nopl, const char *device, unsigned char bits,
		     int channels, int freq, unsigned long bufsize)
  : EmuPlayer(nopl, bits, channels, freq, bufsize)
{
  int format = (bits == 16 ? AFMT_S16_LE : AFMT_S8);

  // Set to default if no device given
  if(!device) device = DEFAULT_DEVICE;

  // open OSS audio device
  if((audio_fd = open(device, O_WRONLY, 0)) == -1) {
    perror(device);
    exit(EXIT_FAILURE);
  }

  ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
  ioctl(audio_fd, SOUND_PCM_WRITE_CHANNELS, &channels);
  ioctl(audio_fd, SNDCTL_DSP_SPEED, &freq);
}

OSSPlayer::~OSSPlayer()
{
  close(audio_fd);
}

void OSSPlayer::output(const void *buf, unsigned long size)
{
  write(audio_fd, buf, size);
}
