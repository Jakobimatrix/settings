cmake_minimum_required(VERSION 2.8)

set(SETTINGS_LIB_VERSION "1.0.0")


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

add_library(settings_lib_${SETTINGS_LIB_VERSION} INTERFACE)

# Create an alias with the version in its name
add_library(settings_lib ALIAS settings_lib_${SETTINGS_LIB_VERSION})

target_link_libraries(settings_lib_${SETTINGS_LIB_VERSION} INTERFACE
    tinyxml2
    utils_lib_0.0.1)

target_include_directories(settings_lib_${SETTINGS_LIB_VERSION}
      INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

option(TEST_SETTINGS "Build Boost Tests for settings_lib and tinyxml2" FALSE)

if(TEST_SETTINGS)
  enable_testing()
  add_subdirectory(tests)
  set(tinyxml2_BUILD_TESTING TRUE)
  add_subdirectory(src/tinyxml2)
endif()
