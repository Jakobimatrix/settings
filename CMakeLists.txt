cmake_minimum_required(VERSION 2.8)

add_subdirectory(src/tinyxml2)

add_library(settings_lib INTERFACE)

target_link_libraries(settings_lib
      INTERFACE tinyxml2)

target_include_directories(settings_lib
      INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      INTERFACE tinyxml2)

# testing
add_subdirectory(tests)
