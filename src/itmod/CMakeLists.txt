if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif ()
add_library(ppplay_input_it MODULE
        plugin.cpp
        itmodule.h
        itmodule.cpp
        instrument.h
        sample.h
        slavechannel.h
        slavechannel.cpp
        hostchannel.h
        filters.h
        )
target_link_libraries(ppplay_input_it ppplay_module_base ppplay_core)
install(TARGETS ppplay_input_it LIBRARY DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} COMPONENT Plugins OPTIONAL)
