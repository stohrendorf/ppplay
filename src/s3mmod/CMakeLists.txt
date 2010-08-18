add_library(s3mmod STATIC
	s3mbase.h s3mbase.cpp
	s3msample.h s3msample.cpp
	s3mpattern.h s3mpattern.cpp
	s3mchannel.h s3mchannel.cpp
	s3mmodule.h s3mmodule.cpp)

# target_link_libraries(s3mmod genmod binstream fifoout)
