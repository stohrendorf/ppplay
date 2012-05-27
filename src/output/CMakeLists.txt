# set(audioout_SRCS audiofifo.cpp iaudiosource.cpp iaudiooutput.cpp sdlaudiooutput.cpp audiotypes.cpp)
# if(${WITH_MP3LAME})
# 	set(audioout_SRCS ${audioout_SRCS} mp3audiooutput.cpp)
# endif()

# add_library(audioout STATIC ${audioout_SRCS})

if(${WITH_MP3LAME})
	add_library(ppplay_output_mp3 SHARED mp3audiooutput.cpp)
	target_link_libraries(ppplay_output_mp3 ppplay_core)
	install(TARGETS ppplay_output_mp3 LIBRARY DESTINATION lib)
endif()

add_library(ppplay_output_sdl SHARED sdlaudiooutput.cpp)
target_link_libraries(ppplay_output_sdl ppplay_core)
install(TARGETS ppplay_output_sdl LIBRARY DESTINATION lib)
