add_definitions(-DBOOST_TEST_MAIN -DBOOST_TEST_DYN_LINK)
add_executable(
	field_test_exe
	field_test.cpp
)
target_link_libraries(field_test_exe ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(FieldTest ${CMAKE_CURRENT_BINARY_DIR}/field_test_exe)
