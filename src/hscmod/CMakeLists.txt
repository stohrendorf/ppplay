if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
add_library(ppplay_input_hsc MODULE
	plugin.cpp
	hscmodule.cpp
)
target_link_libraries(ppplay_input_hsc ppplay_module_base ppplay_core ppplay_opl)
install(TARGETS ppplay_input_hsc LIBRARY DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} COMPONENT Plugins OPTIONAL)
