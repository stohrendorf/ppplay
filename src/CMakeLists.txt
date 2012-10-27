include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(output)

add_subdirectory(stream)

add_subdirectory(ppg)

add_subdirectory(genmod)
add_subdirectory(s3mmod)
add_subdirectory(xmmod)
add_subdirectory(modmod)

add_subdirectory(ymf262)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/Mainpage.dox.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/Mainpage.dox"
	ESCAPE_QUOTES
)

set_source_files_properties(Mainpage.dox PROPERTIES GENERATED TRUE)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(WIN32)
	add_definitions(-DL4CXX_NO_ANSI_COLORS)
endif()

include(GenerateExportHeader)
add_compiler_export_flags()

add_library(ppplay_core SHARED
	stuff/pluginregistry.cpp
	stuff/numberutils.cpp
	stuff/sdltimer.cpp
	stuff/stringutils.cpp
	output/audiofifo.cpp
	output/abstractaudiooutput.cpp
	output/abstractaudiosource.cpp
	light4cxx/location.cpp
	light4cxx/logger.cpp
)
generate_export_header(ppplay_core)

target_link_libraries(ppplay_core ppplay_stream ${SDL_LIBRARY} ${Boost_LIBRARIES})
if(WITH_MP3LAME)
	target_link_libraries(ppplay_core ${mp3lame_LIB})
endif()
install(TARGETS ppplay_core DESTINATION lib EXPORT ppplayExports)
install(FILES
	stuff/field.h
	stuff/inputplugin.h
	stuff/itimer.h
	stuff/numberutils.h
	stuff/stringutils.h
	stuff/trackingcontainer.h
	stuff/utils.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_core_export.h
	DESTINATION include/ppplay/stuff
	COMPONENT Devel
)
install(FILES
	output/abstractaudiooutput.h
	output/abstractaudiosource.h
	output/audiofifo.h
	output/audiotypes.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_core_export.h
	DESTINATION include/ppplay/output
	COMPONENT Devel
)
install(FILES
	light4cxx/level.h
	light4cxx/location.h
	light4cxx/logger.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_core_export.h
	DESTINATION include/ppplay/light4cxx
	COMPONENT Devel
)
