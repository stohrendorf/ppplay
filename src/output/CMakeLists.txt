if(WITH_MP3LAME)
	add_library(ppplay_output_mp3 SHARED mp3audiooutput.cpp)
	target_link_libraries(ppplay_output_mp3 ppplay_core)
	install(TARGETS ppplay_output_mp3 DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if(WITH_OGG)
	add_library(ppplay_output_ogg SHARED oggaudiooutput.cpp)
	target_link_libraries(ppplay_output_ogg ppplay_core)
	install(TARGETS ppplay_output_ogg DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

add_library(ppplay_output_sdl SHARED sdlaudiooutput.cpp)
target_link_libraries(ppplay_output_sdl ppplay_core)
install(TARGETS ppplay_output_sdl DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(ppplay_output_wav SHARED wavaudiooutput.cpp)
target_link_libraries(ppplay_output_wav ppplay_core)
install(TARGETS ppplay_output_wav DESTINATION ${CMAKE_INSTALL_LIBDIR})
