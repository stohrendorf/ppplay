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


#include "config.h"

#include <iostream>
#include <algorithm>
#include <boost/program_options.hpp>
#include "ppg.h"

// #include "fft.h"
#include "genmodule.h"
#include "s3mmodule.h"
//#include "stmmodule.h"

#include "fft.h"

#include "audiofifo.h"

#ifdef WITH_MP3LAME
#	include <lame/lame.h>
#endif

using namespace std;

static const char SvnRevision[] = "$Revision: 70 $";

static const std::size_t BUFFERSIZE = 4096;
static const std::size_t SAMPLECOUNT = BUFFERSIZE/2;
static const std::size_t FRAMECOUNT = SAMPLECOUNT/2;

#ifdef WITH_MP3LAME
static lame_global_flags *lgf;
static unsigned char mp3Buffer[BUFFERSIZE];
static ofstream mp3File;
#endif

static volatile bool playbackStopped = true;
static std::shared_ptr< std::vector<unsigned short> > FL, FR;

static std::shared_ptr<PpgScreen> dosScreen;
static std::size_t updateFrameCounter = 0;

static void my_audio_callback(void *userdata, Uint8 *stream, int len_bytes) {
	try {
		LOG_BEGIN();
		PpgLabel *lb;
		ppp::GenModule *s3m = NULL;
		std::size_t nFrames = len_bytes / sizeof(ppp::BasicSampleFrame);
		s3m = reinterpret_cast<ppp::GenModule*>(userdata);
		if ((s3m == NULL) || playbackStopped) {
			return;
		}
		PPP_TEST(stream == NULL);
		PPP_TEST(len_bytes == 0);
		PPP_TEST(s3m == NULL);
		std::fill_n(stream, len_bytes, 0);
		ppp::AudioFrameBuffer frameBuffer;
		if (!s3m->getFifo(frameBuffer, nFrames)) {
			LOG_MESSAGE("getFifo failed");
			if (!s3m->jumpNextTrack()) {
				SDL_Event x;
				x.type = SDL_KEYDOWN;
				x.key.keysym.sym = SDLK_ESCAPE;
				SDL_PushEvent(&x);
				playbackStopped = true;
				LOG_MESSAGE("jumpNextTrack failed");
				return;
			}
			else if (!s3m->getFifo(frameBuffer, nFrames)) {
				SDL_Event x;
				x.type = SDL_KEYDOWN;
				x.key.keysym.sym = SDLK_ESCAPE;
				SDL_PushEvent(&x);
				playbackStopped = true;
				LOG_MESSAGE("getFifo failed");
				return;
			}
		}
		updateFrameCounter += nFrames;
		if (updateFrameCounter >= FRAMECOUNT) {
			updateFrameCounter = 0;
			//LOG_DEBUG("doing FFT");
			//ppp::FFT::doFFT(xBuffer,FL,FR);
			//LOG_DEBUG("FFT done.");
			#ifdef WITH_MP3LAME
			if (mp3File.is_open()) {
				int res = lame_encode_buffer_interleaved(lgf, &frameBuffer->front().left, nFrames*2, mp3Buffer, BUFFERSIZE);
				if (res < 0) {
					if (res == -1)
						LOG_ERROR("Lame Encoding Buffer too small!");
					else
						LOG_ERROR("Unknown Lame Error.");
				}
				else {
					mp3File.write(reinterpret_cast<char*>(mp3Buffer), res);
				}
			}
			#endif
			dosScreen->clear(' ', dcWhite, dcBlack);
			{
				PpgStereoPeakBar *p = dosScreen->getByPath<PpgStereoPeakBar>("VolBar");
				PPG_TEST(!p);
				p->shift(s3m->playbackFifo.getVolumeLeft()>>8, s3m->playbackFifo.getVolumeRight()>>8);
			}
			int msecs = s3m->getPosition() / 441;
			int msecslen = s3m->getLength() / 441;
			ppp::GenPlaybackInfo pbi = s3m->getPlaybackInfo();
			lb = dosScreen->getByPath<PpgLabel>("Position");
			PPG_TEST(!lb);
			*lb = ppp::stringf("%3d(%3d)/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d \xf9 Track %d/%d",
								pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
								msecslen / 6000, msecslen / 100 % 60, msecslen % 100,
								s3m->getCurrentTrack() + 1, s3m->getTrackCount()
								);
			lb = dosScreen->getByPath<PpgLabel>("PlaybackInfo");
			PPG_TEST(!lb);
			*lb = ppp::stringf("Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", pbi.speed, pbi.tempo, pbi.globalVolume * 100 / 0x40);
			for (int i = 0; i < 16; i++) {
				if (i >= s3m->physChannels())
					break;
				lb = dosScreen->getByPath<PpgLabel>(ppp::stringf("ChanInfo_%d", i));
				PPG_TEST(!lb);
				*lb = s3m->getChanStatus(i);
				lb = dosScreen->getByPath<PpgLabel>(ppp::stringf("ChanCell_%d", i));
				PPG_TEST(!lb);
				*lb = s3m->getChanCellString(i);
			}
// 				dosScreen->draw();
			// $$$$$
			{
				static SDL_Event x;
				x.type = SDL_USEREVENT;
				x.user.code = 1;
				SDL_PushEvent(&x);
			}
		}
		if(len_bytes != nFrames*sizeof(ppp::BasicSampleFrame))
			LOG_WARNING("len_bytes != nFrames*sizeof(ppp::BasicSampleFrame)");
		std::copy(frameBuffer->begin(), frameBuffer->begin()+nFrames, reinterpret_cast<ppp::BasicSampleFrame*>(stream));
		//memcpy(stream, &frameBuffer->front().left, nFrames*sizeof(ppp::BasicSampleFrame));
	}
	catch (PppException &e) {
		LOG_ERROR(string("Audio Callback: ") + e.what());
		SDL_Event x;
		x.type = SDL_KEYDOWN;
		x.key.keysym.sym = SDLK_ESCAPE;
		SDL_PushEvent(&x);
		playbackStopped = true;
	}
	catch (PpgException &e) {
		LOG_ERROR(string("Audio Callback: ") + e.what());
		SDL_Event x;
		x.type = SDL_KEYDOWN;
		x.key.keysym.sym = SDLK_ESCAPE;
		SDL_PushEvent(&x);
		playbackStopped = true;
	}
}

