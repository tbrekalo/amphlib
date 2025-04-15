cmake_minimum_required(VERSION 3.31)
project(
  amphlib
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

option(amphlib_test "Build tests for amphlib" ${PROJECT_IS_TOP_LEVEL})
option(aphtlib_asan "Build amphlib with address sanitizer"
       ${PROJECT_IS_TOP_LEVEL})

add_library(amphlib ./src/lib.cc)
target_include_directories(amphlib PUBLIC ./include)

if(amphlib_asan)
  target_compile_options(
    amphlib PUBLIC $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address
                   -fno-omit-frame-pointer>)
  target_link_options(amphlib PUBLIC
                      $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address>)
endif()

if(amphlib_test)
  find_package(doctest REQUIRED)
  add_executable(test ./src/test.cc)
  target_link_libraries(test doctest::doctest amphlib)
endif()
