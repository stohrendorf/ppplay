add_definitions(-DBOOST_TEST_MAIN -DBOOST_TEST_DYN_LINK)
add_executable(
	field_test_exe
	field_test.cpp
)
target_link_libraries(field_test_exe ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
if(COMPILER_IS_CLANG)
	target_link_libraries(field_test_exe stdc++)
endif()

add_test(NAME FieldTest COMMAND field_test_exe)
