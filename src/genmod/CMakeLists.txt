include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(GenerateExportHeader)
add_compiler_export_flags()

add_library(ppplay_module_base SHARED
	genbase.cpp
	abstractmodule.cpp
	orderentry.cpp
	channelstate.cpp
	sample.cpp
	ipatterncell.cpp
	modulestate.cpp
	standardfxdesc.cpp
)
set_target_properties(ppplay_module_base PROPERTIES SOVERSION 1)
generate_export_header(ppplay_module_base)
target_link_libraries(ppplay_module_base ppplay_core)
install(TARGETS ppplay_module_base DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT ppplayExports)
install(FILES
	abstractmodule.h
	orderentry.h
	breseninter.h
	channelstate.h
	genbase.h
	ipatterncell.h
	modulestate.h
	sample.h
	songinfo.h
	standardfxdesc.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_module_base_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/genmod
	COMPONENT Devel
)
