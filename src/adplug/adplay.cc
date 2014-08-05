/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2007 Simon Peter <dn.tlp@gmx.net>
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
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include "adplug.h"
#include "emuopl.h"

#include <getopt.h>

#include "defines.h"
#include "output.h"
#include "cli-players.h"

/***** Defines *****/

// Default file name of AdPlug's database file
#define ADPLUGDB_FILE		"adplug.db"

// Default AdPlug user's configuration subdirectory
#define ADPLUG_CONFDIR		".adplug"

// Default path to AdPlug's system-wide database file
#ifdef ADPLUG_DATA_DIR
#  define ADPLUGDB_PATH		ADPLUG_DATA_DIR "/" ADPLUGDB_FILE
#else
#  define ADPLUGDB_PATH		ADPLUGDB_FILE
#endif

/***** Typedefs *****/

typedef enum { Emu_Satoh, Emu_Ken } EmuType;

/***** Global variables *****/

static const char	*program_name;
static Player		*player = 0;		// global player object
static CAdPlugDatabase	mydb;
static Copl		*opl = 0;

/***** Configuration (and defaults) *****/

static struct {
  int			buf_size, freq, channels, bits, harmonic, message_level;
  unsigned int		subsong;
  const char		*device;
  char			*userdb;
  bool			endless, showinsts, songinfo, songmessage;
  EmuType		emutype;
  Outputs		output;
} cfg = {
  2048, 44100,
  1, 16, 0,  // Else default to mono (until stereo w/ single OPL is fixed)
  MSG_NOTE,
  -1,
  NULL,
  NULL,
  true, false, false, false,
  Emu_Satoh,
  DEFAULT_DRIVER
};

/***** Global functions *****/

void message(int level, const char *fmt, ...)
{
  va_list argptr;

  if(cfg.message_level < level) return;

  fprintf(stderr, "%s: ", program_name);
  va_start(argptr, fmt);
  vfprintf(stderr, fmt, argptr);
  va_end(argptr);
  fprintf(stderr, "\n");
}

/***** Local functions *****/

static void usage()
/* Print usage information. */
{
  printf("Usage: %s [OPTION]... FILE...\n\n"
	 "Output selection:\n"
	 "  -e, --emulator=EMULATOR    specify emulator to use\n"
	 "  -O, --output=OUTPUT        specify output mechanism\n\n"
	 "OSS driver (oss) specific:\n"
	 "  -d, --device=FILE          set sound device file to FILE\n"
	 "  -b, --buffer=SIZE          set output buffer size to SIZE\n\n"
	 "Disk writer (disk) specific:\n"
	 "  -d, --device=FILE          output to FILE ('-' is stdout)\n\n"
	 "EsounD driver (esound) specific:\n"
	 "  -d, --device=URL           URL to EsounD server host (hostname:port)\n\n"
	 "SDL driver (sdl) specific:\n"
	 "  -b, --buffer=SIZE          set output buffer size to SIZE\n\n"
	 "ALSA driver (alsa) specific:\n"
	 "  -d, --device=DEVICE        set sound device to DEVICE\n"
	 "  -b, --buffer=SIZE          set output buffer size to SIZE\n\n"
	 "Playback quality:\n"
	 "  -8, --8bit                 8-bit sample quality\n"
	 "      --16bit                16-bit sample quality\n"
	 "  -f, --freq=FREQ            set sample frequency to FREQ\n"
 	 "      --surround             stereo/surround stream\n"
	 "      --stereo               stereo stream\n"
	 "      --mono                 mono stream\n\n"
	 "Informative output:\n"
	 "  -i, --instruments          display instrument names\n"
	 "  -r, --realtime             display realtime song info\n"
	 "  -m, --message              display song message\n\n"
	 "Playback:\n"
	 "  -s, --subsong=N            play subsong number N\n"
	 "  -o, --once                 play only once, don't loop\n\n"
	 "Generic:\n"
	 "  -D, --database=FILE        additionally use database file FILE\n"
	 "  -q, --quiet                be more quiet\n"
	 "  -v, --verbose              be more verbose\n"
	 "  -h, --help                 display this help and exit\n"
	 "  -V, --version              output version information and exit\n\n",
	 program_name);

  // Print list of available output mechanisms
  printf("Available emulators: satoh ken\n");
  printf("Available output mechanisms: "
#ifdef DRIVER_OSS
	 "oss "
#endif
#ifdef DRIVER_NULL
	 "null "
#endif
#ifdef DRIVER_DISK
	 "disk "
#endif
#ifdef DRIVER_ESOUND
	 "esound "
#endif
#ifdef DRIVER_QSA
	 "qsa "
#endif
#ifdef DRIVER_SDL
	 "sdl "
#endif
#ifdef DRIVER_AO
	 "ao "
#endif
#ifdef DRIVER_ALSA
	 "alsa "
#endif
	 "\n");
}

