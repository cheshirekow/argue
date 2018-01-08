set(argue_headers
    argue.h)

set(argue_sources
    argue.cc)

set(argue_files
    ${argue_headers}
    ${aruge_sources}
    demo.cc
    test/arg_test.cc
    test/assert_test.cc
    test/parse_test.cc
    test/util_test.cc)

add_library(argue STATIC ${argue_sources})
add_executable(argue-demo demo.cc)
target_link_libraries(argue-demo argue)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/argue_format.stamp
                   COMMAND clang-format -style file -i ${argue_files}
                   COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/argue_format.stamp
                   DEPENDS ${argue_files}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(argue_format
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/argue_format.stamp)
add_dependencies(format argue_format)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/argue_lint.stamp
                   COMMAND cpplint ${argue_files}
                   COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/argue_lint.stamp
                   DEPENDS ${argue_files}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(argue_lint
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/argue_lint.stamp)

# NOTE(josh): format before lint-check, avoid formatting lint
add_dependencies(argue_lint argue_format)
add_dependencies(lint argue_lint)

add_subdirectory(doc)
add_subdirectory(test)
