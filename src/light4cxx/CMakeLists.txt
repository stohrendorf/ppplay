include(GenerateExportHeader)
add_compiler_export_flags()

if(WIN32)
        add_definitions(-DL4CXX_NO_ANSI_COLORS)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(ppplay_light4cxx SHARED
        location.cpp
        logger.cpp
)
generate_export_header(ppplay_light4cxx)

target_link_libraries(ppplay_light4cxx ${Boost_SYSTEM_LIBRARY})

install(TARGETS ppplay_light4cxx DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT ppplayExports)
install(FILES
        level.h
        location.h
        logger.h
        ${CMAKE_CURRENT_BINARY_DIR}/ppplay_light4cxx_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/light4cxx
        COMPONENT Devel
)
