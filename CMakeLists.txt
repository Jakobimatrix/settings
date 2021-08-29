cmake_minimum_required(VERSION 2.8)
project(Settings)

if(NOT "${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(tinyxml2 REQUIRED)

if(tinyxml2_VERSION LESS 9.0.0)
  message("\n\n________________________________________________________________________________")
  message("| CMake could not find an installation of tinyxml2 (Version 9.0.0).")
  message("| I have to use my local repository. This means you will get all the")
  message("| warnings from your compiler from tinyxml2. If you dont want this:")
  message("| Please go into settings/src/tinyxml2/")
  message("| mkdir build && cd build")
  message("| cmake -DCMAKE_BUILD_TYPE=RELEASE ..")
  message("| sudo make install")
  message("________________________________________________________________________________\n\n")
  if(UNIX)
    # show the message 4 seconds and annoy people to make them install the lib.
    execute_process(COMMAND sleep 4)
  endif()
  add_subdirectory(src/tinyxml2)
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
