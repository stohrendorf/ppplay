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
set_target_properties(ppplay_input_s3m PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LOCAL_LIBEXEC})
target_link_libraries(ppplay_input_s3m ppplay_module_base ppplay_core)
install(TARGETS ppplay_input_s3m LIBRARY DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} COMPONENT Plugins OPTIONAL)
