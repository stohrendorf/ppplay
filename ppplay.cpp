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

#include <boost/exception/all.hpp>

#include "config.h"

#include "src/ppg/sdlscreen.h"
#include "src/ui_main.h"

#include "src/s3mmod/s3mmodule.h"
#include "src/xmmod/xmmodule.h"
#include "src/modmod/modmodule.h"

#include "src/output/audiofifo.h"
#include "src/output/sdlaudiooutput.h"

#ifdef WITH_MP3LAME
#include "src/output/mp3audiooutput.h"
#endif

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "light4cxx/logger.h"

#include <SDL.h>

// static const size_t BUFFERSIZE = 4096;
// static const size_t SAMPLECOUNT = BUFFERSIZE / sizeof( BasicSample );
// static const size_t FRAMECOUNT = BUFFERSIZE / sizeof( BasicSampleFrame );

static std::shared_ptr<ppg::SDLScreen> dosScreen;
static std::shared_ptr<UIMain> uiMain;

static IAudioOutput::Ptr output;
static AudioFifo::Ptr fifo;
// static SDL_TimerID updateTimer = nullptr;

namespace config
{
static bool noGUI = false;
static uint16_t maxRepeat = 2;
static std::string filename;
#ifdef WITH_MP3LAME
static bool quickMp3 = false;
#endif
}

