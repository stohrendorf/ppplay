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

static light4cxx::Logger::Ptr logger()
{
	return light4cxx::Logger::get( "ui.main" );
}

UIMain::UIMain( ppg::Widget* parent, const ppp::GenModule::Ptr& module, const IAudioOutput::Ptr& output ):
	Widget( parent ), SDLTimer( 1000 / 30 ),
	m_position( nullptr ),
	m_screenSep1( nullptr ),
	m_screenSep2( nullptr ),
	m_playbackInfo( nullptr ),
	m_volBar( nullptr ),
	m_chanInfos(),
	m_chanCells(),
	m_trackerInfo( nullptr ),
	m_modTitle( nullptr ),
	m_progress( nullptr ),
	m_module( module ),
	m_output( output ),
	m_timerMutex()
{
	logger()->trace( L4CXX_LOCATION, "Initializing" );
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
		m_chanInfos.at( i ) = new ppg::Label( this );
		m_chanInfos.at( i )->setWidth( area().width() - 4 );
		m_chanInfos.at( i )->setPosition( 2, 5 + i );
		m_chanInfos.at( i )->setFgColorRange( 4, ppg::Color::LightRed );
		m_chanInfos.at( i )->setFgColorRange( 5, ppg::Color::BrightWhite, 3 );
		m_chanInfos.at( i )->setFgColorRange( 9, ppg::Color::LightBlue );
		m_chanInfos.at( i )->setFgColorRange( 10, ppg::Color::Aqua, 2 );
		m_chanInfos.at( i )->setFgColorRange( 13, ppg::Color::LightGreen, 6 );
		m_chanInfos.at( i )->setFgColorRange( 35, ppg::Color::BrightWhite, 0 );
		m_chanInfos.at( i )->show();
	}
	for( size_t i = 0; i < m_chanCells.size(); i++ ) {
		m_chanCells.at( i ) = new ppg::Label( this );
		m_chanCells.at( i )->setWidth( area().width() - 4 );
		m_chanCells.at( i )->setPosition( 2, 5 + i );
		m_chanCells.at( i )->alignment = ppg::Label::Alignment::Right;
		m_chanCells.at( i )->setFgColorRange( 0, ppg::Color::White, 0 );
		m_chanCells.at( i )->show();
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
	m_trackerInfo->setText( stringFmt( "Tracker: %s - Channels: %d", std::const_pointer_cast<const ppp::GenModule>(module)->metaInfo().trackerInfo, int(module->channelCount()) ) );
	if( module->isMultiSong() ) {
		m_trackerInfo->setText( m_trackerInfo->text() + " - Multi-song" );
	}
	if( module->trimmedTitle().length() > 0 ) {
		m_modTitle->setText( std::string( " -=\xf0[ " ) + module->filename() + " : " + module->trimmedTitle() + " ]\xf0=- " );
	}
	else {
		m_modTitle->setText( std::string( " -=\xf0[ " ) + module->filename() + " ]\xf0=- " );
	}
	m_progress = new ppg::ProgressBar( this, 0, 40 );
	m_progress->setPosition( ( area().width() - 40 ) / 2, 3 );
	m_progress->setFgColor( ppg::Color::BrightWhite );
	m_progress->show();
	toTop( m_progress );
	logger()->trace( L4CXX_LOCATION, "Initialized" );
}

UIMain::~UIMain()
{
	boost::recursive_mutex::scoped_lock lock(m_timerMutex);
}

void UIMain::drawThis()
{
}

ppg::Label* UIMain::posLabel()
{
	return m_position;
}

ppg::Label* UIMain::playbackInfo()
{
	return m_playbackInfo;
}

ppg::StereoPeakBar* UIMain::volBar()
{
	return m_volBar;
}

ppg::Label* UIMain::chanInfo( size_t idx )
{
	return m_chanInfos.at( idx );
}

ppg::Label* UIMain::chanCell( size_t idx )
{
	return m_chanCells.at( idx );
}

ppg::Label* UIMain::trackerInfo()
{
	return m_trackerInfo;
}

ppg::Label* UIMain::modTitle()
{
	return m_modTitle;
}

void UIMain::onTimer()
{
	boost::recursive_mutex::scoped_lock lock(m_timerMutex);
	IAudioOutput::Ptr outLock( m_output.lock() );
	const std::shared_ptr<const ppp::GenModule> modLock = std::const_pointer_cast<const ppp::GenModule>( m_module.lock() );
	if( m_module.expired() || m_output.expired() ) {
		logger()->trace( L4CXX_LOCATION, "Module expired" );
		return;
	}
	{
		logger()->trace( L4CXX_LOCATION, "Updating" );
		// this MUST be in its own scope so that the lock gets released
		// before drawing. drawing may block.
		//IAudioSource::LockGuard guard( modLock.get() );
		logger()->trace( L4CXX_LOCATION, "Module locked" );
		logger()->trace( L4CXX_LOCATION, "Output device locked" );
		ppg::SDLScreen::instance()->clear( ' ', ppg::Color::White, ppg::Color::Black );
		m_volBar->shift( outLock->volumeLeft() >> 8, outLock->volumeRight() >> 8 );
		size_t msecs = modLock->state().playedFrames / 441;
		size_t msecslen = modLock->length() / 441;
		const ppp::ModuleState state = modLock->state();
		boost::format posStr = boost::format( "{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %02d:%02d.%02d/%02d:%02d.%02d" )
							   % state.order
							   % state.pattern
							   % state.row
							   % ( msecs / 6000 )
							   % ( msecs / 100 % 60 )
							   % ( msecs % 100 )
							   % ( msecslen / 6000 )
							   % ( msecslen / 100 % 60 )
							   % ( msecslen % 100 );
		if( modLock->isMultiSong() ) {
			posStr = boost::format( "%s \xf9 Song %d/%d" ) % posStr % ( modLock->currentSongIndex() + 1 ) % modLock->songCount();
		}
		m_position->setEscapedText( posStr.str() );
// 		if(modLock->isMultiSong()) {
// 			m_position->setEscapedText( ppp::stringf( "{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d \xf9 Song %d/%d",
// 									modLock->order(), modLock->patternIndex(), modLock->row(), msecs / 6000, msecs / 100 % 60, msecs % 100,
// 									msecslen / 6000, msecslen / 100 % 60, msecslen % 100,
// 									modLock->currentSongIndex() + 1, modLock->songCount()
// 									) );
// 		}
// 		else {
// 			m_position->setEscapedText( ppp::stringf( "{BrightWhite;}%3d{White;}(%3d){BrightWhite;}/%2d \xf9 %.2d:%.2d.%.2d/%.2d:%.2d.%.2d",
// 									modLock->order(), modLock->patternIndex(), modLock->row(), msecs / 6000, msecs / 100 % 60, msecs % 100,
// 									msecslen / 6000, msecslen / 100 % 60, msecslen % 100
// 									) );
// 		}
		m_playbackInfo->setEscapedText( stringFmt( "{BrightWhite;}Speed:%2d \xf9 Tempo:%3d \xf9 Vol:%3d%%", state.speed, state.tempo, state.globalVolume * 100 / 0x40 ) );
		for( int i = 0; i < modLock->channelCount(); i++ ) {
			if( i >= 16 )
				break;
			m_chanInfos.at( i )->setText( modLock->channelStatus( i ) );
			m_chanCells.at( i )->setText( modLock->channelCellString( i ) );
		}
		m_progress->setMax( modLock->length() );
		m_progress->setValue( modLock->state().playedFrames );
	}
	logger()->trace( L4CXX_LOCATION, "Drawing" );
	ppg::SDLScreen::instance()->draw();
}
