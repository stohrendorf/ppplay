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

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <csignal>
#include "adplug.h"

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

/***** Global variables *****/

static const char	*program_name;
static std::unique_ptr<Player> output; // global player object

/***** Configuration (and defaults) *****/

static struct {
    int buf_size, message_level;
    int subsong;
    const char *device;
    const char *userdb;
    bool endless, showinsts, songinfo, songmessage;
    Outputs output;
    int repeats;
} cfg = {
    2048, MSG_NOTE,
    -1,
    nullptr,
    nullptr,
    true, false, false, false,
    DEFAULT_DRIVER,
    1
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
    std::cout <<
                 "Usage: " << program_name << " [OPTION]... FILE...\n\n"
                 "Output selection:\n"
                 "  -O, --output=OUTPUT        specify output mechanism (disk/sdl)\n\n"
                 "Disk writer (disk) specific:\n"
                 "  -d, --device=FILE          output to FILE ('-' is stdout)\n\n"
                 "SDL driver (sdl) specific:\n"
                 "  -b, --buffer=SIZE          set output buffer size to SIZE\n\n"
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
                 "  -V, --version              output version information and exit\n\n";
}

static int decode_switches(int argc, char **argv)
/*
 * Set all the option flags according to the switches specified.
 * Return the index of the first non-option argument.
 */
{
    int c;
    struct option const long_options[] = {
    {"buffer", required_argument, NULL, 'b'},	// buffer size
    {"device", required_argument, NULL, 'd'},	// device file
    {"instruments", no_argument, NULL, 'i'},	// show instruments
    {"realtime", no_argument, NULL, 'r'},	// realtime song info
    {"message", no_argument, NULL, 'm'},	// song message
    {"subsong", no_argument, NULL, 's'},	// play subsong
    {"once", no_argument, NULL, 'o'},		// don't loop
    {"help", no_argument, NULL, 'h'},		// display help
    {"version", no_argument, NULL, 'V'},	// version information
    {"output", required_argument, NULL, 'O'},	// output mechanism
    {"quiet", no_argument, NULL, 'q'},		// be more quiet
    {"verbose", no_argument, NULL, 'v'},	// be more verbose
    {"repeats", required_argument, NULL, 'R'},
    {NULL, 0, NULL, 0}				// end of options
};

    while ((c = getopt_long(argc, argv, "8f:b:d:irms:ohVe:O:D:qvR:", long_options, nullptr)) != EOF) {
        switch (c) {
        case 'b': cfg.buf_size = atoi(optarg); break;
        case 'd': cfg.device = optarg; break;
        case 'i': cfg.showinsts = true; break;
        case 'r': cfg.songinfo = true; break;
        case 'm': cfg.songmessage = true; break;
        case 's': cfg.subsong = atoi(optarg); break;
        case 'o': cfg.endless = false; break;
        case 'V': puts(BADPLAY_VERSION); exit(EXIT_SUCCESS);
        case 'h':	usage(); exit(EXIT_SUCCESS); break;
        case 'R': cfg.repeats = atoi(optarg); break;
        case 'O':
            if(!strcmp(optarg,"disk")) {
                cfg.output = Outputs::disk;
                cfg.endless = false; // endless output is almost never desired here
            }
            else if(!strcmp(optarg,"sdl"))
                cfg.output = Outputs::sdl;
            else {
                message(MSG_ERROR, "unknown output method -- %s", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 'q': if(cfg.message_level) cfg.message_level--; break;
        case 'v': cfg.message_level++; break;
        }
    }

    return optind;
}

static void play(const char *fn, Player *output, int subsong = -1)
/*
 * Start playback of subsong 'subsong' of file 'fn', using player
 * 'player'. If 'subsong' is not given or -1, start playback of
 * default subsong of file.
 */
{
    // initialize output & player
    auto player = CAdPlug::factory(fn);

    if(!player) {
        message(MSG_WARN, "unknown filetype -- %s", fn);
        return;
    }

    if(subsong != -1)
        player->rewind(subsong);
    else
        subsong = player->currentSubSong();

    std::cerr << "Playing '" << fn << "'...\n"
              << "Type  : " << player->type() << "\n"
              << "Title : " << player->title() << "\n"
              << "Author: " << player->author() << "\n\n";

    if(cfg.showinsts) {		// display instruments
        std::cerr << "Instrument names:\n";
        for(size_t i = 0; i < player->instrumentCount(); i++)
            std::cerr << i << ": " << player->instrumentTitle(i) << "\n";
        std::cerr << "\n";
    }

    if(cfg.songmessage)	// display song message
        std::cerr << "Song message:\n" << player->description() << "\n\n";

    output->setPlayer( player );

    // play loop
    do {
        if(cfg.songinfo)	// display song info
            std::cerr << "Subsong: " << subsong+1 << "/" << player->subSongCount()+0 << ", Order: "
                      << player->currentOrder()+0 << "/" << player->orderCount()+0 << ", Pattern: "
                      << player->currentPattern()+0 << ", Row: " << player->currentRow()+0 << ", Speed: "
                      << player->currentSpeed()+0 << ", Timer: "
                      << std::fixed << CPlayer::SampleRate/float(player->framesUntilUpdate()) << "Hz     \r";

        output->frame();
    } while(output->isPlaying() || cfg.endless);
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
    // init
    program_name = argv[0];
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    // parse commandline
    int optind = decode_switches(argc,argv);
    if(optind == argc) {	// no filename given
        std::cerr << program_name << ": need at least one file for playback\n";
        std::cerr << "Try '" << program_name << " --help' for more information.\n";
        return EXIT_FAILURE;
    }
    if(argc - optind > 1) cfg.endless = false;	// more than 1 file given

    // init player
    switch(cfg.output) {
    case Outputs::none:
        message(MSG_PANIC, "no output methods compiled in");
        exit(EXIT_FAILURE);
    case Outputs::disk:
        output.reset( new DiskWriter(cfg.device, 44100) );
        break;
    case Outputs::sdl:
        output.reset( new SDLPlayer(44100, cfg.buf_size) );
        break;
    default:
        message(MSG_ERROR, "output method not available");
        return EXIT_FAILURE;
    }

    // play all files from commandline
    for(int i=optind;i<argc;i++) {
        for(int r=0; r<cfg.repeats; ++r) {
            play(argv[i],output.get(),cfg.subsong);
        }
    }

    return EXIT_FAILURE;
}
