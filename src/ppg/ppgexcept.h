/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Steffen Ohrendorf <steffen.ohrendorf@gmx.de>

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

#ifndef PPGEXCEPT_H
#define PPGEXCEPT_H

#include <iostream>
#include <typeinfo>
#include <exception>
#include <stdexcept>

namespace ppg {
	/**
	 * @class Exception
	 * @ingroup Ppg
	 * @brief A simple tracing exception for PeePeeGUI
	 */
	class Exception : public std::exception {
		private:
			std::string m_msg; //!< @brief Exception message
		public:
			/**
			* @brief Constructor with initial message
			* @param[in] msg Initial exception message
			 */
			Exception( const std::string& msg ) throw();
			/**
			 * @brief Constructor with additional information
			 * @param[in] msg Initial Message
			 * @param[in] file File name (use @c __BASE_FILE__ or @c __FILE__ here)
			 * @param[in] lineno Line number (use @c __LINE__ here)
			 * @param[in] function Function (use @c __PRETTY_FUNCTION__ or @c __FUNCTION__ here)
			 */
			Exception( const std::string& msg, int lineno, const char function[] ) throw();
			/**
			 * @brief Constructor with additional information for Re-Throws
			 * @param[in] previous Previous PppException
			 * @param[in] file File name (use @c __BASE_FILE__ or @c __FILE__ here)
			 * @param[in] lineno Line number (use @c __LINE__ here)
			 * @param[in] function Function (use @c __PRETTY_FUNCTION__ or @c __FUNCTION__ here)
			 * @see PPG_RETHROW
			 * @details
			 * Appends the parameters from this contructor to @a previous, giving the possibility
			 * to trace the exception
			 */
			Exception( const Exception& previous, int lineno, const char function[] ) throw();
			/**
			 * @brief Destructor, no operation
			 */
			virtual ~Exception() throw();
			/**
			 * @brief Get the exception message
			 * @return Message
			 */
			virtual const char* what() const throw();
	};

#define PPG_CATCH_STD(extype, postcmd) catch(std::extype &e) { postcmd; throw ppg::Exception(std::string(#extype "[")+e.what()+"]",__LINE__,__PRETTY_FUNCTION__); }

	/**
	 * @brief Catch PeePeeGUI exceptions
	 * @ingroup Ppg
	 * @details
	 * Catches general exceptions and throws a ppg::Exception for tracing the exception
	 */
#define PPG_CATCH_ALL(postcmd) \
	catch(ppg::Exception &e) { postcmd; throw ppg::Exception(e,__LINE__,__PRETTY_FUNCTION__); } \
	PPG_CATCH_STD(bad_alloc, postcmd) PPG_CATCH_STD(bad_cast, postcmd) PPG_CATCH_STD(bad_exception, postcmd) PPG_CATCH_STD(bad_typeid, postcmd) \
	PPG_CATCH_STD(ios_base::failure, postcmd) PPG_CATCH_STD(domain_error, postcmd) PPG_CATCH_STD(invalid_argument, postcmd) \
	PPG_CATCH_STD(length_error, postcmd) PPG_CATCH_STD(out_of_range, postcmd) PPG_CATCH_STD(exception, postcmd) \
	catch(...) { postcmd; throw ppg::Exception("Unknown Exception",__LINE__,__PRETTY_FUNCTION__); }

	/**
	 * @brief Test & throw
	 * @param[in] condition The condition to test
	 * @details
	 * Throws a ::PpgException if @a condition is true, containing the condition in the message
	 */
#define PPG_TEST(condition) if(condition) throw ppg::Exception("[PPG_TEST] " #condition,__LINE__,__PRETTY_FUNCTION__);

	/**
	 * @brief Catch and re-throw a ppg::Exception
	 */
#define PPG_RETHROW() \
	catch(ppg::Exception &e) { throw ppg::Exception(e,__LINE__,__PRETTY_FUNCTION__); }

	/**
	 * @brief Throw a ppg::Exception with position information
	 * @param[in] msg Message passed to the constructor of the ppg::Exception
	 */
#define PPG_THROW(msg) throw ppg::Exception(msg,__LINE__,__PRETTY_FUNCTION__);

} // namespace ppg

#endif
