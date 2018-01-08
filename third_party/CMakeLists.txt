add_library(gtest STATIC googletest/src/gtest-all.cc)
target_include_directories(gtest SYSTEM PUBLIC googletest/include)
target_include_directories(gtest PRIVATE googletest)

# NOTE(josh): compatible with cmake 3.1.0+
target_link_libraries(gtest Threads::Threads)

# NOTE(josh): compatible with cmake 2.8.12+.
# target_compile_options(gtest PUBLIC "-pthread")
# target_link_libraries(gtest PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_library(gtest_main STATIC googletest/src/gtest_main.cc)
target_include_directories(gtest_main SYSTEM PUBLIC googletest/include)
target_include_directories(gtest_main PRIVATE googletest)

# NOTE(josh): compatible with cmake 3.1.0+
target_link_libraries(gtest_main Threads::Threads)

# NOTE(josh): compatible with cmake 2.8.12+.
# target_compile_options(gtest_main PUBLIC "-pthread")
# target_link_libraries(gtest_main PUBLIC ${CMAKE_THREAD_LIBS_INIT})
