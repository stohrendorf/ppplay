/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef XMORDER_H
#define XMORDER_H

#include "genmod/abstractorder.h"

namespace ppp
{
namespace xm
{
class XmOrder : public AbstractOrder
{
    DISABLE_COPY(XmOrder)
        XmOrder() = delete;
public:
    explicit XmOrder(uint8_t idx) : AbstractOrder(idx)
    {
    }
    bool isUnplayed() const override;
};
}
}

#endif // XMORDER_H