static bool initAudio(void *userData) {
	SDL_AudioSpec *desired = new SDL_AudioSpec;
	SDL_AudioSpec *obtained = new SDL_AudioSpec;
	desired->freq = 44100;
	desired->channels = 2;
	desired->format = AUDIO_S16LSB;
	desired->samples = SAMPLECOUNT;
	desired->callback = my_audio_callback;
	desired->userdata = userData;
	if (SDL_OpenAudio(desired, obtained) < 0) {
		LOG_ERROR(string("Couldn't open audio: ") + SDL_GetError());
		return false;
	}
	delete desired;
	return true;
}

#ifdef WITH_MP3LAME
static bool quickMp3 = false;
#endif

namespace bpo = boost::program_options;
using namespace bpo;
static string parseCmdLine(int argc, char *argv[]) {
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
            ("file,f",bpo::value<string>(),"Module file to play")
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
        return string();
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
        return string();
    }
    if(vm.count("version")) {
        cout << PACKAGE_STRING << " - (C) 2010 " << PACKAGE_VENDOR << endl;
        return string();
    }
    if(vm.count("help")||!vm.count("file")) {
        cout << "Usage: ppp [options] <file>" << endl;
		cout << PACKAGE_STRING << ", Copyright (C) 2010 by " << PACKAGE_VENDOR << endl;
        cout << PACKAGE_NAME << " comes with ABSOLUTELY NO WARRANTY; for details type `ppp --warranty'." << endl;
        cout << "This is free software, and you are welcome to redistribute it" << endl;
        cout << "under certain conditions; type `ppp --copyright' for details." << endl;
        cout << genOpts << ioOpts;
        return string();
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
    return vm["file"].as<string>();
}

