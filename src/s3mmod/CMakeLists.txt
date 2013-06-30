set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/ppplay)

if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
add_library(ppplay_input_s3m MODULE
	s3msample.cpp
	s3mpattern.cpp
	s3mchannel.cpp
	s3mmodule.cpp
	s3mcell.cpp
	s3morder.cpp
	plugin.cpp
)
target_link_libraries(ppplay_input_s3m ppplay_core)
install(TARGETS ppplay_input_s3m LIBRARY DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} COMPONENT Plugins OPTIONAL)
