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

#ifndef TRACKINGCONTAINER_H
#define TRACKINGCONTAINER_H

#include "utils.h"

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/addressof.hpp>
#include <vector>
#include <stdexcept>

template<class Tp>
class TrackingContainer
{
public:
	typedef Tp Type;
	typedef typename boost::add_const<Type>::type ConstType;
	typedef typename boost::add_reference<Type>::type Reference;
	typedef typename boost::add_reference<ConstType>::type ConstReference;
public:
	/**
	 * @name Indirection operators
	 * @{
	 */
	template<class T = Type>
	typename boost::enable_if_c<boost::is_pointer<T>::value, T>::type
	operator->() {
		return current();
	}
	template<class T = Type>
	typename boost::enable_if_c<!boost::is_pointer<T>::value, T*>::type
	operator->() {
		return &current();
	}
	template<class T = ConstType>
	typename boost::enable_if_c<boost::is_pointer<T>::value, T>::type
	operator->() const {
		return current();
	}
	template<class T = ConstType>
	typename boost::enable_if_c<!boost::is_pointer<T>::value, T*>::type
	operator->() const {
		return &current();
	}
	template<class T = Type>
	typename boost::enable_if_c<boost::is_pointer<T>::value, T>::type
	operator*() {
		return current();
	}
	template<class T = Type>
	typename boost::enable_if_c<!boost::is_pointer<T>::value, T*>::type
	operator*() {
		return &current();
	}
	template<class T = ConstType>
	typename boost::enable_if_c<boost::is_pointer<T>::value, T>::type
	operator*() const {
		return current();
	}
	template<class T = ConstType>
	typename boost::enable_if_c<!boost::is_pointer<T>::value, T*>::type
	operator*() const {
		return &current();
	}
	/**
	 * @}
	 */
	inline TrackingContainer() : m_container(), m_index( -1 ) {
	}
	/**
	 * @name STL compliant methods
	 * @{
	 */
	inline void push_back( ConstReference value ) {
		m_container.push_back( value );
	}
	inline size_t size() const {
		return m_container.size();
	}
	inline bool empty() const {
		return m_container.empty();
	}
	/**
	 * @}
	 */
	inline size_t where() const {
		return m_index;
	}
	inline bool atEnd() const {
		return where() >= size() - 1;
	}
	inline bool atFront() const {
		return where() == 0;
	}
	inline void revert() {
		m_index = 0;
	}
	inline Reference current() {
		return m_container.at( m_index );
	}
	inline ConstReference current() const {
		return m_container.at( m_index );
	}
	template<class ...Args>
	inline Reference append( const Args& ...args ) {
		push_back( Type( args... ) );
		return m_container.back();
	}
	inline Reference next() {
		if( m_index + 1 >= m_container.size() ) throw std::out_of_range( "No more items at end" );
		m_index++;
		return current();
	}
	inline Reference prev() {
		if( m_index == 0 ) throw std::out_of_range( "No more items at front" );
		m_index--;
		return current();
	}
	inline TrackingContainer& operator++() {
		next();
		return *this;
	}
	inline TrackingContainer& operator--() {
		prev();
		return *this;
	}
private:
	std::vector<Type> m_container;
	size_t m_index;
};

#endif