int main(int argc, char *argv[]) {
	try {
		LOG_BEGIN();
		string modFileName = parseCmdLine(argc,argv);
		if(modFileName.empty())
			return EXIT_SUCCESS;
		LOG_MESSAGE("Initializing SDL");
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
			LOG_ERROR(string("Could not initialize SDL: ") + SDL_GetError());
			SDL_Quit();
			return EXIT_FAILURE;
		}
		LOG_MESSAGE("Initializing PeePeeGUI Elements.");
		PpgLabel *l;
		dosScreen.reset(new PpgScreen(80, 25, PACKAGE_STRING " - Built " __DATE__ " " __TIME__));
		l = new PpgLabel("Position", "");
		l->setWidth(dosScreen->getWidth() - 4);
		l->setPosition(2, 2);
		l->setFgColor(0, dcBrightWhite, 0);
		l->setFgColor(3, dcWhite, 5);
		l->show();
		dosScreen->addChild(*l);
		l = new PpgLabel("ScreenSeparator", " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 ");
		l->setPosition(0, 1);
		l->setFgColor(0, dcBrightWhite, 0);
		l->show();
		dosScreen->addChild(*l);
		l = new PpgLabel("ScreenSeparator2", " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 ");
		l->setPosition(0, 3);
		l->setFgColor(0, dcBrightWhite, 0);
		l->show();
		dosScreen->addChild(*l);
		l = new PpgLabel("PlaybackInfo", "");
		l->setWidth(dosScreen->getWidth() - 4);
		l->setPosition(2, 2);
		l->alignment = PpgLabel::Alignment::alRight;
		l->setFgColor(0, dcBrightWhite, 0);
		l->show();
		dosScreen->addChild(*l);
		PpgStereoPeakBar *p = new PpgStereoPeakBar("VolBar", 16, 256, 1, true);
		p->setPosition((dosScreen->getWidth() - p->length()) / 2, 4);
		p->show();
		dosScreen->addChild(*p);
		for (int i = 0; i < 16; i++) {
			l = new PpgLabel(ppp::stringf("ChanInfo_%d", i), "");
			l->setWidth(dosScreen->getWidth() - 4);
			l->setPosition(2, 5 + i);
			l->setFgColor(4, dcLightRed);
			l->setFgColor(5, dcBrightWhite, 3);
			l->setFgColor(9, dcLightBlue);
			l->setFgColor(10, dcAqua, 2);
			l->setFgColor(13, dcLightGreen, 6);
			l->setFgColor(35, dcBrightWhite, 0);
			l->show();
			dosScreen->addChild(*l);
			l = new PpgLabel(ppp::stringf("ChanCell_%d", i), "");
			l->setWidth(dosScreen->getWidth() - 4);
			l->setPosition(2, 5 + i);
			l->alignment = PpgLabel::Alignment::alRight;
			l->setFgColor(0, dcWhite, 0);
			l->show();
			dosScreen->addChild(*l);
		}
		LOG_MESSAGE("Loading the module.");
		ppp::GenModule::Ptr s3m;
		try {
			s3m.reset(new ppp::s3m::S3mModule(44100, 2));
			if (!s3m->load(modFileName)) {
/*				s3m.reset(new ppp::stm::StmModule(44100, 2));
				if (!s3m->load(modFileName)) {*/
					s3m.reset();
					LOG_ERROR("Error on loading the mod...");
					SDL_Quit();
					return EXIT_FAILURE;
/*				}*/
			}
		}
		catch (PppException &e) {
			LOG_ERROR(string("Main: ") + e.what());
			return EXIT_FAILURE;
		}
		catch (PpgException &e) {
			LOG_ERROR(string("Main: ") + e.what());
			return EXIT_FAILURE;
		}
		l = new PpgLabel("TrackerInfo", ppp::stringf("Tracker: %s - Channels: %d", s3m->getTrackerInfo().c_str(), s3m->physChannels()));
		if (s3m->isMultiTrack())
			*l += " - Multi-track";
		l->setPosition(2, dosScreen->getBottom() - 1);
		l->setWidth(dosScreen->getWidth() - 4);
		l->alignment = PpgLabel::Alignment::alCenter;
		l->setFgColor(0, dcAqua, 0);
		l->show();
		dosScreen->addChild(*l);
		l = new PpgLabel("ModTitle", "");
		if (s3m->getTrimTitle() != "")
			*l = string(" -=\xf0[ ") + s3m->getFileName() + " : " + s3m->getTrimTitle() + " ]\xf0=- ";
		else
			*l = string(" -=\xf0[ ") + s3m->getFileName() + " ]\xf0=- ";
		l->setFgColor(0, dcBrightWhite, 0);
		l->setWidth(dosScreen->getWidth() - 4);
		l->setPosition(2, 0);
		l->alignment = PpgLabel::Alignment::alCenter;
		l->show();
		dosScreen->addChild(*l);
		dosScreen->show();
		LOG_MESSAGE("Init Fifo");
		s3m->initFifo(FRAMECOUNT);
		playbackStopped = false;
		LOG_MESSAGE("Init Audio");
		if (!initAudio(s3m.get())) {
			LOG_ERROR("Audio Init failed");
			SDL_Quit();
		}
		#ifdef WITH_MP3LAME
// 		quickMp3 = false;
		if(!quickMp3) {
		#endif
			LOG_MESSAGE("Default Output Mode");
			SDL_PauseAudio(0);
			SDL_Event event;
			while (true) {
				if (SDL_WaitEvent(&event)) {
					if(event.type == SDL_KEYDOWN) {
						switch(event.key.keysym.sym) {
							case SDLK_ESCAPE:
								playbackStopped = true;
								break;
							case SDLK_SPACE:
								if (SDL_GetAudioStatus() != SDL_AUDIO_PLAYING)
									SDL_PauseAudio(0);
								else
									SDL_PauseAudio(1);
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
								// TODO: jumpPreviousOrder;
								break;
							default:
								break;
						}
					}
					else if (event.type == SDL_QUIT) {
						playbackStopped = true;
					}
					else if (event.type == SDL_USEREVENT && event.user.code == 1) {
/*						dosScreen->clearOverlay();
						unsigned short dlen = ppp::FFT::fftLength;
						for(unsigned short i=0; i<dlen; i++) {
							unsigned short h = FL->at(i)>>4;
							unsigned char color;
							if(h<10)
								color = dcGreen;
							else if(h<20)
								color = dcLightGreen;
							else if(h<40)
								color = dcYellow;
							else
								color = dcLightRed;
							for(unsigned short y=0; y<h; y++) {
								dosScreen->drawPixel(i*320/dlen,400-1-y,color);
							}
							h = FR->at(i)>>4;
							if(h<10)
								color = dcGreen;
							else if(h<20)
								color = dcLightGreen;
							else if(h<40)
								color = dcYellow;
							else
								color = dcLightRed;
							for(unsigned short y=0; y<h; y++)
								dosScreen->drawPixel(320+i*320/dlen,400-1-y,color);
						}*/
/*						ppp::BasicSample *smpPtr = &xBuffer->front().left;
						for(unsigned short i=0; i<SAMPLECOUNT; i++) {
							ppp::BasicSample y = *(smpPtr++) >> 10;
							if(i&1)
								dosScreen->drawPixel(320+i*320/SAMPLECOUNT,400-64+y,dcLightBlue);
							else
								dosScreen->drawPixel(i*320/SAMPLECOUNT,400-64+y,dcLightGreen);
						}*/
						dosScreen->draw();
					}
				}
				if (playbackStopped)
					break;
			}
			SDL_PauseAudio(1);
		#ifdef WITH_MP3LAME
		}
		else {// if(mp3File.is_open()) { // quickMp3
			LOG_MESSAGE("QuickMP3 Output Mode");
			LOG_MESSAGE("Init LAME");
			lgf = lame_init();
			if (!lgf) {
				LOG_ERROR("LAME Init Failed");
				// 			delete s3m;
				SDL_Quit();
				return EXIT_FAILURE;
			}
			LOG_MESSAGE("Setting up LAME Parameters");
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
				LOG_ERROR("LAME Init Step 2 Failed");
				// 			delete s3m;
				SDL_Quit();
				return EXIT_FAILURE;
			}
			LOG_MESSAGE("Opening MP3 file...");
			mp3File.open((modFileName + ".mp3").c_str(), ios::in);
			if (!mp3File.is_open())
				mp3File.open((modFileName + ".mp3").c_str(), ios::out | ios::binary | ios::trunc);
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
						LOG_ERROR("Lame Encoding Buffer too small!");
					else
						LOG_ERROR("Unknown Lame Error.");
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
// 		dosScreen.reset();
		SDL_Quit();
	}
	catch (PppException &e) {
		LOG_ERROR(string("Main (end): ")+e.what());
		return EXIT_FAILURE;
	}
	catch (PpgException &e) {
		LOG_ERROR(string("Main (end): ")+e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
