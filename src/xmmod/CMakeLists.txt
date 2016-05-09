if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
add_library(ppplay_input_xm MODULE
	xmmodule.cpp
	xmpattern.cpp
	xminstrument.cpp
	xmsample.cpp
	xmchannel.cpp
	xmenvelopeprocessor.cpp
	xmcell.cpp
	xmorder.cpp
	plugin.cpp
)
target_link_libraries(ppplay_input_xm ppplay_module_base ppplay_core)
install(TARGETS ppplay_input_xm LIBRARY DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} COMPONENT Plugins OPTIONAL)
