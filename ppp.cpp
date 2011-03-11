/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

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

#include <SDL.h>

#include "config.h"

#include <iostream>
#include <algorithm>
#include <boost/program_options.hpp>
#include "src/ppg/ppg.h"
#include "src/ppg/screen.h"
#include "src/ui_main.h"

#include "src/genmod/genmodule.h"
#include "src/s3mmod/s3mmodule.h"
#include "src/xmmod/xmmodule.h"

#include "src/stuff/fft.h"

#include "src/output/audiofifo.h"
#include "src/output/sdlaudiooutput.h"

#ifdef WITH_MP3LAME
#include "src/output/mp3audiooutput.h"
#endif

static const std::size_t BUFFERSIZE = 4096;
static const std::size_t SAMPLECOUNT = BUFFERSIZE / sizeof( BasicSample );
static const std::size_t FRAMECOUNT = BUFFERSIZE / sizeof( BasicSampleFrame );

static AudioFrameBuffer fftBuffer;

static std::shared_ptr<ppg::Screen> dosScreen;
static UIMain* uiMain = NULL;

static IAudioOutput::Ptr output;
static SDL_TimerID updateTimer = NULL;

static bool noGUI = false;

static void updateDisplay( ppp::GenModule::Ptr& module ) {
	if( !module || !output || noGUI )
		return;
	//dosScreen->clear( ' ', ppg::dcWhite, ppg::dcBlack );
	uiMain->volBar()->shift( output->volumeLeft() >> 8, output->volumeRight() >> 8 );
	std::size_t msecs = module->getPosition() / 441;
	std::size_t msecslen = module->getLength() / 441;
	ppp::GenPlaybackInfo pbi = module->getPlaybackInfo();
	ppg::Label* lb = uiMain->posLabel();
	lb->setText( ppp::stringf( "%3d(%3d)/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d \xf9 Track %d/%d",
	                           pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
	                           msecslen / 6000, msecslen / 100 % 60, msecslen % 100,
	                           module->getCurrentTrack() + 1, module->getTrackCount()
	                         ) );
	lb = uiMain->playbackInfo();
	lb->setText( ppp::stringf( "Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", pbi.speed, pbi.tempo, pbi.globalVolume * 100 / 0x40 ) );
	for( uint8_t i = 0; i < module->channelCount(); i++ ) {
		if( i >= 16 )
			break;
		lb = uiMain->chanInfo( i );
		lb->setText( module->getChanStatus( i ) );
		lb = uiMain->chanCell( i );
		lb->setText( module->getChanCellString( i ) );
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
		if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) < 0 ) {
			LOG_ERROR( "Could not initialize SDL: %s", SDL_GetError() );
			SDL_Quit();
			return EXIT_FAILURE;
		}
		LOG_MESSAGE( "Initializing PeePeeGUI Elements." );
		ppg::Label* l;
		if( !noGUI ) {
			dosScreen.reset( new ppg::Screen( 80, 25, PACKAGE_STRING ) );
			uiMain = new UIMain( dosScreen.get() );
		}
		for( int i = 0; i < 16; i++ ) {
		}
		LOG_MESSAGE( "Loading the module." );
		ppp::GenModule::Ptr s3m;
		try {
			s3m.reset( new ppp::s3m::S3mModule( 44100, 2 ) );
			if( !std::static_pointer_cast<ppp::s3m::S3mModule>( s3m )->load( modFileName ) ) {
				s3m.reset( new ppp::xm::XmModule( 44100, 2 ) );
				if( !std::static_pointer_cast<ppp::xm::XmModule>( s3m )->load( modFileName ) ) {
					s3m.reset();
					LOG_ERROR( "Error on loading the mod..." );
					SDL_Quit();
					return EXIT_FAILURE;
				}
			}
		}
		catch( PppException& e ) {
			LOG_ERROR( "Main: %s", e.what() );
			return EXIT_FAILURE;
		}
		catch( ppg::Exception& e ) {
			LOG_ERROR( "Main: %s", e.what() );
			return EXIT_FAILURE;
		}
		if( !noGUI ) {
			l = uiMain->trackerInfo();
			l->setText( ppp::stringf( "Tracker: %s - Channels: %d", s3m->getTrackerInfo().c_str(), s3m->channelCount() ) );
			if( s3m->isMultiTrack() )
				*l += " - Multi-track";
			l = uiMain->modTitle();
			if( s3m->getTrimTitle() != "" )
				l->setText( std::string( " -=\xf0[ " ) + s3m->getFileName() + " : " + s3m->getTrimTitle() + " ]\xf0=- " );
			else
				l->setText( std::string( " -=\xf0[ " ) + s3m->getFileName() + " ]\xf0=- " );
			dosScreen->show();
		}
		//LOG_MESSAGE_("Init Fifo");
		//s3m->initFifo(ppp::FFT::fftSampleCount);
		fftBuffer.reset( new AudioFrameBuffer::element_type );
		fftBuffer->resize( ppp::FFT::fftSampleCount );
		updateTimer = SDL_AddTimer( 1000 / 30, sdlTimerCallback, &s3m );
