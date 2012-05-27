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

add_library(ppplay_core SHARED
	stuff/moduleregistry.cpp
	stuff/numberutils.cpp
	stuff/sdltimer.cpp
	stuff/stringutils.cpp
	stream/binstream.cpp
	stream/fbinstream.cpp
	stream/iarchive.cpp
	stream/memarchive.cpp
	stream/sbinstream.cpp
	output/audiofifo.cpp
	output/audiotypes.cpp
	output/iaudiooutput.cpp
	output/iaudiosource.cpp
	sdlloader.cpp
	genmod/breseninter.cpp
	genmod/genbase.cpp
	genmod/genchannel.cpp
	genmod/genmodule.cpp
	genmod/genorder.cpp
	genmod/gensample.cpp
	genmod/ipatterncell.cpp
	genmod/modulestate.cpp
	genmod/songinfo.cpp
	light4cxx/location.cpp
	light4cxx/logger.cpp
)

target_link_libraries(ppplay_core ${SDL_LIBRARY} ${Boost_LIBRARIES} ${mp3lame_LIB})
install(TARGETS ppplay_core DESTINATION lib)
