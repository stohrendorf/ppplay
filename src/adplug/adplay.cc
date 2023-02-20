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
#include <csignal>
#include <iostream>

#include <boost/program_options.hpp>

#include "adplug.h"

#include "defines.h"
#include "output.h"
#include "cli-players.h"
#include "light4cxx/logger.h"
#include "mid/multichips.h"

/***** Global variables *****/

namespace
{
light4cxx::Logger* logger = light4cxx::Logger::get( "badplay.main" );
}

static const char* program_name;
static std::unique_ptr<PlayerHandler> output; // global player object

/***** Configuration (and defaults) *****/

struct Configuration
{
  int buf_size;
  int subsong;
  std::string device;
  bool playOnce, showinsts, songinfo, songmessage;
  Outputs output;
  int repeats;
};

static Configuration cfg = {
  2048,
  -1,
  std::string(),
  true, false, false, false,
  DEFAULT_DRIVER,
  1
};

/***** Local functions *****/

static std::string decode_switches(int argc, const char** argv)
/*
 * Set all the option flags according to the switches specified.
 * Return the index of the first non-option argument.
 */
{
  boost::program_options::options_description options( "General Options" );
  options.add_options()
           ( "buffer,b", boost::program_options::value<int>( &cfg.buf_size ), "buffer size" )
           ( "device,d", boost::program_options::value<std::string>( &cfg.device ), "device file" )
           ( "instruments,i", boost::program_options::bool_switch( &cfg.showinsts ), "show instruments" )
           ( "realtime,r", boost::program_options::bool_switch( &cfg.songinfo ), "realtime song info" )
           ( "message,m", boost::program_options::bool_switch( &cfg.songmessage ), "song message" )
           ( "subsong,s", boost::program_options::value<int>( &cfg.subsong )->default_value( 0 ), "play subsong" )
           ( "once,o", boost::program_options::bool_switch( &cfg.playOnce ), "don't loop" )
           ( "help,h", boost::program_options::bool_switch(), "display help" )
           ( "version,V", boost::program_options::bool_switch(), "version information" )
           ( "output,O", boost::program_options::value<std::string>(), "output mechanism" )
           ( "quiet,q", boost::program_options::bool_switch(), "be more quiet" )
           ( "verbose,v", boost::program_options::bool_switch(), "be more verbose" )
           ( "repeats,R", boost::program_options::value<int>( &cfg.repeats ) )
           ( "melodic-bank",
             boost::program_options::value<std::string>()->default_value( "HMIGM" ),
             "Default melodic MIDI bank" )
           ( "percussion-bank",
             boost::program_options::value<std::string>()->default_value( "HMIGP" ),
             "Default percussion MIDI bank" )
           ( "list-banks", boost::program_options::bool_switch(), "List all MIDI banks" )
           ( "file,f", boost::program_options::value<std::string>(), "File to play" );

  boost::program_options::positional_options_description p;
  p.add( "file", 1 );

  boost::program_options::variables_map vm;
  try
  {
    boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( options )
                                                                                            .positional( p ).run(),
                                   vm );
    boost::program_options::notify( vm );
  }
  catch( std::exception& ex )
  {
    std::cerr << "Failed to parse command line: " << ex.what() << std::endl;
    std::cerr << options << std::endl;
    exit( EXIT_FAILURE );
  }

  if( vm["version"].as<bool>() )
  {
    std::cout << BADPLAY_VERSION << std::endl;
    exit( EXIT_SUCCESS );
  }

  if( vm["list-banks"].as<bool>() )
  {
    std::cout << "Known MIDI banks:\n";
    for( const auto& bank: ppp::MultiChips::bankDbInstance().banks() )
    {
      std::cout << bank.first << "\n";
      std::cout << "    " << bank.second.description << "\n";
      if( bank.second.onlyPercussion )
      {
        std::cout << "    ! Supports percussion instruments only.\n";
      }
      if( bank.second.uses4op )
      {
        std::cout << "    ! Contains 4-operator OPL3-only instruments.\n";
      }
    }
    exit( EXIT_SUCCESS );
  }

  if( vm["help"].as<bool>() || vm.count( "file" ) == 0 )
  {
    std::cout << program_name << " [options] <file>\n";
    std::cout << options;
    exit( EXIT_SUCCESS );
  }

  ppp::MultiChips::setDefaultMelodicBank( vm["melodic-bank"].as<std::string>() );
  ppp::MultiChips::setDefaultPercussionBank( vm["percussion-bank"].as<std::string>() );

  if( vm.count( "output" ) )
  {
    if( vm["output"].as<std::string>() == "disk" )
    {
      cfg.output = Outputs::disk;
      cfg.playOnce = true;
    }
    else if( vm["output"].as<std::string>() == "sdl" )
    {
      cfg.output = Outputs::sdl;
    }
    else
    {
      logger->fatal( L4CXX_LOCATION, "unknown output method -- %s", vm["output"].as<std::string>() );
      exit( EXIT_FAILURE );
    }
  }

  light4cxx::Logger::setLevel( light4cxx::Level::Info );

  if( vm["verbose"].as<bool>() )
  {
    light4cxx::Logger::setLevel( light4cxx::Level::Debug );
  }

  if( vm["quiet"].as<bool>() )
  {
    light4cxx::Logger::setLevel( light4cxx::Level::Warn );
  }

  if( vm.count( "file" ) == 0 )
  {
    logger->fatal( L4CXX_LOCATION, "No file specified" );
    exit( EXIT_FAILURE );
  }

  return vm["file"].as<std::string>();
}

