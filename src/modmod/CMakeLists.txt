option(MOD_USE_NTSC_FREQUENCY "Use NTSC frequency base instead of PAL for .mod playback" OFF)
if(MOD_USE_NTSC_FREQUENCY)
	add_definitions(-DMOD_USE_NTSC_FREQUENCY)
endif()

# add_library(modmod STATIC
# 	modsample.cpp
# 	modcell.cpp
# 	modmodule.cpp
# 	modpattern.cpp
# 	modchannel.cpp
# 	modbase.cpp
# 	modorder.cpp
# )

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
target_link_libraries(ppplay_input_mod ppplay_core)
install(TARGETS ppplay_input_mod LIBRARY DESTINATION lib/ppplay)
