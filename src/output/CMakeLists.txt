# set(audioout_SRCS audiofifo.cpp iaudiosource.cpp iaudiooutput.cpp sdlaudiooutput.cpp audiotypes.cpp)
# if(${WITH_MP3LAME})
# 	set(audioout_SRCS ${audioout_SRCS} mp3audiooutput.cpp)
# endif()

# add_library(audioout STATIC ${audioout_SRCS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(${WITH_MP3LAME})
	add_library(ppplay_output_mp3 SHARED mp3audiooutput.cpp)
	target_link_libraries(ppplay_output_mp3 ppplay_core)
	install(TARGETS ppplay_output_mp3 LIBRARY DESTINATION lib)
endif()

if(${WITH_OGG})
	add_library(ppplay_output_ogg SHARED oggaudiooutput.cpp)
	target_link_libraries(ppplay_output_ogg ppplay_core)
	install(TARGETS ppplay_output_ogg LIBRARY DESTINATION lib)
endif()

add_library(ppplay_output_sdl SHARED sdlaudiooutput.cpp)
target_link_libraries(ppplay_output_sdl ppplay_core)
install(TARGETS ppplay_output_sdl LIBRARY DESTINATION lib)

add_library(ppplay_output_wav SHARED wavaudiooutput.cpp)
target_link_libraries(ppplay_output_wav ppplay_core)
install(TARGETS ppplay_output_wav LIBRARY DESTINATION lib)
