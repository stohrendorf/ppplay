if( WITH_MP3LAME )
    add_library( ppplay_output_mp3 STATIC mp3audiooutput.cpp mp3audiooutput.h )
    target_link_libraries( ppplay_output_mp3 PUBLIC ppplay_core )
endif()

if( WITH_OGG )
    add_library( ppplay_output_ogg STATIC oggaudiooutput.cpp oggaudiooutput.h )
    target_link_libraries( ppplay_output_ogg PUBLIC ppplay_core vorbis vorbisenc ogg )
endif()

add_library( ppplay_output_sdl STATIC sdlaudiooutput.cpp sdlaudiooutput.h )
target_link_libraries( ppplay_output_sdl PUBLIC ppplay_core )

add_library( ppplay_output_wav STATIC wavaudiooutput.cpp wavaudiooutput.h )
target_link_libraries( ppplay_output_wav PUBLIC ppplay_core )
