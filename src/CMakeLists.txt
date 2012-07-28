include_directories(.)

add_subdirectory(stuff)
add_subdirectory(output)

add_subdirectory(stream)

add_subdirectory(ppg)

add_subdirectory(genmod)
add_subdirectory(s3mmod)
add_subdirectory(xmmod)
add_subdirectory(modmod)

add_subdirectory(light4cxx)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/Mainpage.dox.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/Mainpage.dox"
	ESCAPE_QUOTES
)

set_source_files_properties(Mainpage.dox PROPERTIES GENERATED TRUE)

include(FindLibArchive)
find_library(LibArchive REQUIRED)
include_directories(${LibArchive_INCLUDE_DIR})

add_library(ppplay_stream SHARED
	stream/stream.cpp
	stream/filestream.cpp
	stream/abstractarchive.cpp
	stream/memarchive.cpp
	stream/memorystream.cpp
	stream/archivefilestream.cpp
)
target_link_libraries(ppplay_stream ${LibArchive_LIBRARY})
install(TARGETS ppplay_stream DESTINATION lib)

add_library(ppplay_module_base SHARED
	genmod/breseninter.cpp
	genmod/genbase.cpp
	genmod/abstractchannel.cpp
	genmod/abstractmodule.cpp
	genmod/abstractorder.cpp
	genmod/sample.cpp
	genmod/ipatterncell.cpp
	genmod/modulestate.cpp
	genmod/songinfo.cpp
)
install(TARGETS ppplay_module_base DESTINATION lib)

add_library(ppplay_core SHARED
	stuff/pluginregistry.cpp
	stuff/numberutils.cpp
	stuff/sdltimer.cpp
	stuff/stringutils.cpp
	output/audiofifo.cpp
	output/audiotypes.cpp
	output/abstractaudiooutput.cpp
	output/abstractaudiosource.cpp
	light4cxx/location.cpp
	light4cxx/logger.cpp
)

target_link_libraries(ppplay_core ppplay_stream ${SDL_LIBRARY} ${Boost_LIBRARIES} ${mp3lame_LIB})
install(TARGETS ppplay_core DESTINATION lib)
