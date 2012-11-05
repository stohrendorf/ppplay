set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/ppplay)

if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
add_library(ppplay_input_hsc MODULE
	plugin.cpp
	hscmodule.cpp
)
target_link_libraries(ppplay_input_hsc ppplay_core ppplay_hsc)
install(TARGETS ppplay_input_hsc LIBRARY DESTINATION lib/ppplay COMPONENT Plugins OPTIONAL)
