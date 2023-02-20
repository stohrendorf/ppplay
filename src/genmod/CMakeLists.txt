add_library( ppplay_module_base STATIC
             genbase.cpp
             abstractmodule.cpp
             orderentry.cpp
             channelstate.cpp
             sample.cpp
             ipatterncell.cpp
             modulestate.cpp
             standardfxdesc.cpp
             abstractmodule.h
             orderentry.h
             stepper.h
             channelstate.h
             genbase.h
             ipatterncell.h
             modulestate.h
             sample.h
             songinfo.h
             standardfxdesc.h
             )
target_link_libraries( ppplay_module_base PUBLIC ppplay_core )
