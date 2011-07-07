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

#include "src/ppg/screen.h"
#include "src/ui_main.h"

#include "src/s3mmod/s3mmodule.h"
#include "src/xmmod/xmmodule.h"

#include "src/output/audiofifo.h"
#include "src/output/sdlaudiooutput.h"

#include "src/logger/logger.h"

#ifdef WITH_MP3LAME
#include "src/output/mp3audiooutput.h"
#endif

#include <iostream>
#include <algorithm>
#include <boost/program_options.hpp>
#include <SDL.h>

static const std::size_t BUFFERSIZE = 4096;
static const std::size_t SAMPLECOUNT = BUFFERSIZE / sizeof( BasicSample );
static const std::size_t FRAMECOUNT = BUFFERSIZE / sizeof( BasicSampleFrame );

static std::shared_ptr<ppg::Screen> dosScreen;
static std::shared_ptr<UIMain> uiMain;

static IAudioOutput::Ptr output;
static SDL_TimerID updateTimer = NULL;

static bool noGUI = false;

static void updateDisplay( ppp::GenModule::Ptr& module ) {
	if( !module || !output || noGUI )
		return;
	dosScreen->clear( ' ', ppg::Color::White, ppg::Color::Black );
	uiMain->volBar()->shift( output->volumeLeft() >> 8, output->volumeRight() >> 8 );
	std::size_t msecs = module->position() / 441;
	std::size_t msecslen = module->length() / 441;
	ppp::GenPlaybackInfo pbi = module->playbackInfo();
	ppg::Label* lb = uiMain->posLabel();
	if(module->isMultiSong()) {
		lb->setEscapedText( ppp::stringf( "{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d \xf9 Song %d/%d",
								pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
								msecslen / 6000, msecslen / 100 % 60, msecslen % 100,
								module->currentSongIndex() + 1, module->songCount()
								) );
	}
	else {
		lb->setEscapedText( ppp::stringf( "{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d",
								pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
								msecslen / 6000, msecslen / 100 % 60, msecslen % 100
								) );
	}
	lb = uiMain->playbackInfo();
	lb->setEscapedText( ppp::stringf( "{BrightWhite;}Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", pbi.speed, pbi.tempo, pbi.globalVolume * 100 / 0x40 ) );
	for( uint8_t i = 0; i < module->channelCount(); i++ ) {
		if( i >= 16 )
			break;
		lb = uiMain->chanInfo( i );
		lb->setText( module->channelStatus( i ) );
		lb = uiMain->chanCell( i );
		lb->setText( module->channelCellString( i ) );
	}
}

static Uint32 sdlTimerCallback( Uint32 interval, void* param ) {
	if( !noGUI ) {
		updateDisplay( *static_cast<ppp::GenModule::Ptr*>( param ) );
		dosScreen->draw();
	}
	return interval;
}

#ifdef WITH_MP3LAME
static bool quickMp3 = false;
#endif

static std::string parseCmdLine( int argc, char* argv[] ) {
	namespace bpo = boost::program_options;
	bpo::options_description genOpts( "General Options" );
	genOpts.add_options()
	( "help,h", "Shows this help and exits" )
	( "version", "Shows version information and exits" )
	( "warranty", "Shows warranty information and exits" )
	( "copyright", "Shows copyright information and exits" )
	( "verbose,v", "Be verbose (includes warnings)" )
	( "very-verbose,V", "FOR DEBUG PURPOSES ONLY! (implies -v, includes all messages)" )
	( "no-gui,n", "No GUI" )
	;
	bpo::options_description ioOpts( "Input/Output Options" );
	ioOpts.add_options()
	( "file,f", bpo::value<std::string>(), "Module file to play" )
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
	using std::cout;
	using std::endl;
	if( vm.count( "warranty" ) ) {
		cout << "**** The following is part of the GNU General Public License version 2 ****" << endl;
		cout << "  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY" << endl;
		cout << "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN" << endl;
		cout << "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES" << endl;
		cout << "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED" << endl;
		cout << "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF" << endl;
		cout << "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS" << endl;
		cout << "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE" << endl;
		cout << "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING," << endl;
		cout << "REPAIR OR CORRECTION." << endl;
		cout << endl;
		cout << "  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING" << endl;
		cout << "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR" << endl;
		cout << "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES," << endl;
		cout << "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING" << endl;
		cout << "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED" << endl;
		cout << "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY" << endl;
		cout << "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER" << endl;
		cout << "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE" << endl;
		cout << "POSSIBILITY OF SUCH DAMAGES." << endl;
		return std::string();
	}
	if( vm.count( "copyright" ) ) {
		cout << "*   This program is free software; you can redistribute it and/or modify  *" << endl;
		cout << "*   it under the terms of the GNU General Public License as published by  *" << endl;
		cout << "*   the Free Software Foundation; either version 2 of the License, or     *" << endl;
		cout << "*   (at your option) any later version.                                   *" << endl;
		cout << "*                                                                         *" << endl;
		cout << "*   This program is distributed in the hope that it will be useful,       *" << endl;
		cout << "*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *" << endl;
		cout << "*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *" << endl;
		cout << "*   GNU General Public License for more details.                          *" << endl;
		cout << "*                                                                         *" << endl;
		cout << "*   You should have received a copy of the GNU General Public License     *" << endl;
		cout << "*   along with this program; if not, write to the                         *" << endl;
		cout << "*   Free Software Foundation, Inc.,                                       *" << endl;
		cout << "*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *" << endl;
		return std::string();
	}
	if( vm.count( "version" ) ) {
		cout << PACKAGE_STRING << " - (C) 2010 " << PACKAGE_VENDOR << endl;
		return std::string();
	}
	if( vm.count( "help" ) || !vm.count( "file" ) ) {
		cout << "Usage: ppp [options] <file>" << endl;
		cout << PACKAGE_STRING << ", Copyright (C) 2010 by " << PACKAGE_VENDOR << endl;
		cout << PACKAGE_NAME << " comes with ABSOLUTELY NO WARRANTY; for details type `ppp --warranty'." << endl;
		cout << "This is free software, and you are welcome to redistribute it" << endl;
		cout << "under certain conditions; type `ppp --copyright' for details." << endl;
		cout << genOpts << ioOpts;
		return std::string();
	}
	if( vm.count( "no-gui" ) ) {
		noGUI = true;
	}
	if( vm.count( "verbose" ) ) {
		setLogLevel( llWarning );
	}
	if( vm.count( "very-verbose" ) ) {
		setLogLevel( llMessage );
	}
	//setLogLevel(llMessage);
#ifdef WITH_MP3LAME
	quickMp3 = vm.count( "quick-mp3" );
#endif
	switch( getLogLevel() ) {
		case llMessage:
			cout << "Log level is: VERY Verbose" << endl;
			break;
		case llWarning:
			cout << "Log level is: Verbose" << endl;
			break;
		case llError:
			cout << "Log level is: Normal" << endl;
			break;
		case llNone: /* logging disabled... */
			break;
		default:
			break;
	}
	return vm["file"].as<std::string>();
}

