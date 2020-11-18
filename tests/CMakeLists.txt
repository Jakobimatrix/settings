project(test_settings_lib)


set(BOOST_INCLUDE_DIRS $boost_installation_prefix/include)

include_directories(${Boost_INCLUDE_DIRS})
link_directories (${Boost_LIBRARY_DIRS})

find_package(Boost COMPONENTS system unit_test_framework filesystem)

if( Boost_FOUND )
  enable_testing()
  add_executable(test_settings test_settings.cpp)

  target_link_libraries(test_settings
                        tinyxml2
                        settings_lib
                        ${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                        )

  # declares a test with our executable
  add_test(NAME settings_test COMMAND test_settings)
endif()
