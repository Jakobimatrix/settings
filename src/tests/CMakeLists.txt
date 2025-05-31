
if (NOT FUZZER_ENABLED)

  # only enable other executables, if Fuzzer is not linked. Fuzzer brings its own main!
  Include(FetchContent)

  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0 # or a later release
  )

  FetchContent_MakeAvailable(Catch2)

  enable_testing()

  add_executable(test_settings src/test_settings.cpp)

  target_link_libraries(test_settings 
    PRIVATE
    Catch2::Catch2WithMain
    settings_lib_2.0.0
    ${ENVIRONMENT_SETTINGS}
    )

  include(CTest)
  #include(Catch) // already included by utils library

  catch_discover_tests(test_settings)


endif()