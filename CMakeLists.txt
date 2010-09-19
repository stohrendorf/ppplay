PROJECT(PeePeePlayer CXX C)

cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0015 NEW)

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	message(FATAL_ERROR "This is a big-endian machine. Big Endian is not supported, sorry.")
endif()

INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES(
	"#if (__GNUC__*100)+__GNUC_MINOR__ < 405 \n#error \"GCC Version < 4.5.0\" \n#endif \nint main() { return 0; } \n"
	IS_GCC_VERSION_45
)
if(NOT IS_GCC_VERSION_45)
	message(FATAL_ERROR "Sorry, you need at least GCC 4.5.")
endif()

INCLUDE(InstallRequiredSystemLibraries)
#########
# configure CPACK
#########
SET(CPACK_PACKAGE_NAME "PeePeePlayer")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Retro-style module player")
SET(CPACK_PACKAGE_VENDOR "Syron <mr.syron@googlemail.com>")
# SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_STRIP_FILES "bin/ppp")
SET(CPACK_SOURCE_STRIP_FILES "")
SET(CPACK_SOURCE_IGNORE_FILES "/\\\\..;/build/;.~$;.\\\\.kdev4;Doxyfile$;config\\\\.h$")
SET(CPACK_PACKAGE_EXECUTABLES "ppp" "PeePeePlayer")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
SET(CPACK_PACKAGE_SOURCE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src")

SET(CPACK_BINARY_RPM "ON")
SET(CPACK_RPM_PACKAGE_RELEASE "1")
SET(CPACK_RPM_PACKAGE_LICENSE "LGPLv3")
SET(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET(CPACK_RPM_PACKAGE_GROUP "Productivity/Multimedia/Sound/Players")
SET(CPACK_RPM_PACKAGE_DESCRIPTION
"PeePeePlayer aims to be an \\\"original\\\" sounding, DOS-like looking Music Tracker Modules player.

Authors:
--------
    Syron <mr.syron@googlemail.com>"
)

INCLUDE(CPack)

#########
# setting compiler flags
#########
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Weffc++ -Wextra -Wold-style-cast -Woverloaded-virtual -pedantic -std=c++0x")
if(MINGW OR WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
endif()
# set(CMAKE_CXX_FLAGS_DEBUG   "-g ${CMAKE_CXX_FLAGS}")
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -mtune=native -march=native ${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "profilegenerate")
	set(CMAKE_CXX_FLAGS "-fprofile-generate -O3 ${CMAKE_CXX_FLAGS}")
	set(LINK_FLAGS "-fprofile-generate")
	message(STATUS "Profiling enabled - after test run, recompile with CMAKE_BUILD_TYPE=profileuse")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "profileuse")
	set(CMAKE_CXX_FLAGS "-fprofile-use -O3 ${CMAKE_CXX_FLAGS}")
	set(LINK_FLAGS "-fprofile-use")
	message(STATUS "Profile data usage")
endif()

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Build type is undefined, assuming 'Debug'")
	set(CMAKE_BUILD_TYPE "Debug")
else()
	message(STATUS "Build type is '${CMAKE_BUILD_TYPE}'")
endif()
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX Flags (Debug): ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX Flags (Release): ${CMAKE_CXX_FLAGS_RELEASE}")

#########
# init main application target settings
#########
ADD_EXECUTABLE(ppp ppp.cpp config.h)
target_link_libraries(ppp stdc++)

if(MINGW OR WIN32)
	target_link_libraries(ppp mingw32)
endif()

add_dependencies(ppp logger genmod s3mmod
	#stmmod
	#modmod
	ppg binstream phaser fifoout fft utils)
target_link_libraries(ppp logger genmod s3mmod
	#stmmod
	#modmod
	ppg binstream phaser fifoout fft utils)

#########
# add SDL
#########
#INCLUDE(FindSDL)
#if(NOT SDL_FOUND OR NOT SDLMAIN_LIBRARY)
#	message(STATUS "${SDL_INCLUDE_DIR}")
#	message(FATAL_ERROR "Could not find the SDL libraries")
#endif()
#message(STATUS "SDL Main Library: ${SDLMAIN_LIBRARY}")
#add_library(SDL SHARED IMPORTED)
#target_link_libraries(ppp ${SDL_LIBRARY})

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
		message(STATUS "${LIBNAME} Library not found")
	else(NOT "${LIBNAME}_LIB")
		message(STATUS "${LIBNAME} Library found: ${${LIBNAME}_LIB}")
		add_library("${LIBNAME}" ${ATTRIBS} IMPORTED)
		target_link_libraries(ppp ${${LIBNAME}_LIB})
	endif(NOT "${LIBNAME}_LIB")
endfunction(trylinklib)
trylinklib("mp3lame" SHARED)
if(NOT mp3lame_LIB)
	set(WITH_MP3LAME FALSE)
	message(STATUS "MP3Lame _not_ found, Quick-MP3 will not be available")
else()
	set(WITH_MP3LAME TRUE)
	message(STATUS "MP3Lame found, Quick-MP3 will be available")
endif()

find_package(SDL REQUIRED)
if(NOT SDL_FOUND)
	message(FATAL_ERROR "SDL not found")
endif()
include_directories(${SDL_INCLUDE_DIR})
if(MINGW OR WIN32)
	target_link_libraries(ppp mingw32)
endif()
target_link_libraries(ppp ${SDL_LIBRARY})

set(Boost_USE_STATIC_LIBS FALSE)
set(Boost_USE_MULTITHREADED TRUE)
set(Boost_ADDITIONAL_VERSIONS "1.44")
include(FindBoost)
find_package(Boost COMPONENTS program_options system)
if(Boost_FOUND)
	message(STATUS "Boost Include Dirs: ${Boost_INCLUDE_DIRS}")
	message(STATUS "Boost Libraries: ${Boost_LIBRARIES}")
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(ppp ${Boost_LIBRARIES})
else()
	message(FATAL_ERROR "Boost Libraries not found!")
endif()

#########
# add link/include dirs
#########
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	src/logger
	src/genmod
	src/genmod/phaser
	src/s3mmod
	#src/stmmod
	#src/modmod
	src/ppg
	src/stream
	src/output
	src/stuff
)

add_subdirectory(src/logger)
add_subdirectory(src/genmod)
#add_subdirectory(src/genmod/phaser)
add_subdirectory(src/s3mmod)
#add_subdirectory(src/stmmod)
#add_subdirectory(src/modmod)
add_subdirectory(src/stream)
add_subdirectory(src/ppg)
add_subdirectory(src/output)
add_subdirectory(src/stuff)

#########
# set install command
#########
install(TARGETS ppp RUNTIME DESTINATION bin COMPONENT application)
cpack_add_component(application DISPLAY_NAME PeePeePlayer REQUIRED)
if(NOT MINGW AND NOT WIN32)
	install(FILES peepeeplayer.desktop DESTINATION share/applications COMPONENT desktopfile)
	cpack_add_component(desktopfile DISPLAY_NAME .desktop file REQUIRED)
endif()

#########
# add docs target & configuration
#########
add_custom_target(docs
	COMMAND doxygen "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMENT "Building the Doxygen docs..."
	SOURCES Doxyfile
)
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
	ESCAPE_QUOTES
)
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h"
	ESCAPE_QUOTES
)
