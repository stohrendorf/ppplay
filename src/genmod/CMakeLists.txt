add_subdirectory(phaser)

add_library(genmod STATIC
	genbase.cpp
	gensample.cpp
	gencell.cpp
	genchannel.cpp
	genmodule.cpp
	breseninter.cpp
)

# target_link_libraries(genmod binstream)
