/*
    PeePeePlayer - an old-fashioned module player
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

#ifndef FIELD_H
#define FIELD_H

#include <vector>
#include <cassert>

/**
 * @class Field
 * @brief A two-dimensional array
 * @tparam T Contained type, must be default-constructable
 */
template<class T>
class Field
{
private:
	std::size_t m_width;
	std::vector<T> m_data;
public:
	explicit inline Field(std::size_t width, std::size_t height)
	: m_width(width), m_data(width*height)
	{
	}
	explicit inline Field()
	: m_width(0), m_data()
	{
	}
	
	virtual ~Field() {}
	
	typename std::vector<T>::iterator operator[](std::size_t y)
	{
		assert(!m_data.empty());
		return m_data.begin() + y*m_width;
	}
	typename std::vector<T>::const_iterator operator[](std::size_t y) const
	{
		assert(!m_data.empty());
		return m_data.cbegin() + y*m_width;
	}
	
	typename std::vector<T>::reference at(std::size_t x, std::size_t y)
	{
		assert(!m_data.empty());
		return m_data.at(y*m_width+x);
	}
	typename std::vector<T>::const_reference at(std::size_t x, std::size_t y) const
	{
		assert(!m_data.empty());
		return m_data.at(y*m_width+x);
	}
	
	std::size_t width() const
	{
		if( m_data.empty() ) {
			return 0;
		}
		return m_width;
	}
	std::size_t height() const
	{
		if( m_data.empty() ) {
			return 0;
		}
		return m_data.size() / m_width;
	}
	
	void reset(std::size_t width, std::size_t height)
	{
		m_data.clear();
		m_data.resize(width*height);
		m_width = width;
	}
};

#endif
