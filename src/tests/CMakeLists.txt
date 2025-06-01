
if (TARGET Catch2::Catch2WithMain)

  add_executable(test_settings src/test_settings.cpp)

  target_link_libraries(test_settings 
    PRIVATE
    Catch2::Catch2WithMain
    settings_lib_2.0.0
    ${ENVIRONMENT_SETTINGS}
    )


  catch_discover_tests(test_settings)


endif()
