add_library(ppplay_bankdb STATIC bankdatabase.h bankdatabase.cpp)
target_link_libraries(ppplay_bankdb PUBLIC ${Boost_SERIALIZATION_LIBRARIES} ppplay_opl)

add_executable(ppplay_bankgen loader.h bankgen.cpp loader.cpp)
target_link_libraries(ppplay_bankgen ppplay_stream ppplay_bankdb)

set(BANKDB_FILE ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/ppplay/bankdb.xml CACHE FILEPATH "Bank database filename." FORCE)

add_custom_command(
    TARGET ppplay_bankgen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/ppplay
    COMMAND ppplay_bankgen ${BANKDB_FILE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/data/oplbanks
    COMMENT "Generating MIDI bank database..."
)

install(FILES ${BANKDB_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ppplay)
