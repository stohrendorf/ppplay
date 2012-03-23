add_library(modmod STATIC
	modsample.cpp
	modcell.cpp
	modmodule.cpp
	modpattern.cpp
	modchannel.cpp
	modbase.cpp
	modorder.cpp
)

option(MOD_USE_NTSC_FREQUENCY "Use NTSC frequency base instead of PAL for .mod playback" OFF)
if(MOD_USE_NTSC_FREQUENCY)
	add_definitions(-DMOD_USE_NTSC_FREQUENCY)
endif()