add_definitions( -DBOOST_TEST_MAIN -DBOOST_TEST_DYN_LINK )
add_executable(
        ppg_test_exe
        ppg_test.cpp
)
target_link_libraries( ppg_test_exe Boost::unit_test_framework ppplay_ppg )
if( COMPILER_IS_CLANG )
    target_link_libraries( ppg_test_exe stdc++ )
endif()

add_test( NAME PpgTest COMMAND ppg_test_exe )
