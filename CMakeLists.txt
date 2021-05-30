cmake_minimum_required(VERSION 2.8)

if(NOT "${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_STANDARD 17)
endif()


include(${CMAKE_CURRENT_SOURCE_DIR}/findTinyXml.cmake)

if(NOT TinyXML2_INCLUDE_DIRS STREQUAL TINYXML2_INCLUDE_DIR-NOTFOUND AND NOT TinyXML2_LIBRARIES STREQUAL TINYXML2_LIBRARY-NOTFOUND)
  # tinyxml successfully installed
else()
  message("\n\n________________________________________________________________________________")
  message("| CMake could not find an installation of tinyxml2. I have to use my local repository.")
  message("| This means you will get all the warnings. If you dont want this:")
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

  set(TinyXML2_LIBRARIES tinyxml2)
  set(TinyXML2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/tinyxml2)
endif()


add_library(settings_lib INTERFACE)
target_link_libraries(settings_lib INTERFACE
    ${TinyXML2_LIBRARIES})

target_include_directories(settings_lib
      INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      INTERFACE SYSTEM ${TinyXML2_INCLUDE_DIRS})


option(TEST_SETTINGS "Build tests for settings_lib and tinyxml2" FALSE)
if(TEST_SETTINGS)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(src/tinyxml2)
endif()