static int decode_switches(int argc, char **argv)
/*
 * Set all the option flags according to the switches specified.
 * Return the index of the first non-option argument.
 */
{
  int c;
  struct option const long_options[] = {
    {"8bit", no_argument, NULL, '8'},		// 8-bit replay
    {"16bit", no_argument, NULL, '1'},		// 16-bit replay
    {"freq", required_argument, NULL, 'f'},	// set frequency
    {"surround", no_argument, NULL, '4'},		// stereo/harmonic replay
    {"stereo", no_argument, NULL, '3'},		// stereo replay
    {"mono", no_argument, NULL, '2'},		// mono replay
    {"buffer", required_argument, NULL, 'b'},	// buffer size
    {"device", required_argument, NULL, 'd'},	// device file
    {"instruments", no_argument, NULL, 'i'},	// show instruments
    {"realtime", no_argument, NULL, 'r'},	// realtime song info
    {"message", no_argument, NULL, 'm'},	// song message
    {"subsong", no_argument, NULL, 's'},	// play subsong
    {"once", no_argument, NULL, 'o'},		// don't loop
    {"help", no_argument, NULL, 'h'},		// display help
    {"version", no_argument, NULL, 'V'},	// version information
    {"emulator", required_argument, NULL, 'e'},	// emulator to use
    {"output", required_argument, NULL, 'O'},	// output mechanism
    {"database", required_argument, NULL, 'D'},	// different database
    {"quiet", no_argument, NULL, 'q'},		// be more quiet
    {"verbose", no_argument, NULL, 'v'},	// be more verbose
    {NULL, 0, NULL, 0}				// end of options
  };

  while ((c = getopt_long(argc, argv, "8f:b:d:irms:ohVe:O:D:qv",
			  long_options, (int *)0)) != EOF) {
      switch (c) {
      case '8': cfg.bits = 8; break;
      case '1': cfg.bits = 16; break;
      case 'f': cfg.freq = atoi(optarg); break;
      case '4': cfg.channels = 2; cfg.harmonic = 1; break;
      case '3': cfg.channels = 2; cfg.harmonic = 0; break;
      case '2': cfg.channels = 1; cfg.harmonic = 0; break;
      case 'b': cfg.buf_size = atoi(optarg); break;
      case 'd': cfg.device = optarg; break;
      case 'i': cfg.showinsts = true; break;
      case 'r': cfg.songinfo = true; break;
      case 'm': cfg.songmessage = true; break;
      case 's': cfg.subsong = atoi(optarg); break;
      case 'o': cfg.endless = false; break;
      case 'V': puts(BADPLAY_VERSION); exit(EXIT_SUCCESS);
      case 'h':	usage(); exit(EXIT_SUCCESS); break;
      case 'D':
	if(!mydb.load(optarg))
	  message(MSG_WARN, "could not open database -- %s", optarg);
	break;
      case 'O':
	if(!strcmp(optarg,"disk")) {
	  cfg.output = disk;
	  cfg.endless = false; // endless output is almost never desired here
        }
	else if(!strcmp(optarg,"sdl")) cfg.output = sdl;
	else {
	  message(MSG_ERROR, "unknown output method -- %s", optarg);
	  exit(EXIT_FAILURE);
	}
	break;
      case 'e':
	if(!strcmp(optarg, "satoh")) cfg.emutype = Emu_Satoh;
	else if(!strcmp(optarg, "ken")) cfg.emutype = Emu_Ken;
	else {
	  message(MSG_ERROR, "unknown emulator -- %s", optarg);
	  exit(EXIT_FAILURE);
	}
      case 'q': if(cfg.message_level) cfg.message_level--; break;
      case 'v': cfg.message_level++; break;
      }
  }

  return optind;
}

