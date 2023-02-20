set(LIBARCHIVE_ROOT "" CACHE PATH "The location of the LibArchive install prefix (only used if the LibArchive is not yet found)")
if(LIBARCHIVE_ROOT)
   list(APPEND CMAKE_PREFIX_PATH ${LIBARCHIVE_ROOT})
endif()
include(FindLibArchive)
find_package(LibArchive REQUIRED)

include_directories(${LibArchive_INCLUDE_DIR})

add_library(ppplay_stream STATIC
	stream.cpp
	filestream.cpp
	abstractarchive.cpp
	memarchive.cpp
	memorystream.cpp
	archivefilestream.cpp
	stream.h
	filestream.h
	abstractarchive.h
	memarchive.h
	memorystream.h
	archivefilestream.h
)

target_link_libraries(ppplay_stream PUBLIC ppplay_light4cxx ${LibArchive_LIBRARY} Boost::filesystem)
