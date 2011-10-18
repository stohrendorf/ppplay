/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2011  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "widget.h"

/**
 * @ingroup Ppg
 * @{
 */

namespace ppg {

/**
 * @class ProgressBar
 * @brief A one-line widget showing a progress
 */
class ProgressBar : public Widget {
	DISABLE_COPY(ProgressBar)
private:
	size_t m_maxVal; //!< @brief The maximum value
	size_t m_value; //!< @brief Position value (must be <= m_maxVal)
	Color m_fgColor; //!< @brief Foreground color
	Color m_bgColor; //!< @brief Background color
	virtual void drawThis();
public:
	/**
	 * @brief Constructor
	 * @param[in] parent Parent widget
	 * @param[in] maxVal Maximum value
	 * @param[in] width The width of the widget (must be >2)
	 */
	ProgressBar(Widget* parent, size_t maxVal, int width);
	//! @copydoc ppg::Widget::~Widget
	virtual ~ProgressBar();
	/**
	 * @brief Maximum value
	 * @return m_maxVal
	 */
	size_t max() const;
	/**
	 * @brief Sets the maximum value
	 * @param[in] maxVal The new maximum value
	 * @note If value() is greater than maxVal, it will be set to maxVal
	 */
	void setMax(size_t maxVal);
	/**
	 * @brief Gets the current value
	 * @return m_value
	 */
	size_t value() const;
	/**
	 * @brief Sets the current value
	 * @param[in] val The new value
	 * @note If @a val is greater than max(), it will be ignored
	 */
	void setValue(size_t val);
	/**
	 * @brief Overriden: Will not change the height
	 */
	virtual int setHeight(int h);
	/**
	 * @brief Sets the foreground color
	 * @param[in] c The new foreground color
	 */
	void setFgColor(Color c);
	/**
	 * @brief Sets the background color
	 * @param[in] c The new background color
	 */
	void setBgColor(Color c);
};

}

/**
 * @}
 */

#endif