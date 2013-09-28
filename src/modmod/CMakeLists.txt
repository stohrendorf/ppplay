option(MOD_USE_NTSC_FREQUENCY "Use NTSC frequency base instead of PAL for .mod playback" OFF)
if(MOD_USE_NTSC_FREQUENCY)
	add_definitions(-DMOD_USE_NTSC_FREQUENCY)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/ppplay)

if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
add_library(ppplay_input_mod MODULE
	modsample.cpp
	modcell.cpp
	modmodule.cpp
	modpattern.cpp
	modchannel.cpp
	modbase.cpp
	modorder.cpp
	plugin.cpp
)
target_link_libraries(ppplay_input_mod ppplay_module_base ppplay_core)
install(TARGETS ppplay_input_mod LIBRARY DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} COMPONENT Plugins OPTIONAL)
