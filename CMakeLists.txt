cmake_minimum_required(VERSION 2.8)

add_subdirectory(src/tinyxml2)

add_library(settings_lib INTERFACE)

target_link_libraries(settings_lib
      INTERFACE tinyxml2)

target_include_directories(settings_lib
      INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      INTERFACE tinyxml2)

# testing
enable_testing()
add_subdirectory(tests)



#If you are creating a shared library and your source cpp files #include the headers of another library
#(Say, QtNetwork for example), but your header files don't include QtNetwork headers, then QtNetwork is a PRIVATE dependency.

#If your source files and your headers include the headers of another library, then it is a PUBLIC dependency.

#If your header files but not your source files include the headers of another library, then it is an INTERFACE dependency.
