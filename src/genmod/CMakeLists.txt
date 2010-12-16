add_subdirectory(phaser)

add_library(genmod STATIC
	genbase.cpp
	gensample.cpp
	genpattern.cpp
	genchannel.cpp
	genmodule.cpp)

# target_link_libraries(genmod binstream)
