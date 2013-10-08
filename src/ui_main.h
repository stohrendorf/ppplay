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

#ifndef UI_MAIN_H
#define UI_MAIN_H

#include "ppg/label.h"
#include "ppg/stereopeakbar.h"
#include "ppg/progressbar.h"
#include "genmod/abstractmodule.h"
#include "output/abstractaudiooutput.h"

#include <array>

class UIMain : public ppg::Widget
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
    std::weak_ptr<ppp::AbstractModule> m_module;
    AbstractAudioOutput::WeakPtr m_output;
    std::vector<uint16_t> m_fftLeft;
    std::vector<uint16_t> m_fftRight;
    virtual void drawThis();
public:
    UIMain( Widget* parent, const ppp::AbstractModule::Ptr& module, const AbstractAudioOutput::Ptr& output );
    virtual ~UIMain() = default;
    void setFft( const std::vector<uint16_t>& left, const std::vector<uint16_t>& right ) {
        m_fftLeft = left;
        m_fftRight = right;
    }
};

#endif // UI_MAIN_H
