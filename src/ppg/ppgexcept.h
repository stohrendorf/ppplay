/***************************************************************************
 *   Copyright (C) 2009 by Syron                                         *
 *   mr.syron@gmail.com                                                    *
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

#ifndef ppgexceptH
#define ppgexceptH

#include <iostream>
#include <typeinfo>
#include <exception>
#include <stdexcept>

/**
 * @ingroup Ppg
 * @brief A simple tracing exception for PeePeeGUI
 */
class PpgException : public std::exception {
	private:
		std::string m_msg; //!< @brief Exception message
	public:
		/**
		* @brief Constructor with initial message
		* @param[in] msg Initial exception message
		 */
		PpgException(const std::string &msg) throw();
		/**
		 * @brief Constructor with additional information
		 * @param[in] msg Initial Message
		 * @param[in] file File name (use @c __BASE_FILE__ or @c __FILE__ here)
		 * @param[in] lineno Line number (use @c __LINE__ here)
		 * @param[in] function Function (use @c __PRETTY_FUNCTION__ or @c __FUNCTION__ here)
		 */
		PpgException(const std::string &msg, const char file[], const int lineno, const char function[]) throw();
		/**
		 * @brief Constructor with additional information for Re-Throws
		 * @param[in] previous Previous PppException
		 * @param[in] file File name (use @c __BASE_FILE__ or @c __FILE__ here)
		 * @param[in] lineno Line number (use @c __LINE__ here)
		 * @param[in] function Function (use @c __PRETTY_FUNCTION__ or @c __FUNCTION__ here)
		 * @see PPP_RETHROW
		 * @details
		 * Appends the parameters from this contructor to @a previous, giving the possibility
		 * to trace the exception
		 */
		PpgException(const PpgException &previous, const char file[], const int lineno, const char function[]) throw();
		/**
		 * @brief Destructor, no operation
		 */
		virtual ~PpgException() throw();
		/**
		 * @brief Get the exception message
		 * @return Message
		 */
		virtual const char *what() const throw();
};

/**
 * @brief Catch PeePeeGUI exceptions
 * @ingroup Ppg
 * @details
 * Catches general exceptions and throws a ::PpgException for tracing the exception
 */
#define PPG_CATCH_ALL() \
	catch(PpgException &e) { throw PpgException(e,__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::bad_alloc &e) { throw PpgException(std::string("bad_alloc[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::bad_cast &e) { throw PpgException(std::string("bad_cast[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::bad_exception &e) { throw PpgException(std::string("bad_exception[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::bad_typeid &e) { throw PpgException(std::string("bad_typeid[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::ios_base::failure &e) { throw PpgException(std::string("ios_base::failure[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::domain_error &e) { throw PpgException(std::string("domain_error[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::invalid_argument &e) { throw PpgException(std::string("invalid_argument[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::length_error &e) { throw PpgException(std::string("length_error[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::out_of_range &e) { throw PpgException(std::string("out_of_range[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(std::exception &e) { throw PpgException(std::string("exception[")+e.what()+"]",__FILE__,__LINE__,__PRETTY_FUNCTION__); } \
	catch(...) { throw PpgException("Unknown Exception",__FILE__,__LINE__,__PRETTY_FUNCTION__); }

/**
 * @brief Test & throw
 * @param[in] condition The condition to test
 * @details
 * Throws a ::PpgException if @a condition is true, containing the condition in the message
 */
#define PPG_TEST(condition) if(condition) throw PpgException("[PPP_TEST] " #condition,__FILE__,__LINE__,__PRETTY_FUNCTION__);

/**
 * @brief Catch and re-throw a PpgException
 */
#define PPG_RETHROW() \
	catch(PppException &e) { throw PpgException(e,__FILE__,__LINE__,__PRETTY_FUNCTION__); }

/**
 * @brief Throw a PpgException with position information
 * @param[in] msg Message passed to the constructor of the PpgException
 */
#define PPG_THROW(msg) throw PpgException(msg,__FILE__,__LINE__,__PRETTY_FUNCTION__);

#endif
