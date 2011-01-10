add_library(audioout STATIC
	audiofifo.cpp iaudiosource.cpp iaudiooutput.cpp sdlaudiooutput.cpp)

target_link_libraries(audioout utils)
