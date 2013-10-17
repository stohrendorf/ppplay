/*
    PPPlay - an old-fashioned module player
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

#include "src/output/sdlaudiooutput.h"
#include "src/output/wavaudiooutput.h"

#ifdef WITH_MP3LAME
#include "src/output/mp3audiooutput.h"
#endif

#ifdef WITH_OGG
#include "src/output/oggaudiooutput.h"
#endif

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/progress.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>

#include "light4cxx/logger.h"

#include "src/stuff/pluginregistry.h"

#include <SDL.h>

namespace
{
std::unique_ptr<ppg::SDLScreen> dosScreen;
UIMain* uiMain;

AbstractAudioOutput::Ptr output;

namespace config
{
bool noGUI = false;
uint16_t maxRepeat = 2;
std::string filename;
std::string outputFilename;
ppp::Sample::Interpolation interpolation = ppp::Sample::Interpolation::Cubic;
int loglevel = 1;
}

void loadUserConfig()
{
    const char* env = getenv( "HOME" );
#ifdef WIN32
    if( !env )
        env = getenv( "APPDATA" );
#endif
    if( !env )
        return;
    std::string cfgFilename = env;
    if( cfgFilename.empty() ) {
        return;
    }
    cfgFilename += "/.ppplayrc";
    boost::property_tree::ptree pt;
    try {
        boost::property_tree::read_ini( cfgFilename, pt );
    }
    catch( boost::property_tree::ini_parser_error& ex ) {
        // Populate the defaults
        pt.put( "config.no_gui", false );
        pt.put( "playback.max_repeat", 2 );
        pt.put( "debug.log_level", 1 );
        pt.put( "playback.interpolation", 2 );
    }
    config::noGUI = pt.get<bool>( "config.no_gui", false );
    config::maxRepeat = pt.get<uint16_t>( "playback.max_repeat", 2 );
    config::loglevel = pt.get<int>( "debug.log_level", 1 );
    config::interpolation = ppp::Sample::Interpolation( pt.get<int>( "playback.interpolation", 2 ) );
    boost::property_tree::write_ini( cfgFilename, pt );
}

bool parseCmdLine( int argc, char* argv[] )
{
    loadUserConfig();
    boost::program_options::options_description genOpts( "General Options" );
    genOpts.add_options()
    ( "help,h", "Shows this help and exits" )
    ( "version", "Shows version information and exits" )
    ( "warranty", "Shows warranty information and exits" )
    ( "copyright", "Shows copyright information and exits" )
    ( "log-level,l", boost::program_options::value<int>( &config::loglevel )->default_value( config::loglevel ), "Sets the log level. Possible values:\n - 0 No logging\n - 1 Errors\n - 2 Warnings\n - 3 Informational\n - 4 Debug\n - 5 Trace\nWhen an invalid level is passed, it will automatically be set to 'Trace'. Levels 4 and 5 will also produce a more verbose output." )
    ( "no-gui,n", "No GUI" )
    ;
    boost::program_options::options_description ioOpts( "Input/Output Options" );
    ioOpts.add_options()
    ( "max-repeat,m", boost::program_options::value<uint16_t>( &config::maxRepeat )->default_value( config::maxRepeat ), "Maximum repeat count (the number of times an order can be played). Specify a number between 1 and 10,000." )
    ( "file,f", boost::program_options::value<std::string>( &config::filename ), "Module file to play" )
    ( "output,o", boost::program_options::value<std::string>( &config::outputFilename )->default_value( std::string() ), "Set mp3/wav filename" )
    ( "interpolation,i", boost::program_options::value<int>()->default_value( int( config::interpolation ) ), "Set interpolation mode:\n - 0 No interpolation\n - 1 Linear interpolation\n - 2 Cubic interpolation" )
    ;
    boost::program_options::positional_options_description p;
    p.add( "file", -1 );

    boost::program_options::options_description allOpts( "All options" );
    allOpts.add( genOpts ).add( ioOpts );

    boost::program_options::variables_map vm;
    boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( allOpts ).positional( p ).run(), vm );
    boost::program_options::notify( vm );

    if( config::maxRepeat < 1 || config::maxRepeat > 10000 ) {
        std::cout << "Error: Maximum repeat count not within 1 to 10,000" << std::endl;
        return false;
    }

    light4cxx::Location::setFormat( "[%T %<5t %>=7.3r] <%L> %m" );
    switch( config::loglevel ) {
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
            break;
        case 5:
            light4cxx::Logger::setLevel( light4cxx::Level::Trace );
            break;
        default:
            light4cxx::Logger::setLevel( light4cxx::Level::All );
    }

    char selfPath[1024];
#ifdef WIN32
    auto selfPathLen = GetModuleFileNameA(nullptr, selfPath, sizeof(selfPath));
    if(selfPathLen != sizeof(selfPath)) {
        ppp::PluginRegistry::instance().setSearchPath( boost::filesystem::path(selfPath).parent_path() / ".." / LIBEXECDIR );
    }
#else
    // this is not NUL-terminated.
    auto selfPathLen = readlink("/proc/self/exe", selfPath, sizeof(selfPath)-1);
    if(selfPathLen != -1) {
        selfPath[selfPathLen] = '\0';
        ppp::PluginRegistry::instance().setSearchPath( boost::filesystem::path(selfPath).parent_path() / ".." / LIBEXECDIR );
    }
#endif

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
        cout << "Usage: ppplay [options] <file>" << endl;
        cout << PACKAGE_STRING << ", Copyright (C) 2010-2013 by " << PACKAGE_VENDOR << endl;
        cout << PACKAGE_NAME << " comes with ABSOLUTELY NO WARRANTY; for details type `ppp --warranty'." << endl;
        cout << "This is free software, and you are welcome to redistribute it" << endl;
        cout << "under certain conditions; type `ppp --copyright' for details." << endl;
        cout << genOpts << ioOpts;
        return false;
    }
    if( vm.count( "no-gui" ) != 0 ) {
        config::noGUI = true;
    }
    switch( vm["interpolation"].as<int>() ) {
        case 0:
            light4cxx::Logger::root()->info( L4CXX_LOCATION, "Sample interpolation: none" );
            config::interpolation = ppp::Sample::Interpolation::None;
            break;
        case 1:
            light4cxx::Logger::root()->info( L4CXX_LOCATION, "Sample interpolation: linear" );
            config::interpolation = ppp::Sample::Interpolation::Linear;
            break;
        case 2:
            light4cxx::Logger::root()->info( L4CXX_LOCATION, "Sample interpolation: cubic" );
            config::interpolation = ppp::Sample::Interpolation::Cubic;
            break;
        default:
            light4cxx::Logger::root()->warn( L4CXX_LOCATION, "Invalid interpolation mode requested" );
    }
    return vm.count( "file" ) != 0;
}

void terminateHandler()
{
    std::cerr << "Unexpected exception: " << boost::current_exception_diagnostic_information() << std::endl;
    std::cerr << "The message above means that PPPlay encountered a problem which was so\n"
              << "unexpected that, despite all taken care, caused it to die a sudden death.\n"
              << "To help PPPlay learn to handle this problem, please submit a bug report to:\n"
              << "    http://sourceforge.net/p/peepeeplayer/tickets/\n"
              << "If possible, please add the file you tried to play and the message above,\n"
              << "so the authors can examine and hopefully solve the mystery. Thank you!"
              << std::endl;
    std::abort();
}

} // anonymous namespace

int main( int argc, char* argv[] )
{
    std::set_terminate(&terminateHandler);
    try {
        if( !parseCmdLine( argc, argv ) ) {
            return EXIT_SUCCESS;
        }
        SDL_Init( SDL_INIT_EVERYTHING );
        light4cxx::Logger::root()->info( L4CXX_LOCATION, "Trying to load '%s'", config::filename );
        ppp::AbstractModule::Ptr module;
        try {
            module = ppp::PluginRegistry::tryLoad( config::filename, 44100, config::maxRepeat, config::interpolation );
            if( !module ) {
                light4cxx::Logger::root()->error( L4CXX_LOCATION, "Failed to load '%s'", config::filename );
                return EXIT_FAILURE;
            }
        }
        catch( ... ) {
            light4cxx::Logger::root()->fatal( L4CXX_LOCATION, "Exception on module loading: %s", boost::current_exception_diagnostic_information() );
            std::cout << std::flush;
            std::cerr << "The message above means that PPPlay encountered a severe problem it\n"
                      << "could not handle gracefully. Please report this problem to the developers at:\n"
                      << "    http://sourceforge.net/p/peepeeplayer/tickets/\n"
                      << "If possible, please add the file you tried to play, as this seems to be the\n"
                      << "root of all evil, as well as the message above. Thank you!"
                      << std::endl;
            return EXIT_FAILURE;
        }
        if( !config::noGUI ) {
            light4cxx::Logger::root()->debug( L4CXX_LOCATION, "Initializing SDL Screen: %s", PACKAGE_STRING );
            dosScreen.reset( new ppg::SDLScreen( 80, 25, PACKAGE_STRING ) );
            dosScreen->setAutoDelete( false );
            dosScreen->show();
        }
        if( config::outputFilename.empty() ) {
            light4cxx::Logger::root()->info( L4CXX_LOCATION, "Init Audio" );
            output.reset( new SDLAudioOutput( module ) );
            if( !output->init( 44100 ) ) {
                light4cxx::Logger::root()->fatal( L4CXX_LOCATION, "Audio Init failed" );
                return EXIT_FAILURE;
            }
            output->play();
            if( dosScreen ) {
                uiMain = new UIMain( dosScreen.get(), module, output );
            }
            SDL_Event event;
            while( output ) {
                if( output->errorCode() == AbstractAudioOutput::InputDry ) {
                    light4cxx::Logger::root()->debug( L4CXX_LOCATION, "Input is dry, trying to jump to the next song" );
                    module->setPaused( true );
                    output->pause();
                    if( !module->jumpNextSong() ) {
                        light4cxx::Logger::root()->debug( L4CXX_LOCATION, "Jump failed, quitting" );
                        output.reset();
                        break;
                    }
                    module->setPaused( false );
                    output->play();
                }
                else if( output->errorCode() != AbstractAudioOutput::NoError ) {
                    light4cxx::Logger::root()->debug( L4CXX_LOCATION, "Input has error, quitting" );
                    output.reset();
                    break;
                }
                if( output && uiMain ) {
                    uiMain->setFft( reinterpret_cast<SDLAudioOutput*>( output.get() )->leftFft(), reinterpret_cast<SDLAudioOutput*>( output.get() )->rightFft() );
                }
                if( !SDL_PollEvent( &event ) ) {
                    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
                    continue;
                }
                if( event.type == SDL_KEYDOWN ) {
                    switch( event.key.keysym.sym ) {
                        case SDLK_ESCAPE:
                            output.reset();
                            break;
                        case SDLK_SPACE:
                            if( output->playing() ) {
                                output->pause();
                            }
                            else if( output->paused() ) {
                                output->play();
                            }
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
        }
        else if( boost::iends_with( config::outputFilename, ".wav" ) ) {
            light4cxx::Logger::root()->info( L4CXX_LOCATION, "QuickWAV Output Mode" );
            if( config::outputFilename.empty() ) {
                config::outputFilename = config::filename + ".wav";
            }
            WavAudioOutput* wavout = new WavAudioOutput( module, config::outputFilename );
            output.reset( wavout );
            if( 0 == wavout->init( 44100 ) ) {
                if( wavout->errorCode() == AbstractAudioOutput::OutputUnavailable ) {
                    light4cxx::Logger::root()->error( L4CXX_LOCATION, "Maybe cannot create WAV File" );
                }
                else {
                    light4cxx::Logger::root()->error( L4CXX_LOCATION, "WAV initialization error: '%d'", wavout->errorCode() );
                }
                return EXIT_FAILURE;
            }
            if( dosScreen ) {
                uiMain = new UIMain( dosScreen.get(), module, output );
            }
            output->play();
            int secs = module->length() / module->frequency();
            boost::progress_display progress( module->length(), std::cout, stringFmt( "QuickWAV: %s (%dm%02ds)\n", config::filename, secs / 60, secs % 60 ) );
            while( output->playing() ) {
                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
                progress += std::const_pointer_cast<const ppp::AbstractModule>( module )->state().playedFrames - progress.count();
            }
            output.reset();
        }
#ifdef WITH_MP3LAME
        else if( boost::iends_with( config::outputFilename, ".mp3" ) ) {
            light4cxx::Logger::root()->info( L4CXX_LOCATION, "QuickMP3 Output Mode" );
            if( config::outputFilename.empty() ) {
                config::outputFilename = config::filename + ".mp3";
            }
            MP3AudioOutput* mp3out = new MP3AudioOutput( module, config::outputFilename );
            output.reset( mp3out );
            mp3out->setID3( boost::trim_copy( module->metaInfo().title ), PACKAGE_STRING, std::const_pointer_cast<const ppp::AbstractModule>( module )->metaInfo().trackerInfo );
            if( 0 == mp3out->init( 44100 ) ) {
                if( mp3out->errorCode() == AbstractAudioOutput::OutputUnavailable ) {
                    light4cxx::Logger::root()->error( L4CXX_LOCATION, "LAME unavailable: Maybe cannot create MP3 File" );
                }
                else {
                    light4cxx::Logger::root()->error( L4CXX_LOCATION, "LAME initialization error: '%d'", mp3out->errorCode() );
                }
                return EXIT_FAILURE;
            }
            if( dosScreen ) {
                uiMain = new UIMain( dosScreen.get(), module, output );
            }
            output->play();
            int secs = module->length() / module->frequency();
            boost::progress_display progress( module->length(), std::cout, stringFmt( "QuickMP3: %s (%dm%02ds)\n", config::filename, secs / 60, secs % 60 ) );
            while( output->playing() ) {
                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
                progress += std::const_pointer_cast<const ppp::AbstractModule>( module )->state().playedFrames - progress.count();
            }
            output.reset();
        }
#endif
#ifdef WITH_OGG
        else if( boost::iends_with( config::outputFilename, ".ogg" ) ) {
            light4cxx::Logger::root()->info( L4CXX_LOCATION, "QuickOGG Output Mode" );
            if( config::outputFilename.empty() ) {
                config::outputFilename = config::filename + ".ogg";
            }
            OggAudioOutput* oggOut = new OggAudioOutput( module, config::outputFilename );
            output.reset( oggOut );
            oggOut->setMeta( boost::trim_copy( module->metaInfo().title ), PACKAGE_STRING, std::const_pointer_cast<const ppp::AbstractModule>( module )->metaInfo().trackerInfo );
            if( 0 == oggOut->init( 44100 ) ) {
                if( oggOut->errorCode() == AbstractAudioOutput::OutputUnavailable ) {
                    light4cxx::Logger::root()->error( L4CXX_LOCATION, "OGG unavailable: Maybe cannot create OGG File" );
                }
                else {
                    light4cxx::Logger::root()->error( L4CXX_LOCATION, "OGG initialization error: '%d'", oggOut->errorCode() );
                }
                return EXIT_FAILURE;
            }
            if( dosScreen ) {
                uiMain = new UIMain( dosScreen.get(), module, output );
            }
            output->play();
            int secs = module->length() / module->frequency();
            boost::progress_display progress( module->length(), std::cout, stringFmt( "QuickOGG: %s (%dm%02ds)\n", config::filename, secs / 60, secs % 60 ) );
            while( output->playing() ) {
                std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
                progress += std::const_pointer_cast<const ppp::AbstractModule>( module )->state().playedFrames - progress.count();
            }
            output.reset();
        }
#endif
    }
    catch( ... ) {
        light4cxx::Logger::root()->fatal( L4CXX_LOCATION, stringFmt( "Main (end): %s", boost::current_exception_diagnostic_information() ) );
        std::cout << std::flush;
        std::cerr << "The message above means that PPPlay encountered a problem which\n"
                  << "was so terrible that, despite all taken care, caused it to die.\n"
                  << "To help PPPlay learn to handle this problem, please submit\n"
                  << "a bug report to:\n"
                  << "    http://sourceforge.net/p/peepeeplayer/tickets/\n"
                  << "If possible, please add the file you tried to play and the message above,\n"
                  << "so the authors can examine and hopefully solve the mystery. Thank you!"
                  << std::endl;
        return EXIT_FAILURE;
    }
    dosScreen.reset();
    return EXIT_SUCCESS;
}
