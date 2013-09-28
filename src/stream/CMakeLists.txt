set(LIBARCHIVE_ROOT "" CACHE PATH "The location of the LibArchive install prefix (only used if the LibArchive is not yet found)")
if(LIBARCHIVE_ROOT)
   list(APPEND CMAKE_PREFIX_PATH ${LIBARCHIVE_ROOT})
endif()
include(FindLibArchive)
find_package(LibArchive REQUIRED)
set_package_info(LibArchive "LibArchive" "http://www.libarchive.org" "LibArchive enables loading modules from archives.")

include_directories(${LibArchive_INCLUDE_DIR})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include(GenerateExportHeader)
add_compiler_export_flags()

add_library(ppplay_stream SHARED
	stream.cpp
	filestream.cpp
	abstractarchive.cpp
	memarchive.cpp
	memorystream.cpp
	archivefilestream.cpp
)

generate_export_header(ppplay_stream)

target_link_libraries(ppplay_stream ppplay_light4cxx ${LibArchive_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
install(TARGETS ppplay_stream DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT ppplayExports)
install(FILES
	abstractarchive.h
	archivefilestream.h
	filestream.h
	iserializable.h
	memarchive.h
	memorystream.h
	stream.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_stream_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/stream
)
