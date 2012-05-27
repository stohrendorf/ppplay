# add_library(s3mmod STATIC
# 	s3msample.cpp
# 	s3mpattern.cpp
# 	s3mchannel.cpp
# 	s3mmodule.cpp
# 	s3mcell.cpp
# 	s3morder.cpp
# )

add_library(ppplay_input_s3m MODULE
	s3msample.cpp
	s3mpattern.cpp
	s3mchannel.cpp
	s3mmodule.cpp
	s3mcell.cpp
	s3morder.cpp
	plugin.cpp
)
target_link_libraries(ppplay_input_s3m ppplay_core)
install(TARGETS ppplay_input_s3m LIBRARY DESTINATION lib/ppplay)
