#include "ui_main.h"

UIMain::UIMain(ppg::Widget* parent): Widget(parent)
{
	setSize(parent->area().size());
	setPosition(0,0);
	show();
	m_position = new ppg::Label(this);
	m_position->setWidth(parent->area().width()-4);
	m_position->setPosition(2,2);
	m_position->setFgColor(0, ppg::dcBrightWhite, 0);
	m_position->setFgColor(3, ppg::dcWhite, 5);
	m_position->show();
	m_screenSep1 = new ppg::Label(this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 ");
	m_screenSep1->setPosition(0,1);
	m_screenSep1->setFgColor(0, ppg::dcBrightWhite, 0);
	m_screenSep1->show();
	m_screenSep2 = new ppg::Label(this, " \xc4 \xc4\xc4  \xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4   \xc4\xc4\xc4\xc4   \xc4\xc4\xc4  \xc4\xc4 \xc4 ");
	m_screenSep2->setPosition(0,3);
	m_screenSep2->setFgColor(0, ppg::dcBrightWhite, 0);
	m_screenSep2->show();
	m_playbackInfo = new ppg::Label(this);
	m_playbackInfo->setWidth(area().width()-4);
	m_playbackInfo->setPosition(2, 2);
	m_playbackInfo->alignment = ppg::Label::Alignment::alRight;
	m_playbackInfo->setFgColor(0, ppg::dcBrightWhite, 0);
	m_playbackInfo->show();
	m_volBar = new ppg::StereoPeakBar(this, 16, 256, 1, true);
	m_volBar->setPosition((area().width()-m_volBar->length())/2, 4);
	m_volBar->show();
	for(size_t i=0; i<m_chanInfos.size(); i++) {
		m_chanInfos[i] = new ppg::Label(this);
		m_chanInfos[i]->setWidth(area().width()-4);
		m_chanInfos[i]->setPosition(2, 5+i);
		m_chanInfos[i]->setFgColor(4, ppg::dcLightRed);
		m_chanInfos[i]->setFgColor(5, ppg::dcBrightWhite, 3);
		m_chanInfos[i]->setFgColor(9, ppg::dcLightBlue);
		m_chanInfos[i]->setFgColor(10, ppg::dcAqua, 2);
		m_chanInfos[i]->setFgColor(13, ppg::dcLightGreen, 6);
		m_chanInfos[i]->setFgColor(35, ppg::dcBrightWhite, 0);
		m_chanInfos[i]->show();
	}
	for(size_t i=0; i<m_chanCells.size(); i++) {
		m_chanCells[i] = new ppg::Label(this);
		m_chanCells[i]->setWidth(area().width() - 4);
		m_chanCells[i]->setPosition(2, 5 + i);
		m_chanCells[i]->alignment = ppg::Label::Alignment::alRight;
		m_chanCells[i]->setFgColor(0, ppg::dcWhite, 0);
		m_chanCells[i]->show();
	}
	m_trackerInfo = new ppg::Label(this);
	m_trackerInfo->setPosition(2, area().height() - 2);
	m_trackerInfo->setWidth(area().width() - 4);
	m_trackerInfo->alignment = ppg::Label::Alignment::alCenter;
	m_trackerInfo->setFgColor(0, ppg::dcAqua, 0);
	m_trackerInfo->show();
	m_modTitle = new ppg::Label(this);
	m_modTitle->setFgColor(0, ppg::dcBrightWhite, 0);
	m_modTitle->setWidth(area().width() - 4);
	m_modTitle->setPosition(2, 0);
	m_modTitle->alignment = ppg::Label::Alignment::alCenter;
	m_modTitle->show();
}
