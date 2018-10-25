add_library(ppplay_input_s3m STATIC
    s3msample.cpp
    s3mpattern.cpp
    s3mchannel.cpp
    s3mmodule.cpp
    s3mcell.cpp
    s3morder.cpp
    s3msample.h
    s3mpattern.h
    s3mchannel.h
    s3mmodule.h
    s3mcell.h
    s3morder.h
            plugin.h
)
target_link_libraries(ppplay_input_s3m PUBLIC ppplay_module_base ppplay_core)
