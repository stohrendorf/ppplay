add_library(genmod STATIC
	genbase.h genbase.cpp
	gensample.h gensample.cpp
	genpattern.h genpattern.cpp
	genchannel.h genchannel.cpp
	genmodule.h genmodule.cpp
	pvector.h
	pppexcept.h pppexcept.cpp
	breseninter.h)
#target_link_libraries(genmod binstream fifoout)
