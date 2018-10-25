add_library(ppplay_ppg STATIC
	widget.cpp
	label.cpp
	sdlscreen.cpp
	stereopeakbar.cpp
	progressbar.cpp
	fonts.cpp
	widget.h
	label.h
	sdlscreen.h
	stereopeakbar.h
	progressbar.h
	color.h
	point.h
	rect.h
	fonts.h
)
target_link_libraries(ppplay_ppg PUBLIC ppplay_core)

add_subdirectory(tests)
