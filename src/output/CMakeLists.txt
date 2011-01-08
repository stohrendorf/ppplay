add_library(fifoout STATIC
	audiofifo.cpp)

add_library(audioout STATIC
	iaudiosource.cpp outgen.cpp outsdl.cpp)

target_link_libraries(audioout fifoout)