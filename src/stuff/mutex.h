/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Steffen Ohrendorf <email>

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


#ifndef MUTEX_H
#define MUTEX_H

#include "utils.h"

class Mutex {
		DISABLE_COPY(Mutex)
	public:
		Mutex();
		~Mutex();
		void lock();
		void unlock();
	private:
		struct SDL_mutex* m_mutex;
};

#endif // MUTEX_H
