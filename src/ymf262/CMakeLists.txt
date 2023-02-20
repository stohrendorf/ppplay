add_library( ppplay_opl STATIC
             channel.cpp
             envelopegenerator.cpp
             operator.cpp
             opl3.cpp
             phasegenerator.cpp
             channel.h
             envelopegenerator.h
             operator.h
             opl3.h
             phasegenerator.h
             oplfilter.h
             )

target_link_libraries( ppplay_opl PUBLIC ppplay_stream )

option( BUILD_IMFPLAY "Build the OPL-only IMF-Play program" ON )
if( BUILD_IMFPLAY )
    add_executable( imfplay imfplay.cpp )
    if( COMPILER_IS_CLANG )
        target_link_libraries( imfplay stdc++ )
    endif()
    target_link_libraries( imfplay ppplay_opl Boost::program_options Boost::filesystem ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ppplay_stream ppplay_core )
    install( TARGETS imfplay DESTINATION bin COMPONENT application )
endif()
