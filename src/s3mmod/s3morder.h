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


#ifndef S3MORDER_H
#define S3MORDER_H

#include "genmod/genorder.h"

namespace ppp
{
namespace s3m
{

class S3mOrder : public GenOrder
{
	DISABLE_COPY( S3mOrder )
	S3mOrder() = delete;
public:
	inline S3mOrder( uint8_t idx ) : GenOrder( idx ) {
	}
	virtual bool isUnplayed() const;
};

}
}

#endif // S3MORDER_H