# add_library(ppg STATIC
# 	widget.cpp
# 	label.cpp sdlscreen.cpp
# 	point.cpp
# 	stereopeakbar.cpp
# 	progressbar.cpp
# 	fonts.cpp
# )

add_library(ppplay_ppg SHARED
	widget.cpp
	label.cpp
	sdlscreen.cpp
	point.cpp
	stereopeakbar.cpp
	progressbar.cpp
	fonts.cpp
)
target_link_libraries(ppplay_ppg ppplay_core)
install(TARGETS ppplay_ppg DESTINATION lib)
