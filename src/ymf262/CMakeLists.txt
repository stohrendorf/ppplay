include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include(GenerateExportHeader)
add_compiler_export_flags()

add_library(ppplay_opl SHARED
	channel.cpp
	envelopegenerator.cpp
	operator.cpp
	opl3.cpp
	phasegenerator.cpp
)

generate_export_header(ppplay_opl)

install(TARGETS ppplay_opl DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT ppplayExports)
install(FILES
	opl3.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_opl_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/opl
)
option(BUILD_DROPLAY "Build the OPL-only DRO-Play program" ON)
if(BUILD_DROPLAY)
	add_executable(imfplay imfplay.cpp)
	install(TARGETS imfplay DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT application)
	target_link_libraries(imfplay ppplay_opl ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${SDL_LIBRARY} ppplay_stream ppplay_core)
endif()
