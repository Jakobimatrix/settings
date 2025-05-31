

if (FUZZER_ENABLED)

  add_executable(fuzzer_settings src/fuzzer_settings.cpp)

  target_link_libraries(fuzzer_settings 
    PRIVATE 
    settings_lib_2.0.0
    ${ENVIRONMENT_SETTINGS}
  )

else()
  # only enable other executables (like tests), if Fuzzer is not linked. Fuzzer brings its own main!
    
  add_executable(example src/example.cpp)

  # Optionally, you can also install the executable
  install(TARGETS example DESTINATION bin)

  target_link_libraries(example 
    PRIVATE
    settings_lib_2.0.0
    ${ENVIRONMENT_SETTINGS}
  ) 


endif()

