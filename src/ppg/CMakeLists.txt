set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include(GenerateExportHeader)
add_compiler_export_flags()

add_library(ppplay_ppg SHARED
	widget.cpp
	label.cpp
	sdlscreen.cpp
	point.cpp
	stereopeakbar.cpp
	progressbar.cpp
	fonts.cpp
)
generate_export_header(ppplay_ppg)
target_link_libraries(ppplay_ppg ppplay_core)
install(TARGETS ppplay_ppg DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT ppplayExports)

install(FILES
	color.h
	label.h
	point.h
	progressbar.h
	rect.h
	sdlscreen.h
	stereopeakbar.h
	widget.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_ppg_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/ppg
	COMPONENT Devel
)

add_subdirectory(tests)