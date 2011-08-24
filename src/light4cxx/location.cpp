#include "location.h"

#include "level.h"
#include "logger.h"

#include <stdexcept>
#include <sstream>

namespace light4cxx {

static std::string s_format = "[%T %-5t] %L: %m%n";

static std::string levelString(Level l)
{
	switch(l) {
		case Level::Off:
			return std::string();
		case Level::Trace:
			return "TRACE";
		case Level::Debug:
			return "DEBUG";
		case Level::Info:
			return "INFO";
		case Level::Warn:
			return "WARN";
		case Level::Error:
			return "ERROR";
		case Level::All:
			throw std::runtime_error("Logging level invalid: Level::All should not be passed to levelString()");
		default:
			throw std::runtime_error("Logging level invalid: Unknown Level passed to levelString()");
	}
}

void Location::setFormat(const std::string& fmt)
{
	s_format = fmt;
}

std::string Location::toString(light4cxx::Level l, const light4cxx::Logger& logger, const std::string& msg) const
{
	std::ostringstream oss;
	const std::ios_base::fmtflags clearFlags = oss.flags();
	int state = 0;
	for(int i=0; i<s_format.length(); i++) {
		char c = s_format.at(i);
		switch(state) {
			case 0:
				// scan for %
				if(c=='%') {
					state = 1;
					break;
				}
				oss << c;
				break;
			case 1:
				// flags
				switch(c) {
					case '#':
						oss << std::showbase;
						break;
					case '0':
						oss.fill('0');
						break;
					case '-':
						oss << std::left;
						break;
					case '+':
						oss << std::showpos;
						break;
					default:
						state = 2;
						i--;
						break;
				}
				break;
			case 2:
				// field width
				if(isdigit(c)) {
					oss.width( c-'0' );
				}
				else {
					i--;
				}
				state = 3;
				break;
			case 3:
				// precision dot
				if(c=='.') {
					state = 4;
				}
				else {
					// no precision
					state = 5;
					i--;
				}
				break;
			case 4:
				// precision value
				if(isdigit(c)) {
					oss.precision(c - '0');
				}
				else {
					oss.precision(0);
					i--;
				}
				state = 5;
				break;
			case 5:
				// format specifier
				switch(c) {
					case '%':
						oss << '%';
						break;
					case 'f':
						oss << m_function;
						break;
					case 'F':
						oss << m_file;
						break;
					case 'l':
						oss << m_line;
						break;
					case 'L':
						oss << logger.name();
						break;
					case 'm':
						oss << msg;
						break;
					case 'n':
						oss << std::endl;
						break;
					case 't':
						oss << levelString(l);
						break;
					case 'T':
						oss << std::hex << m_threadId;
						break;
					default:
						throw std::runtime_error( std::string("Unknown format specifier: ") + s_format.substr(i) );
				}
				state = 0;
				oss.flags( clearFlags );
				break;
			default:
				throw std::runtime_error("Invalid format parsing state");
		}
	}
	return oss.str();
}

}