static void play(const char *fn, Player *pl, int subsong = -1)
/*
 * Start playback of subsong 'subsong' of file 'fn', using player
 * 'player'. If 'subsong' is not given or -1, start playback of
 * default subsong of file.
 */
{
  unsigned long i;

  // initialize output & player
  pl->get_opl()->init();
  pl->p = CAdPlug::factory(fn,pl->get_opl());

  if(!pl->p) {
    message(MSG_WARN, "unknown filetype -- %s", fn);
    return;
  }

  if(subsong != -1)
    pl->p->rewind(subsong);
#ifdef HAVE_ADPLUG_GETSUBSONG
  else
    subsong = pl->p->getsubsong();
#endif

  fprintf(stderr, "Playing '%s'...\n"
	  "Type  : %s\n"
	  "Title : %s\n"
	  "Author: %s\n\n", fn, pl->p->gettype().c_str(),
	  pl->p->gettitle().c_str(), pl->p->getauthor().c_str());

  if(cfg.showinsts) {		// display instruments
    fprintf(stderr, "Instrument names:\n");
    for(i = 0;i < pl->p->getinstruments(); i++)
      fprintf(stderr, "%2lu: %s\n", i, pl->p->getinstrument(i).c_str());
    fprintf(stderr, "\n");
  }

  if(cfg.songmessage)	// display song message
    fprintf(stderr, "Song message:\n%s\n\n", pl->p->getdesc().c_str());

  // play loop
  do {
    if(cfg.songinfo)	// display song info
      fprintf(stderr, "Subsong: %d/%d, Order: %d/%d, Pattern: %d/%d, Row: %d, "
	      "Speed: %d, Timer: %.2fHz     \r",
	      subsong, pl->p->getsubsongs()-1, pl->p->getorder(),
	      pl->p->getorders(), pl->p->getpattern(), pl->p->getpatterns(),
	      pl->p->getrow(), pl->p->getspeed(), pl->p->getrefresh());

    pl->frame();
  } while(pl->playing || cfg.endless);
}

static void shutdown(void)
/* General deinitialization handler. */
{
  if(player) delete player;
  if(opl) delete opl;
}

static void sighandler(int signal)
/* Handles all kinds of signals. */
{
  switch(signal) {
  case SIGINT:
  case SIGTERM:
    exit(EXIT_SUCCESS);
  }
}

/***** Main program *****/

int main(int argc, char **argv)
{
  int			optind, i;
  const char		*homedir;
  char			*userdb;

  // init
  program_name = argv[0];
  atexit(shutdown);
  signal(SIGINT, sighandler); signal(SIGTERM, sighandler);

  // Try user's home directory first, before trying the default location.
  homedir = getenv("HOME");
  if(homedir) {
    userdb = (char *)malloc(strlen(homedir) + strlen(ADPLUG_CONFDIR) +
			    strlen(ADPLUGDB_FILE) + 3);
    strcpy(userdb, homedir); strcat(userdb, "/" ADPLUG_CONFDIR "/");
    strcat(userdb, ADPLUGDB_FILE);
  }

  // parse commandline
  optind = decode_switches(argc,argv);
  if(optind == argc) {	// no filename given
    fprintf(stderr, "%s: need at least one file for playback\n", program_name);
    fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
    if(userdb) free(userdb);
    exit(EXIT_FAILURE);
  }
  if(argc - optind > 1) cfg.endless = false;	// more than 1 file given

  // init emulator
  opl = new CEmuopl(cfg.freq, cfg.bits == 16, cfg.channels == 2);

  // init player
  switch(cfg.output) {
  case none:
    message(MSG_PANIC, "no output methods compiled in");
    exit(EXIT_FAILURE);
  case disk:
    player = new DiskWriter(opl, cfg.device, cfg.bits, cfg.channels, cfg.freq);
    break;
  case sdl:
    player = new SDLPlayer(opl, cfg.bits, cfg.channels, cfg.freq, cfg.buf_size);
    break;
  default:
    message(MSG_ERROR, "output method not available");
    exit(EXIT_FAILURE);
  }

  // load database
  if(userdb) { mydb.load(userdb); free(userdb); }
  mydb.load(ADPLUGDB_PATH);
  CAdPlug::set_database(&mydb);

  // play all files from commandline
  for(i=optind;i<argc;i++)
    play(argv[i],player,cfg.subsong);

  // deinit
  exit(EXIT_SUCCESS);
}
