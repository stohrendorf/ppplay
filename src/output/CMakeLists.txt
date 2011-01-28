set(audioout_SRCS audiofifo.cpp iaudiosource.cpp iaudiooutput.cpp sdlaudiooutput.cpp)
if(${WITH_MP3LAME})
	set(audioout_SRCS ${audioout_SRCS} mp3audiooutput.cpp)
endif()

add_library(audioout STATIC ${audioout_SRCS})

target_link_libraries(audioout utils)
