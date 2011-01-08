add_library(fifoout STATIC
	audiofifo.cpp)

add_library(audioout STATIC
	iaudiosource.cpp iaudiooutput.cpp sdlaudiooutput.cpp)

target_link_libraries(audioout fifoout)