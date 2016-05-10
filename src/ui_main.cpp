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

#include "ui_main.h"

#include "ppg/sdlscreen.h"
#include "genmod/channelstate.h"
#include "genmod/genbase.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace
{
light4cxx::Logger* logger()
{
    return light4cxx::Logger::get("ui.main");
}
std::string stateToString(size_t idx, const ppp::ChannelState& state)
{
    std::string res = stringFmt("%02d ", idx + 1);
    if(!state.active)
    {
        return res;
    }
    res += state.noteTriggered ? '*' : ' ';
    if(state.note == ppp::ChannelState::NoNote)
    {
        res += "... ";
    }
    else if(state.note == ppp::ChannelState::NoteCut)
    {
        res += "^^^ ";
    }
    else if(state.note == ppp::ChannelState::KeyOff)
    {
        res += "=== ";
    }
    else if(state.note == ppp::ChannelState::TooLow)
    {
        res += "___ ";
    }
    else if(state.note == ppp::ChannelState::TooHigh)
    {
        res += "+++ ";
    }
    else
    {
        res += ppp::NoteNames[state.note % 12];
        res += char('0' + state.note / 12);
        res += ' ';
    }

    res += state.fxDesc;
    res += stringFmt(" V:%3d%%", int(state.volume));
    if(state.panning == -100)
    {
        res += " P:Left  ";
    }
    else if(state.panning == 0)
    {
        res += " P:Centr ";
    }
    else if(state.panning == 100)
    {
        res += " P:Right ";
    }
    else if(state.panning == ppp::ChannelState::Surround)
    {
        res += " P:Srnd  ";
    }
    else
    {
        res += stringFmt(" P:%4d%% ", int(state.panning));
    }
    res += state.instrumentName;
    return res;
}
}

