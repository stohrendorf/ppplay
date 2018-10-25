add_library( ppplay_input_xm STATIC
             xmmodule.cpp
             xmpattern.cpp
             xminstrument.cpp
             xmsample.cpp
             xmchannel.cpp
             xmenvelopeprocessor.cpp
             xmcell.cpp
             xmmodule.h
             xmpattern.h
             xminstrument.h
             xmsample.h
             xmchannel.h
             xmenvelopeprocessor.h
             xmcell.h
             )
target_link_libraries( ppplay_input_xm PUBLIC ppplay_module_base ppplay_core )
