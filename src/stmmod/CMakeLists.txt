add_library(stmmod STATIC
	stmbase.h stmbase.cpp
	stmsample.h stmsample.cpp
	stmpattern.h stmpattern.cpp
	stmchannel.h stmchannel.cpp
	stmmodule.h stmmodule.cpp)
target_link_libraries(stmmod genmod binstream fifoout)
