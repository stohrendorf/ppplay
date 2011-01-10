/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <SDL.h>

#include "config.h"

//! @todo Remove this when output code is stable/finished
#undef WITH_MP3LAME

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
#	include <lame/lame.h>
#endif

//using namespace std;

static const std::size_t BUFFERSIZE = 4096;
static const std::size_t SAMPLECOUNT = BUFFERSIZE / sizeof(ppp::BasicSample);
static const std::size_t FRAMECOUNT = BUFFERSIZE / sizeof(ppp::BasicSampleFrame);

#ifdef WITH_MP3LAME
static lame_global_flags *lgf;
static unsigned char mp3Buffer[BUFFERSIZE];
static std::ofstream mp3File;
#endif

static volatile bool playbackStopped = true;
static ppp::AudioFrameBuffer fftBuffer;

static std::shared_ptr<ppg::Screen> dosScreen;
static UIMain* uiMain;

#if 0
		ppg::Label *lb;
		ppp::GenModule *s3m = NULL;
		int nFrames = len_bytes / sizeof(ppp::BasicSampleFrame);
		s3m = reinterpret_cast<ppp::GenModule*>(userdata);
		if ((s3m == NULL) || playbackStopped) {
			return;
		}
		PPP_TEST(s3m == NULL);
		
		if(nFrames > s3m->playbackFifo.minFrameCount()) {
			LOG_MESSAGE("Adjusting FIFO buffer length from %d frames to %d frames", s3m->playbackFifo.minFrameCount(), nFrames);
			s3m->playbackFifo.setMinFrameCount(nFrames);
		}
		updateFrameCounter += nFrames;
		// TODO make this non-magic...
		if (updateFrameCounter >= (44100/50)) {
			updateFrameCounter = 0;
			dosScreen->clear(' ', ppg::dcWhite, ppg::dcBlack);
			{
				uiMain->volBar()->shift(s3m->playbackFifo.volumeLeft()>>8, s3m->playbackFifo.volumeRight()>>8);
			}
			int msecs = s3m->getPosition() / 441;
			int msecslen = s3m->getLength() / 441;
			ppp::GenPlaybackInfo pbi = s3m->getPlaybackInfo();
			lb = uiMain->posLabel();
			*lb = ppp::stringf("%3d(%3d)/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d \xf9 Track %d/%d",
								pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
								msecslen / 6000, msecslen / 100 % 60, msecslen % 100,
								s3m->getCurrentTrack() + 1, s3m->getTrackCount()
								);
			lb = uiMain->playbackInfo();
			*lb = ppp::stringf("Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", pbi.speed, pbi.tempo, pbi.globalVolume * 100 / 0x40);
			for (int i = 0; i < s3m->channelCount(); i++) {
				if(i>=16)
					break;
				lb = uiMain->chanInfo(i);
				*lb = s3m->getChanStatus(i);
				lb = uiMain->chanCell(i);
				*lb = s3m->getChanCellString(i);
			}
				dosScreen->draw();
		}

#else
static IAudioOutput::Ptr output;
static SDL_TimerID updateTimer = NULL;

static void updateDisplay(ppp::GenModule* module) {
	if(!module || playbackStopped)
		return;
	dosScreen->clear(' ', ppg::dcWhite, ppg::dcBlack);
	uiMain->volBar()->shift(output->volumeLeft()>>8, output->volumeRight()>>8);
	std::size_t msecs = module->getPosition() / 441;
	std::size_t msecslen = module->getLength() / 441;
	ppp::GenPlaybackInfo pbi = module->getPlaybackInfo();
	ppg::Label* lb = uiMain->posLabel();
	*lb = ppp::stringf("%3d(%3d)/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d \xf9 Track %d/%d",
						pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
						msecslen / 6000, msecslen / 100 % 60, msecslen % 100,
						module->getCurrentTrack() + 1, module->getTrackCount()
						);
	lb = uiMain->playbackInfo();
	*lb = ppp::stringf("Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", pbi.speed, pbi.tempo, pbi.globalVolume * 100 / 0x40);
	for (uint8_t i = 0; i < module->channelCount(); i++) {
		if(i>=16)
			break;
		lb = uiMain->chanInfo(i);
		*lb = module->getChanStatus(i);
		lb = uiMain->chanCell(i);
		*lb = module->getChanCellString(i);
	}
	//dosScreen->draw();
}

