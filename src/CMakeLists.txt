include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(light4cxx)
add_subdirectory(output)

add_subdirectory(stream)
add_subdirectory(stuff)

add_subdirectory(ppg)

add_subdirectory(genmod)
add_subdirectory(s3mmod)
add_subdirectory(xmmod)
add_subdirectory(modmod)
add_subdirectory(hscmod)

add_subdirectory(ymf262)

option(BUILD_BADPLAY "Build the BAdPlay player" ON)
if(BUILD_BADPLAY)
    add_subdirectory(adplug)
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/Mainpage.dox.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/Mainpage.dox"
	ESCAPE_QUOTES
)

set_source_files_properties(Mainpage.dox PROPERTIES GENERATED TRUE)

include(GenerateExportHeader)
add_compiler_export_flags()

set_source_files_properties(stuff/pluginregistry.cpp PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS};LIBEXECDIR=\"${CMAKE_INSTALL_FULL_LIBEXECDIR}\"")

add_library(ppplay_core SHARED
    stuff/pluginregistry.cpp
    stuff/numberutils.cpp
    stuff/sdltimer.cpp
    stuff/system.cpp
    stuff/pluginregistry.h
    stuff/numberutils.h
    stuff/sdltimer.h
    stuff/system.h
    stuff/field.h
    stuff/inputplugin.h
    stuff/itimer.h
    stuff/numberutils.h
    stuff/stringutils.h
    stuff/trackingcontainer.h
    stuff/utils.h
    output/audiofifo.cpp
    output/abstractaudiooutput.cpp
    output/abstractaudiosource.cpp
    output/fft.cpp
    output/fftobserver.cpp
    output/volumeobserver.cpp
    output/audiofifo.h
    output/abstractaudiooutput.h
    output/abstractaudiosource.h
    output/fft.h
    output/fftobserver.h
    output/volumeobserver.h
)
generate_export_header(ppplay_core)
set_target_properties(ppplay_core PROPERTIES SOVERSION 1)

target_link_libraries(ppplay_core ppplay_stream ${SDL2_LIBRARY} ${Boost_SYSTEM_LIBRARY})
if(WITH_MP3LAME)
	target_link_libraries(ppplay_core ${mp3lame_LIB})
endif()
install(TARGETS ppplay_core DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT ppplayExports)
install(FILES
	stuff/field.h
	stuff/inputplugin.h
	stuff/itimer.h
	stuff/numberutils.h
	stuff/stringutils.h
	stuff/trackingcontainer.h
	stuff/utils.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_core_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/stuff
	COMPONENT Devel
)
install(FILES
	output/abstractaudiooutput.h
	output/abstractaudiosource.h
	output/audiofifo.h
	output/audiotypes.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_core_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/output
	COMPONENT Devel
)
