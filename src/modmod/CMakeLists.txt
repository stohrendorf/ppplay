add_library(modmod STATIC
	modbase.h modbase.cpp
	modsample.h modsample.cpp
	modpattern.h modpattern.cpp
	modchannel.h modchannel.cpp
	modmodule.h modmodule.cpp)

target_link_libraries(modmod genmod binstream fifoout)
