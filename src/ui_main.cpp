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

#include "ui_main.h"

UIMain::UIMain( Widget* parent ): Widget( parent ),
	m_position(NULL),
	m_screenSep1(NULL),
	m_screenSep2(NULL),
	m_playbackInfo(NULL),
	m_volBar(NULL),
	m_chanInfos(),
	m_chanCells(),
	m_trackerInfo(NULL),
	m_modTitle(NULL)
{
	setSize( parent->area().size() );
	setPosition( 0, 0 );
	show();
	m_position = new ppg::Label( this );
	m_position->setWidth( parent->area().width() - 4 );
	m_position->setPosition( 2, 2 );
	m_position->setFgColorRange( 0, ppg::dcBrightWhite, 0 );
	m_position->setFgColorRange( 3, ppg::dcWhite, 5 );
	m_position->show();
	m_screenSep1 = new ppg::Label( this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 " );
	m_screenSep1->setPosition( 0, 1 );
	m_screenSep1->setFgColorRange( 0, ppg::dcBrightWhite, 0 );
	m_screenSep1->show();
	m_screenSep2 = new ppg::Label( this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 " );
	m_screenSep2->setPosition( 0, 3 );
	m_screenSep2->setFgColorRange( 0, ppg::dcBrightWhite, 0 );
	m_screenSep2->show();
	m_playbackInfo = new ppg::Label( this );
	m_playbackInfo->setWidth( area().width() - 4 );
	m_playbackInfo->setPosition( 2, 2 );
	m_playbackInfo->alignment = ppg::Label::Alignment::Right;
	m_playbackInfo->setFgColorRange( 0, ppg::dcBrightWhite, 0 );
	m_playbackInfo->show();
	m_volBar = new ppg::StereoPeakBar( this, 16, 256, 1, true );
	m_volBar->setPosition( ( area().width() - m_volBar->length() ) / 2, 4 );
	m_volBar->show();
	for( size_t i = 0; i < m_chanInfos.size(); i++ ) {
		m_chanInfos[i] = new ppg::Label( this );
		m_chanInfos[i]->setWidth( area().width() - 4 );
		m_chanInfos[i]->setPosition( 2, 5 + i );
		m_chanInfos[i]->setFgColorRange( 4, ppg::dcLightRed );
		m_chanInfos[i]->setFgColorRange( 5, ppg::dcBrightWhite, 3 );
		m_chanInfos[i]->setFgColorRange( 9, ppg::dcLightBlue );
		m_chanInfos[i]->setFgColorRange( 10, ppg::dcAqua, 2 );
		m_chanInfos[i]->setFgColorRange( 13, ppg::dcLightGreen, 6 );
		m_chanInfos[i]->setFgColorRange( 35, ppg::dcBrightWhite, 0 );
		m_chanInfos[i]->show();
	}
	for( size_t i = 0; i < m_chanCells.size(); i++ ) {
		m_chanCells[i] = new ppg::Label( this );
		m_chanCells[i]->setWidth( area().width() - 4 );
		m_chanCells[i]->setPosition( 2, 5 + i );
		m_chanCells[i]->alignment = ppg::Label::Alignment::Right;
		m_chanCells[i]->setFgColorRange( 0, ppg::dcWhite, 0 );
		m_chanCells[i]->show();
	}
	m_trackerInfo = new ppg::Label( this );
	m_trackerInfo->setPosition( 2, area().height() - 2 );
	m_trackerInfo->setWidth( area().width() - 4 );
	m_trackerInfo->alignment = ppg::Label::Alignment::Center;
	m_trackerInfo->setFgColorRange( 0, ppg::dcAqua, 0 );
	m_trackerInfo->show();
	m_modTitle = new ppg::Label( this );
	m_modTitle->setWidth( area().width() - 4 );
	m_modTitle->setPosition( 2, 0 );
	m_modTitle->alignment = ppg::Label::Alignment::Center;
	m_modTitle->setFgColorRange( 0, ppg::dcBrightWhite, 0 );
	m_modTitle->show();
}

void UIMain::drawThis() throw(ppg::Exception)
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
    return m_chanInfos[idx];
}

ppg::Label *UIMain::chanCell(size_t idx)
{
    return m_chanCells[idx];
}

ppg::Label *UIMain::trackerInfo()
{
    return m_trackerInfo;
}

ppg::Label *UIMain::modTitle()
{
    return m_modTitle;
}