int main( int argc, char* argv[] ) {
	try {
		std::string modFileName = parseCmdLine( argc, argv );
		if( modFileName.empty() )
			return EXIT_SUCCESS;
		LOG_MESSAGE( "Initializing SDL" );
		if( SDL_Init( /* SDL_INIT_VIDEO | SDL_INIT_AUDIO | */ SDL_INIT_TIMER ) < 0 ) {
			LOG_ERROR( "Could not initialize SDL: %s", SDL_GetError() );
			SDL_Quit();
			return EXIT_FAILURE;
		}
		LOG_MESSAGE( "Initializing PeePeeGUI Elements." );
		ppg::Label* l;
		if( !noGUI ) {
			dosScreen.reset( new ppg::Screen( 80, 25, PACKAGE_STRING ) );
			dosScreen->setAutoDelete(false);
			uiMain.reset( new UIMain( dosScreen.get() ) );
		}
		LOG_MESSAGE( "Loading the module." );
		ppp::GenModule::Ptr module;
		try {
			module = ppp::s3m::S3mModule::factory( modFileName, 44100, 2 );
			if( !module ) {
				module = ppp::xm::XmModule::factory( modFileName, 44100, 2 );
				if( !module ) {
					LOG_ERROR( "Error on loading the mod..." );
					SDL_Quit();
					return EXIT_FAILURE;
				}
			}
		}
		catch( ... ) {
			LOG_ERROR( "Main: %s", boost::current_exception_diagnostic_information().c_str() );
			return EXIT_FAILURE;
		}
		if( !noGUI ) {
			l = uiMain->trackerInfo();
			l->setText( ppp::stringf( "Tracker: %s - Channels: %d", module->trackerInfo().c_str(), module->channelCount() ) );
			if( module->isMultiSong() )
				l->setText( l->text() + " - Multi-song" );
			l = uiMain->modTitle();
			if( module->trimmedTitle().length()>0 )
				l->setText( std::string( " -=\xf0[ " ) + module->filename() + " : " + module->trimmedTitle() + " ]\xf0=- " );
			else
				l->setText( std::string( " -=\xf0[ " ) + module->filename() + " ]\xf0=- " );
			dosScreen->show();
		}
		//LOG_MESSAGE_("Init Fifo");
		//s3m->initFifo(ppp::FFT::fftSampleCount);
		updateTimer = SDL_AddTimer( 1000 / 30, sdlTimerCallback, &module );
#ifdef WITH_MP3LAME
		if( !quickMp3 ) {
#endif
			LOG_MESSAGE( "Init Audio" );
			output.reset( new SDLAudioOutput( module ) );
			if( !output->init( 44100 ) ) {
				LOG_ERROR( "Audio Init failed" );
				return EXIT_FAILURE;
			}
			LOG_MESSAGE( "Default Output Mode" );
			output->play();
			SDL_Event event;
			while(output) {
				if( output && output->errorCode() == IAudioOutput::InputDry ) {
					if( !module->jumpNextSong() ) {
						output.reset();
						break;
					}
					output->init( module->frequency() );
					output->play();
				}
				else if(output && output->errorCode() != IAudioOutput::NoError) {
					output.reset();
					break;
				}
				if( !SDL_PollEvent( &event ) ) {
					usleep( 10000 );
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
				else if( !noGUI && event.type == SDL_MOUSEMOTION ) {
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
			LOG_MESSAGE( "QuickMP3 Output Mode" );
			MP3AudioOutput* mp3out = new MP3AudioOutput( module, modFileName + ".mp3" );
			output.reset( mp3out );
			mp3out->setID3( module->trimmedTitle(), PACKAGE_STRING, module->trackerInfo() );
			if( 0 == mp3out->init( 44100 ) ) {
				if( mp3out->errorCode() == IAudioOutput::OutputUnavailable )
					LOG_ERROR( "LAME unavailable: Maybe cannot create MP3 File" );
				else
					LOG_ERROR( "LAME initialization error: %d", mp3out->errorCode() );
				SDL_Quit();
				return EXIT_FAILURE;
			}
			output->play();
			while( output->playing() ) {
				// ...
			}
		}
#endif
		SDL_RemoveTimer( updateTimer );
		updateTimer = NULL;
		SDL_Quit();
	}
	catch( ... ) {
		LOG_ERROR( "Main (end): %s", boost::current_exception_diagnostic_information().c_str() );
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

