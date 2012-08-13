include(FindLibArchive)
find_library(LibArchive REQUIRED)
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

target_link_libraries(ppplay_stream ${LibArchive_LIBRARY})
install(TARGETS ppplay_stream DESTINATION lib EXPORT ppplayExports)
install(FILES
	abstractarchive.h
	archivefilestream.h
	filestream.h
	iserializable.h
	memarchive.h
	memorystream.h
	stream.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_stream_export.h
	DESTINATION include/ppplay/stream
)
