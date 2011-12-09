cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0015 NEW)

if(MINGW)
	set(CMAKE_RC_COMPILER_INIT windres)
	ENABLE_LANGUAGE(RC)
	SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>")
	PROJECT(PeePeePlayer CXX C RC)
else()
	PROJECT(PeePeePlayer CXX C)
endif(MINGW)

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	message(FATAL_ERROR "This is a big-endian machine. Big Endian is not supported, sorry.")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE _gcc_version)
	string(STRIP ${_gcc_version} _gcc_version)
	message(STATUS "Your GCC Version is ${_gcc_version}, required is 4.6.0")
	if(${_gcc_version} VERSION_LESS "4.6.0")
		message(FATAL_ERROR "Please update your GCC to at least version 4.6.0")
	endif()
else()
	message(FATAL_ERROR "Sorry, you need at least GCC 4.6.0")
endif()

if(NOT __COMPILER_GNU)
	message(WARNING "Manually including GCC configuration")
	include(Compiler/GNU)
	__compiler_gnu(CXX)
endif()

INCLUDE(InstallRequiredSystemLibraries)
#########
# configure CPACK
#########
SET(CPACK_PACKAGE_NAME "PeePeePlayer")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Retro-style module player")
SET(CPACK_PACKAGE_VENDOR "Steffen Ohrendorf <steffen.ohrendorf@gmx.de>")
# SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "3-wip")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_STRIP_FILES "bin/ppplay")
SET(CPACK_SOURCE_STRIP_FILES "")
SET(CPACK_SOURCE_IGNORE_FILES "/\\\\..;/build/;.~$;.\\\\.kdev4;Doxyfile$;config\\\\.h$")
SET(CPACK_PACKAGE_EXECUTABLES "ppplay" "PeePeePlayer")
SET(CPACK_MONOLITHIC_INSTALL TRUE)
# SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
# SET(CPACK_PACKAGE_SOURCE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src")

