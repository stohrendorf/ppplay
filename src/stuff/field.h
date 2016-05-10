/*
    PPPlay - an old-fashioned module player
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
#include <type_traits>
#include <stdexcept>

#include <boost/throw_exception.hpp>

/**
 * @class Field
 * @brief A two-dimensional array
 * @tparam T Contained type, must be default-constructible
 */
template<class T>
class Field
{
    static_assert(std::is_default_constructible<T>::value, "T must be default constructible");
private:
    std::size_t m_width;
    std::size_t m_height;
    std::vector<T> m_data;
public:
    explicit inline Field(std::size_t width, std::size_t height, const T& val = T())
        : m_width(width), m_height(height), m_data(width* height, val)
    {
    }
    explicit inline Field() noexcept
        : m_width(0), m_height(0), m_data()
    {
    }

    virtual ~Field() = default;

    typename std::vector<T>::iterator operator[](std::size_t y)
    {
        if(y >= m_height)
            BOOST_THROW_EXCEPTION(std::out_of_range("Y coordinate out of bounds"));
        return m_data.begin() + y * m_width;
    }
    typename std::vector<T>::const_iterator operator[](std::size_t y) const
    {
        if(y >= m_height)
            BOOST_THROW_EXCEPTION(std::out_of_range("Y coordinate out of bounds"));
        return m_data.cbegin() + y * m_width;
    }

    typename std::vector<T>::reference at(std::size_t x, std::size_t y)
    {
        if(y >= m_height || x >= m_width)
            BOOST_THROW_EXCEPTION(std::out_of_range("X and/or Y coordinate out of bounds"));
        return m_data.at(y * m_width + x);
    }
    typename std::vector<T>::const_reference at(std::size_t x, std::size_t y) const
    {
        if(y >= m_height || x >= m_width)
            BOOST_THROW_EXCEPTION(std::out_of_range("X and/or Y coordinate out of bounds"));
        return m_data.at(y * m_width + x);
    }

    std::size_t width() const noexcept
    {
        return m_width;
    }
    std::size_t height() const noexcept
    {
        return m_height;
    }

    void reset(std::size_t width, std::size_t height, const T& val = T())
    {
        m_data.clear();
        m_data.resize(width * height, val);
        m_width = width;
        m_height = height;
    }

    void clear()
    {
        reset(0, 0);
    }
};

#endif
