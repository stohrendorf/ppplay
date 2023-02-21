add_library( ppplay_light4cxx STATIC
             location.cpp
             logger.cpp
             location.h
             logger.h
             level.h
             )

target_link_libraries( ppplay_light4cxx PUBLIC Boost::system )
