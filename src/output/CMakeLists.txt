include(GenerateExportHeader)
add_compiler_export_flags()

if(WITH_MP3LAME)
    add_library(ppplay_output_mp3 SHARED mp3audiooutput.cpp mp3audiooutput.h)
    generate_export_header(ppplay_output_mp3)
    target_include_directories(ppplay_output_mp3 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(ppplay_output_mp3 PROPERTIES SOVERSION 1)
    target_link_libraries(ppplay_output_mp3 ppplay_core)
    install(TARGETS ppplay_output_mp3 DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ppplay_output_mp3_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/ppg
        COMPONENT Devel
    )
endif()

if(WITH_OGG)
    add_library(ppplay_output_ogg SHARED oggaudiooutput.cpp oggaudiooutput.h)
    generate_export_header(ppplay_output_ogg)
    target_include_directories(ppplay_output_ogg PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(ppplay_output_ogg PROPERTIES SOVERSION 1)
    target_link_libraries(ppplay_output_ogg ppplay_core)
    install(TARGETS ppplay_output_ogg DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ppplay_output_ogg_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/ppg
        COMPONENT Devel
    )
endif()

add_library(ppplay_output_sdl SHARED sdlaudiooutput.cpp sdlaudiooutput.h)
set_target_properties(ppplay_output_sdl PROPERTIES SOVERSION 1)
generate_export_header(ppplay_output_sdl)
target_link_libraries(ppplay_output_sdl PUBLIC ppplay_core)
target_include_directories(ppplay_output_sdl PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
install(TARGETS ppplay_output_sdl DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(ppplay_output_wav SHARED wavaudiooutput.cpp wavaudiooutput.h)
set_target_properties(ppplay_output_wav PROPERTIES SOVERSION 1)
generate_export_header(ppplay_output_wav)
target_link_libraries(ppplay_output_wav PUBLIC ppplay_core)
target_include_directories(ppplay_output_wav PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
install(TARGETS ppplay_output_wav DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ppplay_output_wav_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/ppplay_output_sdl_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppplay/ppg
    COMPONENT Devel
)
