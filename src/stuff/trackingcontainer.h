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

#ifndef PPPLAY_TRACKINGCONTAINER_H
#define PPPLAY_TRACKINGCONTAINER_H

#include "utils.h"

#include <vector>
#include <stdexcept>
#include <type_traits>
#include <limits>

/**
 * @class TrackingContainer
 * @ingroup Common
 * A wrapper around a std::vector that keeps track of the currently used element
 * @tparam Tp Contained type
 * @note This class has overloaded indirection operators for better access to the
 *       current element.
 */
template<class Tp>
class TrackingContainer
{
public:
	//! Typedef for the underlying type.
	typedef Tp Type;
	
	typedef typename std::add_const<Type>::type ConstType;
	typedef typename std::add_lvalue_reference<Type>::type Reference;
	typedef typename std::add_rvalue_reference<Type>::type RValReference;
	typedef typename std::add_lvalue_reference<ConstType>::type ConstReference;
public:
	/**
	 * @name Indirection operators
	 * @{
	 */
	template<class T = Type>
	typename std::enable_if<std::is_pointer<T>::value, T>::type
	operator->() {
		return current();
	}
	template<class T = Type>
	typename std::enable_if<!std::is_pointer<T>::value, T*>::type
	operator->() {
		return &current();
	}
	template<class T = ConstType>
	typename std::enable_if<std::is_pointer<T>::value, T>::type
	operator->() const {
		return current();
	}
	template<class T = ConstType>
	typename std::enable_if<!std::is_pointer<T>::value, T*>::type
	operator->() const {
		return &current();
	}
	template<class T = Type>
	typename std::enable_if<std::is_pointer<T>::value, T>::type
	operator*() {
		return current();
	}
	template<class T = Type>
	typename std::enable_if<!std::is_pointer<T>::value, T*>::type
	operator*() {
		return &current();
	}
	template<class T = ConstType>
	typename std::enable_if<std::is_pointer<T>::value, T>::type
	operator*() const {
		return current();
	}
	template<class T = ConstType>
	typename std::enable_if<!std::is_pointer<T>::value, T*>::type
	operator*() const {
		return &current();
	}
	/**
	 * @}
	 */
	inline TrackingContainer() : m_container(), m_index( std::numeric_limits<size_t>::max() )
	{
	}
	TrackingContainer(const TrackingContainer<Type>&) = delete;
	inline TrackingContainer(TrackingContainer<Type>&& rhs) : m_container(std::move(rhs.m_container)), m_index(rhs.m_index)
	{
		rhs.clear();
	}
	TrackingContainer<Type>& operator=(const TrackingContainer<Type>&) = delete;
	TrackingContainer<Type>& operator=(TrackingContainer<Type>&& rhs)
	{
		m_container = std::move(rhs.m_container);
		m_index = rhs.m_index;
		rhs.clear();
		return *this;
	}
	/**
	 * @name STL compliant methods
	 * @{
	 */
	template<class T = Type>
	typename std::enable_if<!std::is_move_constructible<T>::value, void>::type
	push_back( ConstReference value ) {
		m_container.push_back( value );
	}
	template<class T = Type>
	typename std::enable_if<std::is_move_constructible<T>::value, void>::type
	push_back( RValReference value ) {
		m_container.push_back( value );
	}
	inline size_t size() const {
		return m_container.size();
	}
	inline bool empty() const {
		return m_container.empty();
	}
	inline void clear() {
		m_container.clear();
		m_index = std::numeric_limits<size_t>::max();
	}
	inline typename std::vector<Type>::iterator begin() {
		return m_container.begin();
	}
	inline typename std::vector<Type>::iterator end() {
		return m_container.end();
	}
	inline typename std::vector<Type>::const_iterator begin() const {
		return m_container.begin();
	}
	inline typename std::vector<Type>::const_iterator end() const {
		return m_container.end();
	}
	/**
	 * @}
	 */
	/**
	 * @brief Index of the current element
	 * @retval -1 if the container is empty
	 */
	inline size_t where() const {
		return m_index;
	}
	/**
	 * @brief Checks if the current index is at the end of the container
	 * @retval true if @c where() points to the last element
	 */
	inline bool atEnd() const {
		return m_index==std::numeric_limits<size_t>::max() || m_index >= size() - 1;
	}
	/**
	 * @brief Checks if the current index is at the front of the container
	 * @retval true if @c where() points to the first element
	 */
	inline bool atFront() const {
		return m_index==std::numeric_limits<size_t>::max() || m_index == 0;
	}
	/**
	 * @brief Resets the current element to be the first one
	 */
	inline void revert() {
		if(m_index!=std::numeric_limits<size_t>::max()) {
			m_index = 0;
		}
	}
	/**
	 * @brief Get the current element
	 * @return Reference to the current element
	 */
	inline Reference current() {
		return m_container.at( m_index );
	}
	/**
	 * @brief Get the current element (const version)
	 * @return ConstReference to the current element
	 */
	inline ConstReference current() const {
		return m_container.at( m_index );
	}
	/**
	 * @brief Append a new element to the container
	 * @tparam Args Types of the constructor arguments to @c Type
	 * @param[in] args Arguments to the constructor of @c Type
	 * @return Reference to the newly created element
	 */
	template<class ...Args>
	inline Reference append( const Args& ...args ) {
		push_back( Type( args... ) );
		return m_container.back();
	}
	/**
	 * @brief Go the the next element if possible
	 * @return Reference to the next element
	 * @throw std::out_of_range if trying to go beyond the end or if the container is empty
	 */
	inline Reference next() {
		if( m_index + 1 >= m_container.size() ) throw std::out_of_range( "No more items at end" );
		m_index++;
		return current();
	}
	/**
	 * @brief Go the the previous element if possible
	 * @return Reference to the previous element
	 * @throw std::out_of_range if trying to go beyond the front or if the container is empty
	 */
	inline Reference prev() {
		if( m_index == std::numeric_limits<size_t>::max() ) throw std::out_of_range( "Container is empty" );
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

