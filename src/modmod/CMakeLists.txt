option(MOD_USE_NTSC_FREQUENCY "Use NTSC frequency base instead of PAL for .mod playback" OFF)
if(MOD_USE_NTSC_FREQUENCY)
	add_definitions(-DMOD_USE_NTSC_FREQUENCY)
endif()

add_library(ppplay_input_mod STATIC
	modsample.cpp
	modcell.cpp
	modmodule.cpp
	modpattern.cpp
	modchannel.cpp
	modbase.cpp
	modsample.h
	modcell.h
	modmodule.h
	modpattern.h
	modchannel.h
	modbase.h
	    plugin.h
)
target_link_libraries(ppplay_input_mod PUBLIC ppplay_module_base ppplay_core)
