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

#include "ui_main.h"

#include "ppg/sdlscreen.h"

UIMain::UIMain( ppg::Widget* parent, const ppp::GenModule::Ptr& module, const IAudioOutput::Ptr& output ):
	Widget( parent ), SDLTimer(1000/30),
	m_position(nullptr),
	m_screenSep1(nullptr),
	m_screenSep2(nullptr),
	m_playbackInfo(nullptr),
	m_volBar(nullptr),
	m_chanInfos(),
	m_chanCells(),
	m_trackerInfo(nullptr),
	m_modTitle(nullptr),
	m_module(module),
	m_output(output)
{
	setSize( parent->area().size() );
	setPosition( 0, 0 );
	show();
	m_position = new ppg::Label( this );
	m_position->setWidth( parent->area().width() - 4 );
	m_position->setPosition( 2, 2 );
/*	m_position->setFgColorRange( 0, ppg::Color::BrightWhite, 0 );
	m_position->setFgColorRange( 3, ppg::Color::White, 5 );*/
	m_position->show();
	m_screenSep1 = new ppg::Label( this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 " );
	m_screenSep1->setPosition( 0, 1 );
	m_screenSep1->setFgColorRange( 0, ppg::Color::BrightWhite, 0 );
	m_screenSep1->show();
	m_screenSep2 = new ppg::Label( this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 " );
	m_screenSep2->setPosition( 0, 3 );
	m_screenSep2->setFgColorRange( 0, ppg::Color::BrightWhite, 0 );
	m_screenSep2->show();
	m_playbackInfo = new ppg::Label( this );
	m_playbackInfo->setWidth( area().width() - 4 );
	m_playbackInfo->setPosition( 2, 2 );
	m_playbackInfo->alignment = ppg::Label::Alignment::Right;
// 	m_playbackInfo->setFgColorRange( 0, ppg::Color::BrightWhite, 0 );
	m_playbackInfo->show();
	m_volBar = new ppg::StereoPeakBar( this, 16, 256, 1, true );
	m_volBar->setPosition( ( area().width() - m_volBar->length() ) / 2, 4 );
	m_volBar->show();
	for( size_t i = 0; i < m_chanInfos.size(); i++ ) {
		m_chanInfos.at(i) = new ppg::Label( this );
		m_chanInfos.at(i)->setWidth( area().width() - 4 );
		m_chanInfos.at(i)->setPosition( 2, 5 + i );
		m_chanInfos.at(i)->setFgColorRange( 4, ppg::Color::LightRed );
		m_chanInfos.at(i)->setFgColorRange( 5, ppg::Color::BrightWhite, 3 );
		m_chanInfos.at(i)->setFgColorRange( 9, ppg::Color::LightBlue );
		m_chanInfos.at(i)->setFgColorRange( 10, ppg::Color::Aqua, 2 );
		m_chanInfos.at(i)->setFgColorRange( 13, ppg::Color::LightGreen, 6 );
		m_chanInfos.at(i)->setFgColorRange( 35, ppg::Color::BrightWhite, 0 );
		m_chanInfos.at(i)->show();
	}
	for( size_t i = 0; i < m_chanCells.size(); i++ ) {
		m_chanCells.at(i) = new ppg::Label( this );
		m_chanCells.at(i)->setWidth( area().width() - 4 );
		m_chanCells.at(i)->setPosition( 2, 5 + i );
		m_chanCells.at(i)->alignment = ppg::Label::Alignment::Right;
		m_chanCells.at(i)->setFgColorRange( 0, ppg::Color::White, 0 );
		m_chanCells.at(i)->show();
	}
	m_trackerInfo = new ppg::Label( this );
	m_trackerInfo->setPosition( 2, area().height() - 2 );
	m_trackerInfo->setWidth( area().width() - 4 );
	m_trackerInfo->alignment = ppg::Label::Alignment::Center;
	m_trackerInfo->setFgColorRange( 0, ppg::Color::Aqua, 0 );
	m_trackerInfo->show();
	m_modTitle = new ppg::Label( this );
	m_modTitle->setWidth( area().width() - 4 );
	m_modTitle->setPosition( 2, 0 );
	m_modTitle->alignment = ppg::Label::Alignment::Center;
	m_modTitle->setFgColorRange( 0, ppg::Color::BrightWhite, 0 );
	m_modTitle->show();
	m_trackerInfo->setText( ppp::stringf( "Tracker: %s - Channels: %d", module->trackerInfo().c_str(), module->channelCount() ) );
	if( module->isMultiSong() )
		m_trackerInfo->setText( m_trackerInfo->text() + " - Multi-song" );
	if( module->trimmedTitle().length()>0 )
		m_modTitle->setText( std::string( " -=\xf0[ " ) + module->filename() + " : " + module->trimmedTitle() + " ]\xf0=- " );
	else
		m_modTitle->setText( std::string( " -=\xf0[ " ) + module->filename() + " ]\xf0=- " );
}

void UIMain::drawThis()
{
}

ppg::Label *UIMain::posLabel()
{
    return m_position;
}

ppg::Label *UIMain::playbackInfo()
{
    return m_playbackInfo;
}

ppg::StereoPeakBar *UIMain::volBar()
{
    return m_volBar;
}

ppg::Label *UIMain::chanInfo(size_t idx)
{
    return m_chanInfos.at(idx);
}

ppg::Label *UIMain::chanCell(size_t idx)
{
    return m_chanCells.at(idx);
}

ppg::Label *UIMain::trackerInfo()
{
    return m_trackerInfo;
}

ppg::Label *UIMain::modTitle()
{
    return m_modTitle;
}

void UIMain::onTimer()
{
	if( m_module.expired() || m_output.expired() ) {
		return;
	}
	ppp::GenModule::Ptr modLock( std::static_pointer_cast<ppp::GenModule>( m_module.lock() ) );
	IAudioSource::LockGuard guard(modLock.get());
	IAudioOutput::Ptr outLock(m_output.lock());
	ppg::SDLScreen::instance()->clear( ' ', ppg::Color::White, ppg::Color::Black );
	m_volBar->shift( outLock->volumeLeft() >> 8, outLock->volumeRight() >> 8 );
	size_t msecs = modLock->position() / 441;
	size_t msecslen = modLock->length() / 441;
	ppp::GenPlaybackInfo pbi = modLock->playbackInfo();
	if(modLock->isMultiSong()) {
		m_position->setEscapedText( ppp::stringf( "{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d \xf9 Song %d/%d",
								pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
								msecslen / 6000, msecslen / 100 % 60, msecslen % 100,
								modLock->currentSongIndex() + 1, modLock->songCount()
								) );
	}
	else {
		m_position->setEscapedText( ppp::stringf( "{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d",
								pbi.order, pbi.pattern, pbi.row, msecs / 6000, msecs / 100 % 60, msecs % 100,
								msecslen / 6000, msecslen / 100 % 60, msecslen % 100
								) );
	}
	m_playbackInfo->setEscapedText( ppp::stringf( "{BrightWhite;}Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", pbi.speed, pbi.tempo, pbi.globalVolume * 100 / 0x40 ) );
	for( uint8_t i = 0; i < modLock->channelCount(); i++ ) {
		if( i >= 16 )
			break;
		m_chanInfos.at(i)->setText( modLock->channelStatus( i ) );
		m_chanCells.at(i)->setText( modLock->channelCellString( i ) );
	}
	ppg::SDLScreen::instance()->draw();
}
