#ifndef UI_MAIN_H
#define UI_MAIN_H

#include "ppg/ppg.h"
#include "ppg/ppgbase.h"

#include <array>

class UIMain : public ppg::Widget {
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
	virtual void drawThis() throw(ppg::Exception) {}
public:
	UIMain(ppg::Widget*parent);
	ppg::Label* const posLabel() { return m_position; }
	ppg::Label* const playbackInfo() { return m_playbackInfo; }
	ppg::StereoPeakBar* const volBar() { return m_volBar; }
	ppg::Label* const chanInfo(std::size_t idx) { return m_chanInfos[idx]; }
	ppg::Label* const chanCell(std::size_t idx) { return m_chanCells[idx]; }
	ppg::Label* const trackerInfo() { return m_trackerInfo; }
	ppg::Label* const modTitle() { return m_modTitle; }
};

#endif // UI_MAIN_H