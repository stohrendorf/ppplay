/***************************************************************************
 *   Copyright (C) 2009 by Syron                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef pvectorH
#define pvectorH

#include <memory>
#include <vector>

/**
 * @file
 * @ingroup Common
 * @brief The ::PVector template definition
 */

/**
 * @ingroup Common
 * @brief Defines a template class like std::vector, but uses std::shared_ptr of type @a T as elements
 * @tparam T Type to create a pointer from
 * @details
 * This template is like std::vector, but automatically uses typed pointers
 * of the used type. @n
 * Practically, it is a pointer-containing vector subset with RAII.
 */
template < class T >
class PVector {
	public:
		typedef std::shared_ptr<T> TypePtr; //!< @brief Vector's data type
		typedef std::shared_ptr< PVector<T> > Ptr; //!< @brief Class pointer
	private:
		std::vector<TypePtr> m_data; //!< @brief Vector containing the data
	public:
		PVector() : m_data() { }
		/**
		 * @brief Get the vector's size
		 * @return The vector's size
		 */
		unsigned int size() const { return m_data.size(); }
		/**
		 * @brief Array operator member access
		 * @param[in] index Index of the element to access
		 * @return Reference to the element
		 */
		TypePtr &operator[](std::size_t index) { return m_data[index]; }
		TypePtr &at(std::size_t index) { return m_data[index]; }
		/**
		 * @brief Array operator member access (const version)
		 * @param[in] index Index of the element to access
		 * @return Reference to the element
		 */
		const TypePtr &operator[](std::size_t index) const { return m_data[index]; }
		const TypePtr &at(std::size_t index) const { return m_data[index]; }
		/**
		 * @brief Add an element to the vector
		 * @param[in] val Element to add
		 */
		void push_back(TypePtr val) { m_data.push_back(val); }
};

#define PVECTOR_TEMPLATE_DECL(defname) \
extern template class PVector< defname >; \
extern template class std::shared_ptr< PVector< defname > >; \
extern template class std::vector< std::shared_ptr< defname > >;

#define PVECTOR_TEMPLATE_IMPL(defname) \
template class PVector< defname >; \
template class std::shared_ptr< PVector< defname > >; \
template class std::vector< std::shared_ptr< defname > >;

#endif
