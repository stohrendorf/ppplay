add_library(genmod STATIC
	genbase.cpp
	gensample.cpp
	genpattern.cpp
	genchannel.cpp
	genmodule.cpp
	pppexcept.cpp)
#target_link_libraries(genmod binstream fifoout)
add_subdirectory(phaser)