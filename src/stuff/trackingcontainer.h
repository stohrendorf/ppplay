#pragma once
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


#include <vector>
#include <stdexcept>
#include <type_traits>
#include <boost/throw_exception.hpp>

namespace detail
{
template<typename T>
struct PointerTraits
{
  using Type = typename std::remove_reference<T>::type;

  static constexpr T* getAddress(T& val)
  {
    return &val;
  }
};

template<typename T>
struct PointerTraits<T*>
{
  using Type = typename std::remove_reference<T>::type;

  static constexpr T* getAddress(T* val)
  {
    return val;
  }
};

template<typename T>
struct PointerTraits<std::unique_ptr<T>>
{
  using Type = typename std::remove_reference<T>::type;

  static constexpr T* getAddress(const std::unique_ptr<T>& val)
  {
    return val.get();
  }
};

template<typename T>
struct PointerTraits<std::shared_ptr<T>>
{
  using Type = typename std::remove_reference<T>::type;

  static constexpr T* getAddress(const std::unique_ptr<T>& val)
  {
    return val.get();
  }
};
}

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
  using Type = Tp;

  typedef typename std::add_const<Type>::type ConstType;
  typedef typename std::add_lvalue_reference<Type>::type Reference;
  typedef typename std::add_rvalue_reference<Type>::type RValReference;
  typedef typename std::add_lvalue_reference<ConstType>::type ConstReference;

public:
  /**
   * @name Indirection operators
   * @{
   */
  typename detail::PointerTraits<Type>::Type*
  operator->()
  {
    return detail::PointerTraits<Type>::getAddress( current() );
  }

  const typename detail::PointerTraits<Type>::Type*
  operator->() const
  {
    return detail::PointerTraits<Type>::getAddress( current() );
  }

  typename detail::PointerTraits<Type>::Type&
  operator*()
  {
    return *detail::PointerTraits<Type>::getAddress( current() );
  }

  const typename detail::PointerTraits<Type>::Type&
  operator*() const
  {
    return *detail::PointerTraits<Type>::getAddress( current() );
  }

  /**
   * @}
   */
  explicit TrackingContainer() = default;

  TrackingContainer(const TrackingContainer<Type>&) = delete;

  TrackingContainer(TrackingContainer<Type>&& rhs) noexcept
    : m_container( std::move( rhs.m_container ) )
    , m_cursor( std::move( rhs.m_cursor ) )
  {
  }

  TrackingContainer<Type>& operator=(const TrackingContainer<Type>&) = delete;

  TrackingContainer<Type>& operator=(TrackingContainer<Type>&& rhs) noexcept
  {
    m_container = std::move( rhs.m_container );
    m_cursor = std::move( rhs.m_cursor );
    return *this;
  }

  /**
   * @name STL compliant methods
   * @{
   */
  void push_back(const Type& value)
  {
    m_container.emplace_back( value );
  }

  inline size_t size() const noexcept
  {
    return m_container.size();
  }

  inline bool empty() const noexcept
  {
    return m_container.empty();
  }

  inline void clear()
  {
    m_container.clear();
    m_cursor.reset();
  }

  inline typename std::vector<Type>::iterator begin() noexcept
  {
    return m_container.begin();
  }

  inline typename std::vector<Type>::iterator end() noexcept
  {
    return m_container.end();
  }

  inline typename std::vector<Type>::const_iterator begin() const noexcept
  {
    return m_container.begin();
  }

  inline typename std::vector<Type>::const_iterator end() const noexcept
  {
    return m_container.end();
  }
  /**
   * @}
   */

  /**
   * @brief Index of the current element
   */
  inline size_t where() const
  {
    return *m_cursor;
  }

  /**
   * @brief Checks if the current index is at the end of the container
   * @retval true if @c where() points to the last element
   */
  inline bool atEnd() const noexcept
  {
    return m_cursor.is_initialized() && *m_cursor >= size() - 1;
  }

  /**
   * @brief Checks if the current index is at the front of the container
   * @retval true if @c where() points to the first element
   */
  inline bool atFront() const noexcept
  {
    return m_cursor.is_initialized() && *m_cursor == 0;
  }

  /**
   * @brief Resets the current element to be the first one
   */
  inline void revert() noexcept
  {
    if( !m_container.empty() )
    {
      m_cursor = 0;
    }
  }

  /**
   * @brief Check if the cursor is dangling
   */
  inline bool isDangling() const
  {
    return !m_cursor;
  }

  /**
   * @brief Get the current element
   * @return Reference to the current element
   */
  inline Reference current()
  {
    return m_container.at( *m_cursor );
  }

  /**
   * @brief Get the current element (const version)
   * @return ConstReference to the current element
   */
  inline ConstReference current() const
  {
    return m_container.at( *m_cursor );
  }

  /**
   * @brief Append a new element to the container
   * @tparam Args Types of the constructor arguments to @c Type
   * @param[in] args Arguments to the constructor of @c Type
   * @return Reference to the newly created element
   */
  template<class ...Args>
  inline Reference emplace_back(Args&& ...args)
  {
    m_container.emplace_back( std::forward<Args>( args )... );
    return m_container.back();
  }

  /**
   * @brief Go the the next element if possible
   * @return Reference to the next element
   * @throw std::out_of_range if trying to go beyond the end or if the container is empty
   */
  inline Reference next()
  {
    if( !m_cursor )
    {
      if( m_container.empty() )
        BOOST_THROW_EXCEPTION( std::out_of_range( "No more items at end" ) );

      m_cursor = 0;
      return current();
    }

    if( *m_cursor + 1 >= m_container.size() )
      BOOST_THROW_EXCEPTION( std::out_of_range( "No more items at end" ) );
    ++*m_cursor;
    return current();
  }

  /**
   * @brief Go the the previous element if possible
   * @return Reference to the previous element
   * @throw std::out_of_range if trying to go beyond the front or if the container is empty
   */
  inline Reference prev()
  {
    if( isDangling() )
      BOOST_THROW_EXCEPTION( std::out_of_range( "Container has dangling position" ) );
    if( *m_cursor == 0 )
      BOOST_THROW_EXCEPTION( std::out_of_range( "No more items at front" ) );
    --*m_cursor;
    return current();
  }

  inline TrackingContainer& operator++()
  {
    next();
    return *this;
  }

  inline TrackingContainer& operator--()
  {
    prev();
    return *this;
  }

private:
  std::vector<Type> m_container{};
  boost::optional<size_t> m_cursor = boost::none;
};
