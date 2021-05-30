cmake  .. -DTEST_SETTINGS=True
cmake --build .
ctest --output-on-failure

