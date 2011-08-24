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

