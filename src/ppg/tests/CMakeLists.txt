add_definitions(-DBOOST_TEST_MAIN -DBOOST_TEST_DYN_LINK)
add_executable(
	ppg_test_exe
	ppg_test.cpp
)
target_link_libraries(ppg_test_exe ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME PpgTest COMMAND ppg_test_exe)
