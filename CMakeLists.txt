cmake_minimum_required( VERSION 3.12 )
if( NOT VCPKG )
    set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}" )
endif()

if( MINGW )
    set( CMAKE_RC_COMPILER_INIT windres )
    ENABLE_LANGUAGE( RC )
    SET( CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -O coff -o <OBJECT> <SOURCE>" )
    PROJECT( PPPlay CXX C RC )
    if( CMAKE_SIZEOF_VOID_P MATCHES "8" )
        add_definitions( -DBOOST_USE_WINDOWS_H )
    endif()
else()
    PROJECT( PPPlay CXX C )
endif( MINGW )

if( WIN32 )
    add_definitions( -DWIN32 )
endif()

INCLUDE( TestBigEndian )
TEST_BIG_ENDIAN( IS_BIG_ENDIAN )
if( IS_BIG_ENDIAN )
    message( FATAL_ERROR "This is a big-endian machine. Big Endian is not supported, sorry." )
endif()

string( FIND "${CMAKE_CXX_COMPILER}" "clang" COMPILER_IS_CLANG )
if( COMPILER_IS_CLANG EQUAL "-1" )
    set( COMPILER_IS_CLANG FALSE )
else()
    set( COMPILER_IS_CLANG TRUE )
endif()
if( COMPILER_IS_CLANG )
    message( STATUS "Detected clang compiler" )
elseif( CMAKE_COMPILER_IS_GNUCXX )
    message( STATUS "Your GCC Version is ${CMAKE_CXX_COMPILER_VERSION}, required is 4.8.2" )
    if( ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.8.2" )
        message( FATAL_ERROR "Please update your GCC to at least version 4.8.2" )
    elseif( ${CMAKE_CXX_COMPILER_VERSION} VERSION_EQUAL "4.8.2" OR ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.8.2" )
        option( SANITIZE_ADDRESS "Use address sanitization" OFF )
        option( SANITIZE_THREAD "Use thread sanitization" OFF )
        if( SANITIZE_ADDRESS AND SANITIZE_THREAD )
            message( WARNING "You cannot use SANITIZE_ADDRESS and SANITIZE_THREAD at the same time. SANITIZE_ADDRESS will be used only." )
        endif()
        if( SANITIZE_ADDRESS )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer" )
            set( SANITIZE_THREAD OFF )
        elseif( SANITIZE_THREAD )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread" )
        endif()
    endif()
elseif( MSVC )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251" )
endif()

INCLUDE( InstallRequiredSystemLibraries )

#########
# configure CPACK
#########
SET( CPACK_PACKAGE_NAME "PPPlay" )
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Retro-style module player" )
SET( CPACK_PACKAGE_VENDOR "Steffen Ohrendorf <steffen.ohrendorf@gmx.de>" )
# SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/gpl-3.0.txt" )
SET( CPACK_PACKAGE_VERSION_MAJOR "0" )
SET( CPACK_PACKAGE_VERSION_MINOR "2" )
SET( CPACK_PACKAGE_VERSION_PATCH "0-alpha0" )
SET( CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}" )
SET( CPACK_STRIP_FILES TRUE )
SET( CPACK_SOURCE_STRIP_FILES "" )
SET( CPACK_SOURCE_IGNORE_FILES "/\\\\..;/build/;.~$;.\\\\.kdev4;Doxyfile$;config\\\\.h$" )
SET( CPACK_PACKAGE_EXECUTABLES "ppplay" "PPPlay" )
SET( CPACK_MONOLITHIC_INSTALL TRUE )
# SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
# SET(CPACK_PACKAGE_SOURCE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src")

if( LINUX OR UNIX )
    SET( CPACK_BINARY_RPM ON )
    SET( CPACK_RPM_PACKAGE_RELEASE "1" )
    SET( CPACK_RPM_PACKAGE_LICENSE "GPLv3" )
    SET( CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )
    SET( CPACK_RPM_PACKAGE_GROUP "Productivity/Multimedia/Sound/Players" )
    SET( CPACK_RPM_PACKAGE_DESCRIPTION
         "PPPlay aims to be an \\\"original\\\" sounding, DOS-like looking Music Tracker Modules player.

Authors:
--------
    Steffen Ohrendorf <steffen.ohrendorf@gmx.de>"
         )
endif()

INCLUDE( CPack )

#########
# setting compiler flags
#########
if( CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG )
    if( NOT WIN32 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Weffc++ -Wextra -Wold-style-cast -Woverloaded-virtual -pedantic -std=c++14" )
    else()
        # Boost produces massive amounts of warnings on Windows, as it's most likely not within the compiler's system path.
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -std=c++14" )
    endif()

    option( WITH_PROFILING "Whether to enable profiling (-pg)" OFF )
    if( WITH_PROFILING )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg" )
        message( WARNING "You have enabled profiling. This will decrease performance and should be disabled when building for distribution." )
    endif()

    option( WITH_NATIVE_MARCH "Sets -march=native (WARNING: Do not use when distributing binaries!)" OFF )
    if( WITH_NATIVE_MARCH )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native" )
        message( WARNING "You have enabled a Native Build (-march=native). This will most likely ONLY work on YOUR machine!" )
    endif()

    option( WITH_LTO "Enables link-time optimization" OFF )
    if( WITH_LTO )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto" )
        message( STATUS "Link-time optimization (-flto) enabled" )
    endif()
endif()

if( MINGW )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows" )
endif()

#########
# add SDL
#########
find_package( SDL2 REQUIRED )

#set(Boost_USE_STATIC_LIBS FALSE)
set( Boost_USE_MULTITHREADED TRUE )
add_definitions( -DBOOST_ALL_NO_LIB -DBOOST_ALL_DYN_LINK )
find_package( Boost COMPONENTS program_options filesystem system unit_test_framework serialization REQUIRED )

include_directories( src )
enable_testing()
add_subdirectory( src )


#########
# init main application target settings
#########
set( ppp_SRCS ppplay.cpp src/ui_main.cpp )
if( WIN32 OR MINGW )
    set( ppp_SRCS ${ppp_SRCS} ppplay.rc )
endif()
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/src )
ADD_EXECUTABLE( ppplay ${ppp_SRCS} config.h )

if( MINGW )
    target_link_libraries( ppplay stdc++ mingw32 )
elseif( COMPILER_IS_CLANG )
    target_link_libraries( ppplay stdc++ )
endif()

target_link_libraries( ppplay ppplay_core ppplay_module_base ppplay_ppg ppplay_output_sdl ppplay_output_wav Boost::program_options ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} )

#########
# link libraries
#########
function( trylinklib LIBNAME )
    find_library(
            "${LIBNAME}_LIB"
            NAMES "${LIBNAME}"
            HINTS
            PATH_SUFFIXES lib64 lib32 lib
            PATHS
            /sw
            /opt/local
            /opt/csw
            /opt
    )
    if( ${LIBNAME}_LIB )
        add_library( "${LIBNAME}" INTERFACE )
        target_link_libraries( "${LIBNAME}" INTERFACE "${${LIBNAME}_LIB}" )
    endif()
    mark_as_advanced( FORCE "${LIBNAME}_LIB" )
endfunction( trylinklib )

trylinklib( "mp3lame" )
if( mp3lame_LIB )
    option( WITH_MP3LAME "Whether to include MP3Lame support." ON )
endif()
if( WITH_MP3LAME )
    target_link_libraries( ppplay ppplay_output_mp3 )
endif()

trylinklib( "vorbis" )
trylinklib( "vorbisenc" )
trylinklib( "ogg" )
if( vorbis_LIB AND vorbisenc_LIB AND ogg_LIB )
    option( WITH_OGG "Whether to include OGG support." ON )
endif()
if( WITH_OGG )
    target_link_libraries( ppplay ppplay_output_ogg )
endif()

target_link_libraries( ppplay ppplay_input_it ppplay_input_hsc ppplay_input_s3m ppplay_input_mod ppplay_input_xm )

#########
# set install command
#########
install( TARGETS ppplay DESTINATION bin COMPONENT application )
cpack_add_component( application DISPLAY_NAME PPPlay REQUIRED )
if( LINUX OR UNIX )
    install( FILES data/ppplay-hsc.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages )
    install( CODE "execute_process( COMMAND update-mime-database ${CMAKE_INSTALL_FULL_DATAROOTDIR}/mime )" )
    install( FILES ppplay.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )
    install( CODE "execute_process( COMMAND update-desktop-database -q )" )
endif()

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
        ESCAPE_QUOTES
)
