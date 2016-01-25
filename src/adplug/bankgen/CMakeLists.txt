add_library(ppplay_bankdb STATIC bankdatabase.cpp)
target_link_libraries(ppplay_bankdb PUBLIC ${Boost_SERIALIZATION_LIBRARIES} ppplay_opl)

add_executable(ppplay_bankgen bankgen.cpp loader.cpp)
target_link_libraries(ppplay_bankgen ppplay_stream ppplay_bankdb)