static void play(const char* fn, PlayerHandler* output, const boost::optional<size_t>& subsong)
/*
 * Start playback of subsong 'subsong' of file 'fn', using player
 * 'player'. If 'subsong' is not given or -1, start playback of
 * default subsong of file.
 */
{
  // initialize output & player
  auto player = AdPlug::factory( fn );

  if( !player )
  {
    logger->warn( L4CXX_LOCATION, "unknown filetype -- %s", fn );
    return;
  }

  size_t ss = subsong.get_value_or( player->currentSubSong() );

  if( subsong.is_initialized() )
  {
    player->rewind( ss );
  }

  std::cerr << "Playing '" << fn << "'...\n"
            << "Type  : " << player->type() << "\n"
            << "Title : " << player->title() << "\n"
            << "Author: " << player->author() << "\n\n";

  if( cfg.showinsts )
  { // display instruments
    std::cerr << "Instrument names:\n";
    for( size_t i = 0; i < player->instrumentCount(); i++ )
    {
      std::cerr << i << ": " << player->instrumentTitle( i ) << "\n";
    }
    std::cerr << "\n";
  }

  if( cfg.songmessage )
  { // display song message
    std::cerr << "Song message:\n" << player->description() << "\n\n";
  }

  output->setPlayer( player );

  // play loop
  do
  {
    if( cfg.songinfo )
    { // display song info
      std::cerr << "Subsong: " << ss + 1 << "/" << player->subSongCount() + 0 << ", Order: "
                << player->currentOrder() + 0 << "/" << player->orderCount() + 0 << ", Pattern: "
                << player->currentPattern() + 0 << ", Row: " << player->currentRow() + 0 << ", Speed: "
                << player->currentSpeed() + 0 << ", Timer: "
                << std::fixed << Player::SampleRate / float( player->framesUntilUpdate() ) << "Hz     \r";
    }

    output->frame();
  } while( output->isPlaying() || !cfg.playOnce );
}

static void sighandler(int signal)
/* Handles all kinds of signals. */
{
  switch( signal )
  {
  case SIGINT:
  case SIGTERM:
    exit( EXIT_SUCCESS );
  }
}

/***** Main program *****/

int main(int argc, const char** argv)
{
  // init
  program_name = argv[0];
  signal( SIGINT, sighandler );
  signal( SIGTERM, sighandler );

  // parse commandline
  auto fn = decode_switches( argc, argv );

  // init player
  switch( cfg.output )
  {
  case Outputs::none:
    logger->fatal( L4CXX_LOCATION, "no output methods compiled in" );
    exit( EXIT_FAILURE );
  case Outputs::disk:
    output = std::make_unique<DiskWriter>( cfg.device.c_str(), 44100 );
    break;
  case Outputs::sdl:
    output = std::make_unique<SDLPlayer>( 44100, cfg.buf_size );
    break;
  default:
    logger->error( L4CXX_LOCATION, "output method not available" );
    return EXIT_FAILURE;
  }

  // play all files from commandline
  for( int r = 0; r < cfg.repeats; ++r )
  {
    play( fn.c_str(), output.get(), cfg.subsong );
  }

  return EXIT_FAILURE;
}
