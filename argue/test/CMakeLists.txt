add_executable(argue-parse_test parse_test.cc)
target_link_libraries(argue-parse_test argue gtest gtest_main)
add_test(NAME argue-parse_test
         COMMAND argue-parse_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(argue-arg_test arg_test.cc)
target_link_libraries(argue-arg_test argue gtest gtest_main)
add_test(NAME argue-arg_test
         COMMAND argue-arg_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(argue-util_test util_test.cc)
target_link_libraries(argue-util_test argue gtest gtest_main)
add_test(NAME argue-util_test
         COMMAND argue-util_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(argue-assert_test assert_test.cc)
target_link_libraries(argue-assert_test argue gtest gtest_main)
add_test(NAME argue-assert_test
         COMMAND argue-assert_test
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