static bool parseCmdLine( int argc, char* argv[] )
{
	int loglevel = 0;
	namespace bpo = boost::program_options;
	bpo::options_description genOpts( "General Options" );
	genOpts.add_options()
	( "help,h", "Shows this help and exits" )
	( "version", "Shows version information and exits" )
	( "warranty", "Shows warranty information and exits" )
	( "copyright", "Shows copyright information and exits" )
	( "log-level,l", bpo::value<int>( &loglevel )->default_value( 1 ), "Sets the log level. Possible values:\n - 0 No logging\n - 1 Errors\n - 2 Warnings\n - 3 Informational\n - 4 Debug\n - 5 Trace\nWhen an invalid level is passed, it will automatically be set to 'Trace'. Levels 4 and 5 will also produce a more verbose output." )
	( "no-gui,n", "No GUI" )
	;
	bpo::options_description ioOpts( "Input/Output Options" );
	ioOpts.add_options()
	( "max-repeat,m", bpo::value<uint16_t>( &config::maxRepeat )->default_value( 2 ), "Maximum repeat count (the number of times an order can be played). Specify a number between 1 and 10,000." )
	( "file,f", bpo::value<std::string>( &config::filename ), "Module file to play" )
#ifdef WITH_MP3LAME
	( "quick-mp3,q", "Produces only an mp3 without sound output" )
#endif
	;
	bpo::positional_options_description p;
	p.add( "file", -1 );

	bpo::options_description allOpts( "All options" );
	allOpts.add( genOpts ).add( ioOpts );

	bpo::variables_map vm;
	bpo::store( bpo::command_line_parser( argc, argv ).options( allOpts ).positional( p ).run(), vm );
	bpo::notify( vm );

	if( config::maxRepeat < 1 || config::maxRepeat > 10000 ) {
		std::cout << "Error: Maximum repeat count not within 1 to 10,000" << std::endl;
		return false;
	}

	light4cxx::Location::setFormat( "[%T %<5t %>=7.3r] %L: %m%n" );
	switch( loglevel ) {
		case 0:
			light4cxx::Logger::setLevel( light4cxx::Level::Off );
			break;
		case 1:
			light4cxx::Logger::setLevel( light4cxx::Level::Error );
			break;
		case 2:
			light4cxx::Logger::setLevel( light4cxx::Level::Warn );
			break;
		case 3:
			light4cxx::Logger::setLevel( light4cxx::Level::Info );
			break;
		case 4:
			light4cxx::Logger::setLevel( light4cxx::Level::Debug );
// 			light4cxx::Location::setFormat("[%T %-5t %9p] %L (in %f:%l): %m%n");
			break;
		case 5:
			light4cxx::Logger::setLevel( light4cxx::Level::Trace );
// 			light4cxx::Location::setFormat("[%T %-5t %9p] %L (in %f:%l): %m%n");
			break;
		default:
			light4cxx::Logger::setLevel( light4cxx::Level::All );
// 			light4cxx::Location::setFormat("[%T %-5t %9p] %L (in %f:%l): %m%n");
	}

	using std::cout;
	using std::endl;
	if( vm.count( "warranty" ) ) {
		cout << "**** The following is part of the GNU General Public License version 3 ****" << endl;
		cout << "	  15. Disclaimer of Warranty." << endl;
		cout << endl;
		cout << "  THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY" << endl;
		cout << "APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT" << endl;
		cout << "HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY" << endl;
		cout << "OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO," << endl;
		cout << "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR" << endl;
		cout << "PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM" << endl;
		cout << "IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF" << endl;
		cout << "ALL NECESSARY SERVICING, REPAIR OR CORRECTION." << endl;
		cout << endl;
		cout << "  16. Limitation of Liability." << endl;
		cout << endl;
		cout << "  IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING" << endl;
		cout << "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS" << endl;
		cout << "THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY" << endl;
		cout << "GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE" << endl;
		cout << "USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF" << endl;
		cout << "DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD" << endl;
		cout << "PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS)," << endl;
		cout << "EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF" << endl;
		cout << "SUCH DAMAGES." << endl;

		return false;
	}
	if( vm.count( "copyright" ) ) {
		cout << "*   This program is free software; you can redistribute it and/or modify  *" << endl;
		cout << "*   it under the terms of the GNU General Public License as published by  *" << endl;
		cout << "*   the Free Software Foundation; either version 3 of the License, or     *" << endl;
		cout << "*   (at your option) any later version.                                   *" << endl;
		cout << "*                                                                         *" << endl;
		cout << "*   This program is distributed in the hope that it will be useful,       *" << endl;
		cout << "*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *" << endl;
		cout << "*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *" << endl;
		cout << "*   GNU General Public License for more details.                          *" << endl;
		cout << "*                                                                         *" << endl;
		cout << "*   You should have received a copy of the GNU General Public License     *" << endl;
		cout << "*   along with this program; if not, see <http://www.gnu.org/licenses/>.  *" << endl;
		return false;
	}
	if( vm.count( "version" ) ) {
		cout << PACKAGE_STRING << " - (C) 2010 " << PACKAGE_VENDOR << endl;
		return false;
	}
	if( vm.count( "help" ) || !vm.count( "file" ) ) {
		cout << "Usage: ppp [options] <file>" << endl;
		cout << PACKAGE_STRING << ", Copyright (C) 2010 by " << PACKAGE_VENDOR << endl;
		cout << PACKAGE_NAME << " comes with ABSOLUTELY NO WARRANTY; for details type `ppp --warranty'." << endl;
		cout << "This is free software, and you are welcome to redistribute it" << endl;
		cout << "under certain conditions; type `ppp --copyright' for details." << endl;
		cout << genOpts << ioOpts;
		return false;
	}
	if( vm.count( "no-gui" ) != 0 ) {
		config::noGUI = true;
	}
#ifdef WITH_MP3LAME
	config::quickMp3 = vm.count( "quick-mp3" );
#endif
	return vm.count( "file" ) != 0;
}

