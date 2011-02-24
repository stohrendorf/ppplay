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

#ifndef pppexceptH
#define pppexceptH

#include <iostream>
#include <string>
#include <typeinfo>
#include <exception>
#include <stdexcept>

/**
 * @file
 * @ingroup Common
 * @brief PeePeePlayer Exception definitions
 */

/**
 * @ingroup Common
 * @brief A simple tracing exception for PeePeePlayer
 */
class PppException : public std::exception {
	private:
		std::string m_msg; //!< @brief Exception message
	public:
		/**
		 * @brief Constructor with initial message
		 * @param[in] msg Initial exception message
		 */
		PppException( const std::string& msg ) throw();
		/**
		 * @brief Constructor with additional information
		 * @param[in] msg Initial Message
		 * @param[in] lineno Line number (use @c __LINE__ here)
		 * @param[in] function Function (use @c __PRETTY_FUNCTION__ or @c __FUNCTION__ here)
		 */
		PppException( const std::string& msg, unsigned int lineno, const char function[] ) throw();
		/**
		 * @brief Constructor with additional information for Re-Throws
		 * @param[in] previous Previous PppException
		 * @param[in] lineno Line number (use @c __LINE__ here)
		 * @param[in] function Function (use @c __PRETTY_FUNCTION__ or @c __FUNCTION__ here)
		 * @see PPP_RETHROW
		 * @details
		 * Appends the parameters from this contructor to @a previous, giving the possibility
		 * to trace the exception
		 */
		PppException( const PppException& previous, unsigned int lineno, const char function[] ) throw();
		/**
		 * @brief Destructor, no operation
		 */
		virtual ~PppException() throw();
		/**
		 * @brief Get the exception message
		 * @return Message
		 */
		virtual const char* what() const throw();
};

/**
 * @brief Generator for @c std exceptions
 * @ingroup Common
 * @param[in] extype Name of the exception type in @c std to catch
 * @param[in] postcmd Optional commands to execute before throwing the PppException
 * @see PPP_CATCH_ALL
 */
#define PPP_CATCH_STD(extype, postcmd) catch(std::extype &e) { postcmd; throw PppException(std::string(#extype "[")+e.what()+"]",__LINE__,__PRETTY_FUNCTION__); }

/**
 * @brief Catch PeePeePlayer exceptions
 * @ingroup Common
 * @param[in] postcmd Instructions to execute before a ::PppException is thrown
 * @details
 * Catches general exceptions and throws a ::PppException for tracing the exception
 */
#define PPP_CATCH_ALL(postcmd) \
	catch(PppException &e) { postcmd; throw PppException(e,__LINE__,__PRETTY_FUNCTION__); } \
	PPP_CATCH_STD(bad_alloc, postcmd) PPP_CATCH_STD(bad_cast, postcmd) PPP_CATCH_STD(bad_exception, postcmd) PPP_CATCH_STD(bad_typeid, postcmd) \
	PPP_CATCH_STD(ios_base::failure, postcmd) PPP_CATCH_STD(domain_error, postcmd) PPP_CATCH_STD(invalid_argument, postcmd) \
	PPP_CATCH_STD(length_error, postcmd) PPP_CATCH_STD(out_of_range, postcmd) PPP_CATCH_STD(exception, postcmd) \
	catch(...) { postcmd; throw PppException("Unknown Exception",__LINE__,__PRETTY_FUNCTION__); }

/**
 * @brief Test & throw
 * @param[in] condition The condition to test
 * @details
 * Throws a ::PppException if @a condition is true, containing the condition in the message
 */
#define PPP_TEST(condition) if(condition) throw PppException("[PPP_TEST] " #condition,__LINE__,__PRETTY_FUNCTION__);

/**
 * @brief Catch and re-throw a PppException
 */
#define PPP_RETHROW() catch(PppException &e) { throw PppException(e,__LINE__,__PRETTY_FUNCTION__); }

/**
 * @brief Throw a PppException with position information
 * @param[in] msg Message passed to the constructor of ::PppException
 */
#define PPP_THROW(msg) throw PppException(msg,__LINE__,__PRETTY_FUNCTION__);

#endif
