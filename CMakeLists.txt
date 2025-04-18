cmake_minimum_required(VERSION 3.25)
project(
  amphlib
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

option(amphlib_test "Build tests for amphlib" ${PROJECT_IS_TOP_LEVEL})
option(aphtlib_asan "Build amphlib with address sanitizer"
       ${PROJECT_IS_TOP_LEVEL})

find_package(SQLite3 REQUIRED)
add_library(amphlib src/book.cc src/isbn.cc src/lib.cc src/library.cc
                    src/uuid.cc)
target_include_directories(amphlib PUBLIC include)
target_link_libraries(amphlib PRIVATE SQLite::SQLite3)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(amphlib PUBLIC "-stdlib=libc++")
  target_link_options(amphlib PUBLIC "-stdlib=libc++")
endif()

if(amphlib_asan)
  target_compile_options(
    amphlib PUBLIC $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address
                   -fno-omit-frame-pointer>)
  target_link_options(amphlib PUBLIC
                      $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address>)
endif()

if(amphlib_test)
  find_package(doctest QUIET)
  add_executable(test ./src/test.cc)
  target_link_libraries(test doctest::doctest amphlib)
endif()
