option(MOD_USE_NTSC_FREQUENCY "Use NTSC frequency base instead of PAL for .mod playback" OFF)
if(MOD_USE_NTSC_FREQUENCY)
	add_definitions(-DMOD_USE_NTSC_FREQUENCY)
endif()

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
	modsample.h
	modcell.h
	modmodule.h
	modpattern.h
	modchannel.h
	modbase.h
	modorder.h
	plugin.cpp
)
target_link_libraries(ppplay_input_mod ppplay_module_base ppplay_core)
install(TARGETS ppplay_input_mod LIBRARY DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} COMPONENT Plugins OPTIONAL)
