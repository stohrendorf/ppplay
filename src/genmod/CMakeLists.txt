include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(GenerateExportHeader)
add_compiler_export_flags()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_library(ppplay_module_base SHARED
	genbase.cpp
	abstractchannel.cpp
	abstractmodule.cpp
	abstractorder.cpp
	sample.cpp
	ipatterncell.cpp
	modulestate.cpp
	songinfo.cpp
)
generate_export_header(ppplay_module_base)
install(TARGETS ppplay_module_base DESTINATION lib EXPORT ppplayExports)
install(FILES
	abstractchannel.h
	abstractmodule.h
	abstractorder.h
	breseninter.h
	genbase.h
	ipatterncell.h
	modulestate.h
	sample.h
	songinfo.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_module_base_export.h
	DESTINATION include/ppplay/genmod
	COMPONENT Devel
)
