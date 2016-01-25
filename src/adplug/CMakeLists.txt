set(BADPLAY_SRCS
    a2m.cpp
    adl.cpp
    adplay.cc
    adplug.cpp
    adtrack.cpp
    amd.cpp
    bam.cpp
    bmf.cpp
    cff.cpp
    cmf.cpp
    d00.cpp
    debug.c
    dfm.cpp
    disk.cc
    dmo.cpp
    dro.cpp
    dro2.cpp
    dtm.cpp
    flash.cpp
    fmc.cpp
    getopt.c
    getopt1.c
    hsc.cpp
    hsp.cpp
    hybrid.cpp
    hyp.cpp
    imf.cpp
    jbm.cpp
    ksm.cpp
    lds.cpp
    mad.cpp
    mid.cpp
    mkj.cpp
    msc.cpp
    mtk.cpp
    output.cc
    player.cpp
    players.cpp
    protrack.cpp
    psi.cpp
    rad.cpp
    rat.cpp
    raw.cpp
    rix.cpp
    rol.cpp
    s3m.cpp
    sa2.cpp
    sdl.cc
    sng.cpp
    u6m.cpp
    xad.cpp
    xsm.cpp
    mid/almidi.cpp
    mid/multichips.cpp
)

option(USE_BANKDB "Use MIDI bank database instead of hard-coded Duke Nukem 3D instruments" ON)
if(NOT USE_BANKDB)
    list(APPEND BADPLAY_SRCS mid/timbre.cpp)
endif()

add_executable(badplay ${BADPLAY_SRCS})
target_link_libraries(badplay ${SDL_LIBRARY} ppplay_core ppplay_opl)

if(USE_BANKDB)
    target_compile_definitions(badplay PRIVATE -DUSE_BANKDB)

    add_subdirectory(bankgen)

    set(BANKDB_FILE ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/ppplay/bankdb.txt)

    add_custom_command(
        TARGET badplay POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/ppplay
        COMMAND ppplay_bankgen ARGS ${BANKDB_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/data/oplbanks
        COMMENT "Generating MIDI bank database..."
    )
    # add_dependencies(badplay ${CMAKE_SOURCE_DIR}/data/bankdb.txt)
    target_link_libraries(badplay ppplay_bankdb)

    install(FILES ${BANKDB_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ppplay)
endif()

install(TARGETS badplay DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT application)