int main( int argc, char* argv[] )
{
	try {
		if( !parseCmdLine( argc, argv ) )
			return EXIT_SUCCESS;
		light4cxx::Logger::root()->info( L4CXX_LOCATION, boost::format( "Trying to load '%s'" ) % config::filename );
		ppp::GenModule::Ptr module;
		try {
			module = ppp::s3m::S3mModule::factory( config::filename, 44100, config::maxRepeat );
			if( !module ) {
				module = ppp::xm::XmModule::factory( config::filename, 44100, config::maxRepeat );
				if( !module ) {
					module = ppp::mod::ModModule::factory( config::filename, 44100, config::maxRepeat );
					if( !module ) {
						light4cxx::Logger::root()->error( L4CXX_LOCATION, "Error on loading the mod..." );
						return EXIT_FAILURE;
					}
				}
			}
		}
		catch( ... ) {
			light4cxx::Logger::root()->fatal( L4CXX_LOCATION, boost::format( "Main: %s" ) % boost::current_exception_diagnostic_information() );
			return EXIT_FAILURE;
		}
		if( !config::noGUI ) {
			light4cxx::Logger::root()->trace( L4CXX_LOCATION, "Initializing SDL Screen" );
			dosScreen.reset( new ppg::SDLScreen( 80, 25, PACKAGE_STRING ) );
			dosScreen->setAutoDelete( false );
			dosScreen->show();
		}
#ifdef WITH_MP3LAME
		if( !config::quickMp3 ) {
#endif
			light4cxx::Logger::root()->info( L4CXX_LOCATION, "Init Audio" );
			fifo.reset( new AudioFifo( module, 4096 ) );
			output.reset( new SDLAudioOutput( fifo ) );
			if( !output->init( 44100 ) ) {
				light4cxx::Logger::root()->fatal( L4CXX_LOCATION, "Audio Init failed" );
				return EXIT_FAILURE;
			}
			output->play();
			if( dosScreen ) {
				uiMain.reset( new UIMain( dosScreen.get(), module, output ) );
			}
			SDL_Event event;
			while( output ) {
				if( output && output->errorCode() == IAudioOutput::InputDry ) {
					light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Input is dry, trying to jump to the next song");
					if( !module->jumpNextSong() ) {
						light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Jump failed, quitting");
						output.reset();
						break;
					}
// 					output->init( module->frequency() );
// 					output->play();
				}
				else if( output && output->errorCode() != IAudioOutput::NoError ) {
					light4cxx::Logger::root()->debug(L4CXX_LOCATION, "Input has error, quitting");
					output.reset();
					break;
				}
				if( !SDL_PollEvent( &event ) ) {
					// usleep( 10000 );
					boost::this_thread::sleep( boost::posix_time::millisec( 10 ) );
					continue;
				}
				if( event.type == SDL_KEYDOWN ) {
					switch( event.key.keysym.sym ) {
						case SDLK_ESCAPE:
							output.reset();
							break;
						case SDLK_SPACE:
							if( output->playing() )
								output->pause();
							else if( output->paused() )
								output->play();
							break;
						case SDLK_END:
							if( !module->jumpNextSong() )
								output.reset();
							break;
						case SDLK_HOME:
							module->jumpPrevSong();
							break;
						case SDLK_PAGEDOWN:
							if( !module->jumpNextOrder() ) {
								if( !module->jumpNextSong() )
									output.reset();
							}
							break;
						case SDLK_PAGEUP:
							module->jumpPrevOrder();
							break;
						default:
							break;
					}
				}
				else if( !config::noGUI && event.type == SDL_MOUSEMOTION ) {
					dosScreen->onMouseMove( event.motion.x / 8, event.motion.y / 16 );
				}
				else if( event.type == SDL_QUIT ) {
					output.reset();
				}
			}
			if( output )
				output.reset();
#ifdef WITH_MP3LAME
		}
		else {   // if(mp3File.is_open()) { // quickMp3
			light4cxx::Logger::root()->info( L4CXX_LOCATION, "QuickMP3 Output Mode" );
			MP3AudioOutput* mp3out = new MP3AudioOutput( module, config::filename + ".mp3" );
			output.reset( mp3out );
			mp3out->setID3( module->trimmedTitle(), PACKAGE_STRING, module->trackerInfo() );
			if( 0 == mp3out->init( 44100 ) ) {
				if( mp3out->errorCode() == IAudioOutput::OutputUnavailable ) {
					light4cxx::Logger::root()->error( L4CXX_LOCATION, "LAME unavailable: Maybe cannot create MP3 File" );
				}
				else {
					light4cxx::Logger::root()->error( L4CXX_LOCATION, boost::format( "LAME initialization error: '%s'" ) % mp3out->errorCode() );
				}
				return EXIT_FAILURE;
			}
			if( dosScreen ) {
				uiMain.reset( new UIMain( dosScreen.get(), module, output ) );
			}
			output->play();
			while( output->playing() ) {
				// ...
			}
		}
#endif
	}
	catch( ... ) {
		light4cxx::Logger::root()->fatal( L4CXX_LOCATION, boost::format( "Main (end): %s" ) % boost::current_exception_diagnostic_information() );
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


