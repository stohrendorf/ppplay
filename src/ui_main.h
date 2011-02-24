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

#ifndef UI_MAIN_H
#define UI_MAIN_H

#include "ppg/ppg.h"

#include <array>

class UIMain : public ppg::Widget {
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
		virtual void drawThis() throw( ppg::Exception ) {}
	public:
		UIMain( ppg::Widget* parent );
		ppg::Label* posLabel() {
			return m_position;
		}
		ppg::Label* playbackInfo() {
			return m_playbackInfo;
		}
		ppg::StereoPeakBar* volBar() {
			return m_volBar;
		}
		ppg::Label* chanInfo( std::size_t idx ) {
			return m_chanInfos[idx];
		}
		ppg::Label* chanCell( std::size_t idx ) {
			return m_chanCells[idx];
		}
		ppg::Label* trackerInfo() {
			return m_trackerInfo;
		}
		ppg::Label* modTitle() {
			return m_modTitle;
		}
};

#endif // UI_MAIN_H
