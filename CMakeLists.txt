cmake_minimum_required(VERSION 2.8)
# Set the policy to avoid warnings about project versioning
cmake_policy(SET CMP0048 NEW)

project(Settings VERSION 1.0.0)

if(NOT "${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(tinyxml2 QUIET)

if(NOT tinyxml2_FOUND)
  add_subdirectory(src/tinyxml2)
  target_compile_options(tinyxml2 PRIVATE
      -w  # Suppress all warnings
      -Wno-error # dont treat warnings as errors
  )
endif()

add_library(settings_lib INTERFACE)

target_link_libraries(settings_lib INTERFACE
    tinyxml2)

target_include_directories(settings_lib
      INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

option(TEST_SETTINGS "Build tests for settings_lib and tinyxml2" FALSE)
if(TEST_SETTINGS)
  enable_testing()
  add_subdirectory(tests)
  set(tinyxml2_BUILD_TESTING TRUE)
  add_subdirectory(src/tinyxml2)
endif()