UIMain::UIMain(ppg::Widget* parent, const ppp::AbstractModule::Ptr& module, const AbstractAudioOutput::Ptr& output) :
    Widget(parent),
    m_position(nullptr),
    m_screenSep1(nullptr),
    m_screenSep2(nullptr),
    m_playbackInfo(nullptr),
    m_volBar(nullptr),
    m_chanInfos(),
    m_chanCells(),
    m_trackerInfo(nullptr),
    m_modTitle(nullptr),
    m_progress(nullptr),
    m_module(module),
    m_output(output),
    m_fftLeft(),
    m_fftRight()
{
    logger()->trace(L4CXX_LOCATION, "Initializing");
    UIMain::setSize(parent->area().size());
    UIMain::setPosition(0, 0);
    show();
    m_position = new ppg::Label(this);
    m_position->setWidth(parent->area().width() - 4);
    m_position->setPosition(2, 2);
    /*	m_position->setFgColorRange( 0, ppg::Color::BrightWhite, 0 );
        m_position->setFgColorRange( 3, ppg::Color::White, 5 );*/
    m_position->show();
    m_screenSep1 = new ppg::Label(this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 ");
    m_screenSep1->setPosition(0, 1);
    m_screenSep1->setFgColorRange(0, ppg::Color::BrightWhite, 0);
    m_screenSep1->show();
    m_screenSep2 = new ppg::Label(this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 ");
    m_screenSep2->setPosition(0, 3);
    m_screenSep2->setFgColorRange(0, ppg::Color::BrightWhite, 0);
    m_screenSep2->show();
    m_playbackInfo = new ppg::Label(this);
    m_playbackInfo->setWidth(area().width() - 4);
    m_playbackInfo->setPosition(2, 2);
    m_playbackInfo->alignment = ppg::Label::Alignment::Right;
    // 	m_playbackInfo->setFgColorRange( 0, ppg::Color::BrightWhite, 0 );
    m_playbackInfo->show();
    m_volBar = new ppg::StereoPeakBar(this, 16, 256, 1, true);
    m_volBar->setPosition((area().width() - m_volBar->length()) / 2, 4);
    m_volBar->show();
    for(size_t i = 0; i < m_chanInfos.size(); i++)
    {
        m_chanInfos.at(i) = new ppg::Label(this);
        m_chanInfos.at(i)->setWidth(area().width() - 4);
        m_chanInfos.at(i)->setPosition(2, 5 + i);
        // note triggered
        m_chanInfos.at(i)->setFgColorRange(3, ppg::Color::LightRed);
        // note
        m_chanInfos.at(i)->setFgColorRange(4, ppg::Color::BrightWhite, 3);
        // effect
        m_chanInfos.at(i)->setFgColorRange(8, ppg::Color::LightGreen, 6);
        m_chanInfos.at(i)->setFgColorRange(15, ppg::Color::BrightWhite, 0);
        m_chanInfos.at(i)->show();
    }
    for(size_t i = 0; i < m_chanCells.size(); i++)
    {
        m_chanCells.at(i) = new ppg::Label(this);
        m_chanCells.at(i)->setWidth(area().width() - 4);
        m_chanCells.at(i)->setPosition(2, 5 + i);
        m_chanCells.at(i)->alignment = ppg::Label::Alignment::Right;
        m_chanCells.at(i)->setFgColorRange(0, ppg::Color::White, 0);
        m_chanCells.at(i)->show();
    }
    m_trackerInfo = new ppg::Label(this);
    m_trackerInfo->setPosition(2, area().height() - 2);
    m_trackerInfo->setWidth(area().width() - 4);
    m_trackerInfo->alignment = ppg::Label::Alignment::Center;
    m_trackerInfo->setFgColorRange(0, ppg::Color::Aqua, 0);
    m_trackerInfo->show();
    m_modTitle = new ppg::Label(this);
    m_modTitle->setWidth(area().width() - 4);
    m_modTitle->setPosition(2, 0);
    m_modTitle->alignment = ppg::Label::Alignment::Center;
    m_modTitle->setFgColorRange(0, ppg::Color::BrightWhite, 0);
    m_modTitle->show();
    m_trackerInfo->setText(stringFmt("Tracker: %s - Channels: %d", std::const_pointer_cast<const ppp::AbstractModule>(module)->metaInfo().trackerInfo, int(module->channelCount())));
    if(module->songCount() > 1)
    {
        m_trackerInfo->setText(m_trackerInfo->text() + " - Multi-song");
    }
#ifdef WIN32
    char fname[260];
    wcstombs(fname, boost::filesystem::path(module->metaInfo().filename).filename().native().c_str(), 259);
#else
    auto fname = boost::filesystem::path(module->metaInfo().filename).filename().native();
#endif
    if(!boost::trim_copy(module->metaInfo().title).empty())
    {
        m_modTitle->setText(std::string(" -=\xf0[ ") + fname + " : " + boost::trim_copy(module->metaInfo().title) + " ]\xf0=- ");
    }
    else
    {
        m_modTitle->setText(std::string(" -=\xf0[ ") + fname + " ]\xf0=- ");
    }
    m_progress = new ppg::ProgressBar(this, 0, 40);
    m_progress->setPosition((area().width() - 40) / 2, 3);
    m_progress->setFgColor(ppg::Color::BrightWhite);
    m_progress->show();
    UIMain::toTop(m_progress);
    logger()->trace(L4CXX_LOCATION, "Initialized");
}

void UIMain::drawThis()
{
    AbstractAudioOutput::Ptr outLock(m_output.lock());
    const std::shared_ptr<const ppp::AbstractModule> modLock = std::const_pointer_cast<const ppp::AbstractModule>(m_module.lock());
    if(m_module.expired() || m_output.expired())
    {
        logger()->trace(L4CXX_LOCATION, "Module or Output Device expired");
        return;
    }
    logger()->trace(L4CXX_LOCATION, "Updating");
    m_volBar->shift(outLock->volumeLeft() >> 8, outLock->volumeRight() >> 8);
    size_t msecs = modLock->state().playedFrames / 441;
    size_t msecslen = modLock->length() / 441;
    const ppp::ModuleState state = modLock->state();
    std::string posStr = stringFmt("{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %02d:%02d.%02d/%02d:%02d.%02d",
                                   state.order,
                                   state.pattern,
                                   state.row,
                                   msecs / 6000,
                                   msecs / 100 % 60,
                                   msecs % 100,
                                   msecslen / 6000,
                                   msecslen / 100 % 60,
                                   msecslen % 100);
    if(modLock->songCount() > 1)
    {
        posStr += stringFmt(" \xf9 Song %d/%d", modLock->currentSongIndex() + 1, modLock->songCount());
    }
    m_position->setEscapedText(posStr);
    m_playbackInfo->setEscapedText(stringFmt("{BrightWhite;}Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", state.speed, state.tempo, state.globalVolume * 100 / 0x40));
    for(int i = 0; i < modLock->channelCount(); i++)
    {
        if(i >= 16)
        {
            break;
        }
        const ppp::ChannelState chanState = modLock->channelStatus(i);
        m_chanCells.at(i)->setText(chanState.cell);
        m_chanInfos.at(i)->setText(stateToString(i, chanState));
    }
    m_progress->setMax(modLock->length());
    m_progress->setValue(modLock->state().playedFrames);
    logger()->trace(L4CXX_LOCATION, "Drawing");

    const int width2 = ppg::SDLScreen::instance()->area().width() * 4;
    const int height = ppg::SDLScreen::instance()->area().height() * 16;
    const float scale = m_fftLeft.size() * 1.0f / width2;

    ppg::SDLScreen::instance()->lockPixels();
    for(size_t i = 0; i < m_fftLeft.size(); i++)
    {
        for(size_t y = 1; y <= m_fftLeft[i] / 4; y++)
        {
            ppg::SDLScreen::instance()->drawPixel(i / scale, height - y - 1, ppg::Color::LightGreen);
        }
        for(size_t y = 1; y <= m_fftRight[i] / 4; y++)
        {
            ppg::SDLScreen::instance()->drawPixel(i / scale + width2, height - y - 1, ppg::Color::LightRed);
        }
    }
    ppg::SDLScreen::instance()->unlockPixels();
}