if(LINUX OR UNIX)
	SET(CPACK_BINARY_RPM ON)
	SET(CPACK_RPM_PACKAGE_RELEASE "1")
	SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
	SET(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
	SET(CPACK_RPM_PACKAGE_GROUP "Productivity/Multimedia/Sound/Players")
	SET(CPACK_RPM_PACKAGE_DESCRIPTION
"PeePeePlayer aims to be an \\\"original\\\" sounding, DOS-like looking Music Tracker Modules player.

Authors:
--------
    Steffen Ohrendorf <steffen.ohrendorf@gmx.de>"
)
endif()

INCLUDE(CPack)

#########
# setting compiler flags
#########
set(CMAKE_CXX_FLAGS "-Wall -Weffc++ -Wextra -Wold-style-cast -Woverloaded-virtual -pedantic -std=c++0x")
option(WITH_PROFILING "Whether to enable profiling (-pg)" OFF)
if(WITH_PROFILING)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
	message(WARNING "You have enabled profiling. This will decrease performance and should be disabled when building for distribution.")
endif()

option(WITH_NATIVE_MARCH "Sets -march=native (WARNING: Do not use when distributing binaries!)" OFF)
if(WITH_NATIVE_MARCH)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
	message(WARNING "You have enabled a Native Build (-march=native). This will most likely ONLY work on YOUR machine!")
endif()

option(WITH_LTO "Enables link-time optimization" OFF)
if(WITH_LTO)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
	message(STATUS "Link-time optimization (-flto) enabled")
endif()

add_definitions(-DBOOST_FILESYSTEM_VERSION=3)

if(MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
endif()

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Build type is undefined, assuming 'RelWithDebInfo'")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" FORCE)
else()
	message(STATUS "Build type is '${CMAKE_BUILD_TYPE}'")
endif()

message(STATUS "CXX Flags (Debug)       : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX Flags (Release)     : ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CXX Flags (Min. Size)   : ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message(STATUS "CXX Flags (Rel. w. Dbg.): ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CXX Flags (Additional)  : ${CMAKE_CXX_FLAGS}")

include(FeatureSummary)
#########
# init main application target settings
#########
set(ppp_SRCS ppplay.cpp src/ui_main.cpp src/sdlloader.cpp)
if(WIN32 OR MINGW)
	set(ppp_SRCS ${ppp_SRCS} ppp.rc)
endif()
ADD_EXECUTABLE(ppplay ${ppp_SRCS} config.h)

if(MINGW)
	target_link_libraries(ppplay stdc++ mingw32)
endif()

target_link_libraries(ppplay rt)
target_link_libraries(ppplay modmod xmmod s3mmod genmod audioout)
target_link_libraries(ppplay utils binstream light4cxx)
target_link_libraries(ppplay ppg timer)

#########
# link libraries
#########
function(trylinklib LIBNAME ATTRIBS)
	find_library(
		"${LIBNAME}_LIB"
		NAMES "${LIBNAME}"
		HINTS
		PATH_SUFFIXES lib64 lib
		PATHS
		/sw
		/opt/local
		/opt/csw
		/opt
	)
	if(NOT "${LIBNAME}_LIB")
		# message(STATUS "${LIBNAME} Library not found")
	else(NOT "${LIBNAME}_LIB")
		# message(STATUS "${LIBNAME} Library found: ${${LIBNAME}_LIB}")
		add_library("${LIBNAME}" ${ATTRIBS} IMPORTED)
		target_link_libraries(ppplay ${${LIBNAME}_LIB})
	endif(NOT "${LIBNAME}_LIB")
	mark_as_advanced(FORCE "${LIBNAME}_LIB")
endfunction(trylinklib)
trylinklib("mp3lame" SHARED)
if(NOT mp3lame_LIB)
	option(WITH_MP3LAME "Whether to include MP3Lame support." OFF)
	# message(STATUS "MP3Lame _not_ found, Quick-MP3 will not be available")
else()
	option(WITH_MP3LAME "Whether to include MP3Lame support." ON)
	# message(STATUS "MP3Lame found, Quick-MP3 will be available")
endif()

add_feature_info(MP3Lame WITH_MP3LAME "The MP3Lame library enables MP3 export.")

#########
# add SDL
#########
find_package(SDL REQUIRED)
if(NOT SDL_FOUND)
	message(FATAL_ERROR "SDL not found")
endif()
set_package_info(SDL "Simple DirectMedia Layer" "http://www.libsdl.org" "The SDL library is used for audio and video output.")
include_directories(${SDL_INCLUDE_DIR})
target_link_libraries(ppplay ${SDL_LIBRARY})

set(Boost_USE_STATIC_LIBS FALSE)
set(Boost_USE_MULTITHREADED TRUE)
include(FindBoost)
find_package(Boost COMPONENTS program_options filesystem system)
if(Boost_FOUND)
	#message(STATUS "Boost Include Dirs: ${Boost_INCLUDE_DIRS}")
	#message(STATUS "Boost Libraries: ${Boost_LIBRARIES}")
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(ppplay ${Boost_LIBRARIES})
else()
	message(FATAL_ERROR "Boost Libraries not found!")
endif()
set_package_info(Boost "Boost libraries" "http://www.boost.org")
set_package_info(program_options "Boost program_options library" "http://www.boost.org" "The Boost program_options library is used to parse command line arguments.")
set_package_info(filesystem "Boost filesystem library" "http://www.boost.org" "The Boost filesystem library is used to handle file paths and information.")
set_package_info(system "Boost system library" "http://www.boost.org" "The Boost system library handles system dependent things.")

include_directories(src)
add_subdirectory(src)

#########
# set install command
#########
install(TARGETS ppplay RUNTIME DESTINATION bin COMPONENT application)
cpack_add_component(application DISPLAY_NAME PeePeePlayer REQUIRED)
if(LINUX OR UNIX)
	install(FILES peepeeplayer.desktop DESTINATION share/applications COMPONENT desktopfile)
	cpack_add_component(desktopfile DISPLAY_NAME .desktop file REQUIRED)
endif()

#########
# add docs target & configuration
#########
include(FindDoxygen)
if(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
	add_custom_target(
		docs
		COMMAND doxygen "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMENT "Building the Doxygen docs..."
		SOURCES Doxyfile
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
	)
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
		"${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
		ESCAPE_QUOTES
	)
endif()

add_feature_info(Doxygen DOXYGEN_FOUND "Doxygen is used to generate the API documentation.")

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h"
	ESCAPE_QUOTES
)

set_package_info(Threads "Threads library")

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
feature_summary(WHAT PACKAGES_FOUND DESCRIPTION "Found packages:")