#ifdef WITH_MP3LAME
		if( !quickMp3 ) {
#endif
			LOG_MESSAGE( "Init Audio" );
			output.reset( new SDLAudioOutput( s3m.get() ) );
			if( !output->init( 44100 ) ) {
				LOG_ERROR( "Audio Init failed" );
				return EXIT_FAILURE;
			}
			LOG_MESSAGE( "Default Output Mode" );
			output->play();
			SDL_Event event;
			while( output && output->errorCode() == IAudioOutput::NoError ) {
				usleep( 1000 );
				if( SDL_PollEvent( &event ) ) {
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
								if( !s3m->jumpNextTrack() )
									output.reset();
								break;
							case SDLK_HOME:
								s3m->jumpPrevTrack();
								break;
							case SDLK_PAGEDOWN:
								if( !s3m->jumpNextOrder() ) {
									// if jumpNextOrder fails, maybe jumpNextTrack works...
									if( !s3m->jumpNextTrack() )
										output.reset();
								}
								break;
							case SDLK_PAGEUP:
								s3m->jumpPrevOrder();
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
					else if( !noGUI && event.type == SDL_USEREVENT && event.user.code == 1 ) {
						dosScreen->clearOverlay();
						ppp::FFT::AmpsData FL, FR;
						ppp::FFT::doFFT( fftBuffer, FL, FR );
						uint16_t* pL = &FL->front();
						uint16_t* pR = &FR->front();
						int dlen = FL->size();
						for( int i = 0; i < dlen; i++ ) {
							uint16_t h = ( *( pL++ ) ) >> 4;
							unsigned char color;
							if( h < 10 )
								color = ppg::dcGreen;
							else if( h < 20 )
								color = ppg::dcLightGreen;
							else if( h < 40 )
								color = ppg::dcYellow;
							else
								color = ppg::dcLightRed;
							for( int y = 0; y < h; y++ ) {
								dosScreen->drawPixel( i * 320 / dlen, 400 - 1 - y, color );
							}
							h = ( *( pR++ ) ) >> 4;
							if( h < 10 )
								color = ppg::dcGreen;
							else if( h < 20 )
								color = ppg::dcLightGreen;
							else if( h < 40 )
								color = ppg::dcYellow;
							else
								color = ppg::dcLightRed;
							for( int y = 0; y < h; y++ )
								dosScreen->drawPixel( 320 + i * 320 / dlen, 400 - 1 - y, color );
						}
						BasicSample* smpPtr = &fftBuffer->front().left;
						dlen = fftBuffer->size() * 2;
						for( int i = 0; i < dlen; i++ ) {
							BasicSample y = *( smpPtr++ ) >> 10;
							if( i & 1 )
								dosScreen->drawPixel( 320 + i * 320 / dlen, 400 - 64 + y, ppg::dcLightBlue );
							else
								dosScreen->drawPixel( i * 320 / dlen, 400 - 64 + y, ppg::dcLightGreen );
						}
						dosScreen->draw();
					}
				}
			}
			if( output )
				output.reset();
#ifdef WITH_MP3LAME
		}
		else {   // if(mp3File.is_open()) { // quickMp3
			LOG_MESSAGE( "QuickMP3 Output Mode" );
			MP3AudioOutput* mp3out = new MP3AudioOutput( s3m.get(), modFileName + ".mp3" );
			output.reset( mp3out );
			mp3out->setID3( s3m->getTrimTitle(), PACKAGE_STRING, s3m->getTrackerInfo() );
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
	catch( PppException& e ) {
		LOG_ERROR( "Main (end): %s", e.what() );
		return EXIT_FAILURE;
	}
	catch( ppg::Exception& e ) {
		LOG_ERROR( "Main (end): %s", e.what() );
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