static Uint32 sdlTimerCallback(Uint32 interval, void* param) {
	updateDisplay(static_cast<ppp::GenModule*>(param));
	dosScreen->draw();
	return interval;
}
#endif

#ifdef WITH_MP3LAME
static bool quickMp3 = false;
#endif

static std::string parseCmdLine(int argc, char *argv[]) {
	namespace bpo = boost::program_options;
    bpo::options_description genOpts("General Options");
    genOpts.add_options()
            ("help,h","Shows this help and exits")
            ("version","Shows version information and exits")
            ("warranty","Shows warranty information and exits")
            ("copyright","Shows copyright information and exits")
            ("verbose,v","Be verbose (includes warnings)")
            ("very-verbose,V","FOR DEBUG PURPOSES ONLY! (implies -v, includes all messages)")
            ;
    bpo::options_description ioOpts("Input/Output Options");
    ioOpts.add_options()
            ("file,f",bpo::value<std::string>(),"Module file to play")
            #ifdef WITH_MP3LAME
            ("quick-mp3,q","Produces only an mp3 without sound output")
            #endif
            ;
    bpo::positional_options_description p;
    p.add("file",-1);

    bpo::options_description allOpts("All options");
    allOpts.add(genOpts).add(ioOpts);

    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(argc,argv).options(allOpts).positional(p).run(),vm);
    bpo::notify(vm);
	using std::cout;
	using std::endl;
    if(vm.count("warranty")) {
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
    if(vm.count("copyright")) {
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
    if(vm.count("version")) {
        cout << PACKAGE_STRING << " - (C) 2010 " << PACKAGE_VENDOR << endl;
        return std::string();
    }
    if(vm.count("help")||!vm.count("file")) {
        cout << "Usage: ppp [options] <file>" << endl;
		cout << PACKAGE_STRING << ", Copyright (C) 2010 by " << PACKAGE_VENDOR << endl;
        cout << PACKAGE_NAME << " comes with ABSOLUTELY NO WARRANTY; for details type `ppp --warranty'." << endl;
        cout << "This is free software, and you are welcome to redistribute it" << endl;
        cout << "under certain conditions; type `ppp --copyright' for details." << endl;
        cout << genOpts << ioOpts;
        return std::string();
    }
    if(vm.count("verbose")) {
        setLogLevel(llWarning);
    }
    if(vm.count("very-verbose")) {
        setLogLevel(llMessage);
    }
    //setLogLevel(llMessage);
	#ifdef WITH_MP3LAME
    quickMp3 = vm.count("quick-mp3");
	#endif
	switch(getLogLevel()) {
		case llMessage: cout << "Log level is: VERY Verbose" << endl; break;
		case llWarning: cout << "Log level is: Verbose" << endl; break;
		case llError: cout << "Log level is: Normal" << endl; break;
		case llNone: /* logging disabled... */ break;
		default: break;
	}
	return vm["file"].as<std::string>();
}

int main(int argc, char *argv[]) {
	try {
		std::string modFileName = parseCmdLine(argc,argv);
		if(modFileName.empty())
			return EXIT_SUCCESS;
		LOG_MESSAGE_("Initializing SDL");
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
			LOG_ERROR("Could not initialize SDL: %s", SDL_GetError());
			SDL_Quit();
			return EXIT_FAILURE;
		}
		LOG_MESSAGE_("Initializing PeePeeGUI Elements.");
		ppg::Label *l;
		dosScreen.reset(new ppg::Screen(80, 25, PACKAGE_STRING " - Built " __DATE__ " " __TIME__));
		uiMain = new UIMain(dosScreen.get());
		for (int i = 0; i < 16; i++) {
		}
		LOG_MESSAGE_("Loading the module.");
		ppp::GenModule::Ptr s3m;
		try {
			s3m.reset(new ppp::s3m::S3mModule(44100, 2));
			if (!s3m->load(modFileName)) {
				s3m.reset(new ppp::xm::XmModule(44100, 2));
				if (!s3m->load(modFileName)) {
					s3m.reset();
					LOG_ERROR_("Error on loading the mod...");
					SDL_Quit();
					return EXIT_FAILURE;
				}
			}
		}
		catch (PppException &e) {
			LOG_ERROR("Main: %s", e.what());
			return EXIT_FAILURE;
		}
		catch (ppg::Exception &e) {
			LOG_ERROR("Main: %s", e.what());
			return EXIT_FAILURE;
		}
		l = uiMain->trackerInfo();
		*l = ppp::stringf("Tracker: %s - Channels: %d", s3m->getTrackerInfo().c_str(), s3m->channelCount());
		if (s3m->isMultiTrack())
			*l += " - Multi-track";
		l = uiMain->modTitle();
		if (s3m->getTrimTitle() != "")
			*l = std::string(" -=\xf0[ ") + s3m->getFileName() + " : " + s3m->getTrimTitle() + " ]\xf0=- ";
		else
			*l = std::string(" -=\xf0[ ") + s3m->getFileName() + " ]\xf0=- ";
		dosScreen->show();
		//LOG_MESSAGE_("Init Fifo");
		//s3m->initFifo(ppp::FFT::fftSampleCount);
		fftBuffer.reset( new ppp::AudioFrameBuffer::element_type );
		fftBuffer->resize( ppp::FFT::fftSampleCount );
		playbackStopped = false;
		#ifdef WITH_MP3LAME
		if(!quickMp3) {
		#endif
			LOG_MESSAGE_("Init Audio");
			output.reset(new SDLAudioOutput(s3m.get()));
			if (!output->init(44100)) {
				LOG_ERROR_("Audio Init failed");
				return EXIT_FAILURE;
			}
			LOG_MESSAGE_("Default Output Mode");
			updateTimer = SDL_AddTimer(1000/30, sdlTimerCallback, s3m.get());
			output->play();
			SDL_Event event;
			while (!output->stopped() && !playbackStopped) {
				if (SDL_WaitEvent(&event)) {
					if(event.type == SDL_KEYDOWN) {
						switch(event.key.keysym.sym) {
							case SDLK_ESCAPE:
								playbackStopped = true;
								break;
							case SDLK_SPACE:
								if(output->playing())
									output->pause();
								else if(output->paused())
									output->play();
								break;
							case SDLK_END:
								if (!s3m->jumpNextTrack())
									playbackStopped = true;
								break;
							case SDLK_HOME:
								s3m->jumpPrevTrack();
								break;
							case SDLK_PAGEDOWN:
								if (!s3m->jumpNextOrder()) {
									// if jumpNextOrder fails, maybe jumpNextTrack works...
									if (!s3m->jumpNextTrack())
										playbackStopped = true;
								}
								break;
							case SDLK_PAGEUP:
								s3m->jumpPrevOrder();
								break;
							default:
								break;
						}
					}
					else if (event.type == SDL_MOUSEMOTION) {
						dosScreen->onMouseMove(event.motion.x/8, event.motion.y/16);
					}
					else if (event.type == SDL_QUIT) {
						playbackStopped = true;
					}
					else if (event.type == SDL_USEREVENT && event.user.code == 1) {
						dosScreen->clearOverlay();
						ppp::FFT::AmpsData FL, FR;
						ppp::FFT::doFFT(fftBuffer,FL,FR);
						uint16_t *pL = &FL->front();
						uint16_t *pR = &FR->front();
						int dlen = FL->size();
						for(int i=0; i<dlen; i++) {
							uint16_t h = (*(pL++))>>4;
							unsigned char color;
							if(h<10)
								color = ppg::dcGreen;
							else if(h<20)
								color = ppg::dcLightGreen;
							else if(h<40)
								color = ppg::dcYellow;
							else
								color = ppg::dcLightRed;
							for(int y=0; y<h; y++) {
								dosScreen->drawPixel(i*320/dlen,400-1-y,color);
							}
							h = (*(pR++))>>4;
							if(h<10)
								color = ppg::dcGreen;
							else if(h<20)
								color = ppg::dcLightGreen;
							else if(h<40)
								color = ppg::dcYellow;
							else
								color = ppg::dcLightRed;
							for(int y=0; y<h; y++)
								dosScreen->drawPixel(320+i*320/dlen,400-1-y,color);
						}
						ppp::BasicSample *smpPtr = &fftBuffer->front().left;
						dlen = fftBuffer->size()*2;
						for(int i=0; i<dlen; i++) {
							ppp::BasicSample y = *(smpPtr++) >> 10;
							if(i&1)
								dosScreen->drawPixel(320+i*320/dlen,400-64+y,ppg::dcLightBlue);
							else
								dosScreen->drawPixel(i*320/dlen,400-64+y,ppg::dcLightGreen);
						}
						dosScreen->draw();
					}
				}
			}
			output->pause();
			SDL_RemoveTimer(updateTimer);
			updateTimer = NULL;
		#ifdef WITH_MP3LAME
		}
		else {// if(mp3File.is_open()) { // quickMp3
			LOG_MESSAGE_("QuickMP3 Output Mode");
			LOG_MESSAGE_("Init LAME");
			lgf = lame_init();
			if (!lgf) {
				LOG_ERROR_("LAME Init Failed");
				// 			delete s3m;
				SDL_Quit();
				return EXIT_FAILURE;
			}
			LOG_MESSAGE_("Setting up LAME Parameters");
			lame_set_in_samplerate(lgf, 44100);
			lame_set_num_channels(lgf, 2);
			lame_set_quality(lgf, 5);
			lame_set_mode(lgf, STEREO);
			lame_set_VBR(lgf, vbr_off);
			id3tag_init(lgf);
			id3tag_add_v2(lgf);
			id3tag_set_title(lgf, s3m->getTrimTitle().c_str());
			id3tag_set_artist(lgf, s3m->getTrackerInfo().c_str());
			id3tag_set_album(lgf, PACKAGE_STRING);
			if (lame_init_params(lgf) < 0) {
				lame_close(lgf);
				LOG_ERROR_("LAME Init Step 2 Failed");
				SDL_Quit();
				return EXIT_FAILURE;
			}
			LOG_MESSAGE_("Opening MP3 file...");
			mp3File.open((modFileName + ".mp3").c_str(), std::ios::in);
			if (!mp3File.is_open())
				mp3File.open((modFileName + ".mp3").c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
			ppp::AudioFrameBuffer xBuffer;
			while(true) {
				if (!s3m->getFifo(xBuffer, FRAMECOUNT)) {
					if (!s3m->jumpNextTrack()) {
						playbackStopped = true;
					}
					else if (!s3m->getFifo(xBuffer,FRAMECOUNT)) {
						playbackStopped = true;
					}
				}
				if(playbackStopped)
					break;
				int res = lame_encode_buffer_interleaved(lgf, &xBuffer->front().left, FRAMECOUNT, mp3Buffer, BUFFERSIZE);
				if (res < 0) {
					if (res == -1)
						LOG_ERROR_("Lame Encoding Buffer too small!");
					else
						LOG_ERROR_("Unknown Lame Error.");
				}
				else {
					mp3File.write(reinterpret_cast<char*>(mp3Buffer), res);
				}
			}
			lame_close(lgf);
			mp3File.close();
		}
		#endif
		while(SDL_GetAudioStatus()==SDL_AUDIO_PLAYING)
			;
		SDL_Quit();
	}
	catch (PppException &e) {
		LOG_ERROR("Main (end): %s", e.what());
		return EXIT_FAILURE;
	}
	catch (ppg::Exception &e) {
		LOG_ERROR("Main (end): %s", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
