add_subdirectory( compression )

set( BADPLAY_SRCS
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
     dfm.cpp
     disk.cc
     dmo.cpp
     dro.cpp
     dro2.cpp
     dtm.cpp
     flash.cpp
     fmc.cpp
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
     mod.cpp
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
     a2m.h
     adl.h
     adplay.cc
     adplug.h
     adtrack.h
     amd.h
     bam.h
     bmf.h
     cff.h
     cmf.h
     d00.h
     dfm.h
     disk.cc
     dmo.h
     dro.h
     dro2.h
     dtm.h
     flash.h
     fmc.h
     hsc.h
     hsp.h
     hybrid.h
     hyp.h
     imf.h
     jbm.h
     ksm.h
     lds.h
     mad.h
     mid.h
     mkj.h
     msc.h
     mtk.h
     output.cc
     player.h
     players.h
     mod.h
     psi.h
     rad.h
     rat.h
     raw.h
     rix.h
     rol.h
     s3m.h
     sa2.h
     sdl.h
     sng.h
     u6m.h
     xad.h
     xsm.h
     mid/almidi.cpp
     mid/multichips.cpp
     mid/almidi.h
     mid/multichips.h
     )

add_executable( badplay ${BADPLAY_SRCS} )
add_subdirectory( bankgen )

target_link_libraries( badplay Boost::program_options ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ppplay_opl badplay_compression ppplay_bankdb ppplay_core )

install( TARGETS badplay DESTINATION bin COMPONENT application )
