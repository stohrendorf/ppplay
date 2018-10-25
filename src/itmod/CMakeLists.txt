add_library( ppplay_input_it STATIC
             itmodule.h
             itmodule.cpp
             instrument.h
             sample.h
             slavechannel.h
             slavechannel.cpp
             hostchannel.h
             filters.h
             )
target_link_libraries( ppplay_input_it PUBLIC ppplay_module_base ppplay_core )
