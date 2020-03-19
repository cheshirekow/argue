# NOTE(josh): don't build gtest if it's not part of this sparse checkout
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
  message("Enabling third_party:googletest for this checkout")
  add_library(gtest STATIC googletest/src/gtest-all.cc)
  add_library(gtest-shared SHARED googletest/src/gtest-all.cc)
  set_target_properties(gtest gtest-shared PROPERTIES OUTPUT_NAME gtest
                                                      DEBUG_POSTFIX d)

  foreach(_target gtest gtest-shared)
    target_include_directories(${_target} SYSTEM PUBLIC googletest/include)
    target_include_directories(${_target} PRIVATE googletest)

    # NOTE(josh): compatible with cmake 3.1.0+
    target_link_libraries(${_target} Threads::Threads)

    # NOTE(josh): compatible with cmake 2.8.12+. target_compile_options(gtest
    # PUBLIC "-pthread") target_link_libraries(gtest PUBLIC
    # ${CMAKE_THREAD_LIBS_INIT})
  endforeach()

  add_library(gtest_main STATIC googletest/src/gtest_main.cc)
  target_include_directories(gtest_main SYSTEM PUBLIC googletest/include)
  target_include_directories(gtest_main PRIVATE googletest)

  # NOTE(josh): compatible with cmake 3.1.0+
  target_link_libraries(gtest_main Threads::Threads)

  # NOTE(josh): compatible with cmake 2.8.12+. target_compile_options(gtest_main
  # PUBLIC "-pthread") target_link_libraries(gtest_main PUBLIC
  # ${CMAKE_THREAD_LIBS_INIT})
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CImg)
  add_library(cimg INTERFACE)
  target_include_directories(cimg INTERFACE CImg)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/phash)
  set(phash_sources phash/src/pHash.cpp)
  add_library(phash STATIC ${phash_sources})
  add_library(phash-shared SHARED ${phash_sources})
  set_target_properties(phash phash-shared PROPERTIES OUTPUT_NAME phash
                                                      DEBUG_POSTFIX d)
  foreach(_target phash phash-shared)
    target_include_directories(${_target} PUBLIC phash/include)
    target_include_directories(${_target} PRIVATE phash/private)
    target_link_libraries(${_target} PUBLIC cimg Threads::Threads)
    target_pkg_depends(${_target} libpng)
  endforeach()
endif()
