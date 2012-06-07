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

#ifndef UI_MAIN_H
#define UI_MAIN_H

#include "ppg/label.h"
#include "ppg/stereopeakbar.h"
#include "ppg/progressbar.h"
#include "stuff/sdltimer.h"
#include "genmod/genmodule.h"
#include "output/iaudiooutput.h"

#include <array>

class UIMain : public ppg::Widget, public SDLTimer
{
	DISABLE_COPY( UIMain )
private:
	ppg::Label* m_position;
	ppg::Label* m_screenSep1;
	ppg::Label* m_screenSep2;
	ppg::Label* m_playbackInfo;
	ppg::StereoPeakBar* m_volBar;
	std::array<ppg::Label*, 16> m_chanInfos;
	std::array<ppg::Label*, 16> m_chanCells;
	ppg::Label* m_trackerInfo;
	ppg::Label* m_modTitle;
	ppg::ProgressBar* m_progress;
	std::weak_ptr<ppp::GenModule> m_module;
	IAudioOutput::WeakPtr m_output;
	virtual void drawThis();
public:
	UIMain( Widget* parent, const ppp::GenModule::Ptr& module, const IAudioOutput::Ptr& output );
	virtual ~UIMain();
	ppg::Label* posLabel();
	ppg::Label* playbackInfo();
	ppg::StereoPeakBar* volBar();
	ppg::Label* chanInfo( size_t idx );
	ppg::Label* chanCell( size_t idx );
	ppg::Label* trackerInfo();
	ppg::Label* modTitle();
	ppg::ProgressBar* progressBar();
	virtual void onTimer();
};

#endif // UI_MAIN_H
